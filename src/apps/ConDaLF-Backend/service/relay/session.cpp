#include <common/logging/logging.h>
#include <cstdint>
#include <cstring>
#include <iostream>
#include <sstream>

#include "session.hpp"

using namespace condalf::service;

/*
TODO:
Missed messages zwischenspeichern?
*/

Session::Session()
{
    host = "";
    port = "";
    disconnected = false;
    session = COAP_INVALID_RVALUE;
    context = -1;
    transmit_queue = new MessageQueue();
    retransmit_queue = new MessageQueue();
    pending_message = nullptr;
}

Session::~Session()
{
    Disconnect();
    delete transmit_queue;
    delete retransmit_queue;

    if (pending_message != nullptr)
    {
        delete[] pending_message->data;
        delete pending_message;
    }
}

bool Session::Connect(common::CoAP::context_descriptor _context, const std::string& _host, const std::string& _port)
{
    // Save values
    host = _host;
    port = _port;
    context = _context;

    // Get the CoAP instance
    auto coap = &common::CoAP::getInstance();

    // Create a session
    session = coap->CreateSession(_context, _host, _port);

    if (session == COAP_INVALID_RVALUE)
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, std::string("Cannot create relay session to") + host + ":" + port);
        return false;
    }
    disconnected = false;
    return true;
}

bool Session::Reconnect()
{
    Disconnect();
    return Connect(context, host, port);
}

void Session::Disconnect()
{
    // Get the CoAP instance and release session
    auto coap = &common::CoAP::getInstance();
    coap->ReleaseSession(session);
    disconnected = true;
}

void Session::delete_pending_message()
{
    if (pending_message != nullptr)
    {
        delete[] pending_message->data;
        delete pending_message;
        pending_message = nullptr;
    }
}

const char* Session::session_str()
{
    return coap_session_str(session);
}

bool Session::send_message(const MessageQueue::Message& msg)
{
    // Create PDU
    auto pdu = coap_new_pdu(msg.type, msg.code, session);
    if (pdu == COAP_INVALID_RVALUE)
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not create pdu.");
        return false;
    }

    // Add Path
    std::stringstream ss_path(msg.uri.c_str());
    std::string uri_segment;
    while(std::getline(ss_path, uri_segment, '/'))
        coap_add_option(pdu,
                        COAP_OPTION_URI_PATH,
                        uri_segment.length(),
                        reinterpret_cast<const uint8_t *>(uri_segment.c_str()));

    // Add Data to PDU
    if (msg.data_size != 0)
    {
        // We prefer to always use block-wise transfer. This way our response handler will get called.
        unsigned char buf[4] = {};
        coap_add_option(pdu,
                        COAP_OPTION_BLOCK1,
                        coap_encode_var_safe(buf,
                                            sizeof(buf),
                                            ((0 << 4) | (0 << 3) | COAP_MAX_BLOCK_SZX)), // block.num = 0, block.m = (set by coap), block.size = max because reliable transmission
                        buf);

        // Copy data for block-wise transfer
        uint8_t* data_copy = new uint8_t[msg.data_size];
        std::memcpy(data_copy, msg.data, msg.data_size);

        // Add large data to PDU
        if (!coap_add_data_large_request(session, 
                                        pdu,
                                        msg.data_size, 
                                        data_copy, 
                                        [](coap_session_t* session, void* data) { delete[] (uint8_t*)data; },
                                        data_copy))
        {
            coap_delete_pdu(pdu);
            common::logging::log_error(std::cerr, LINE_INFORMATION, "Relay could not add data to PDU.");
            return false;
            
        }
    }

    // Get CoAP instance and send PDU
    auto coap = &common::CoAP::getInstance();
    coap_mid_t mid = coap->SendPDU(session, pdu, true);
    return mid != COAP_INVALID_MID;
}

void Session::EnqueueMessage(const MessageQueue::Message& msg)
{
    // Copy msg
    MessageQueue::Message* msg_copy = new MessageQueue::Message();
    *msg_copy = msg;

    // Copy data
    msg_copy->data = new u_int8_t[msg.data_size];
    std::memcpy(msg_copy->data, msg.data, msg.data_size);

    // Insert into transmit queue
    transmit_queue->Insert(msg_copy);
}

bool Session::IsConnected()
{
    return !disconnected && session != COAP_INVALID_RVALUE;
}

common::CoAP::session_ptr Session::GetRawSessionPtr()
{
    return session;
}

void Session::NotifySuccess()
{
    // Check for valid pending message
    if (pending_message == nullptr)
        return;
    common::logging::log_information(std::cout, LINE_INFORMATION, std::string("Server received the message successfully on ") + session_str());

    // Delete pending message
    delete_pending_message();
}

void Session::NotifyFailure()
{
    // Check for valid pending message
    if (pending_message == nullptr)
        return;
    common::logging::log_warning(std::cout, LINE_INFORMATION, std::string("A message could not be delivered on ") + session_str() + ". It will be put into the retransmit queue.");

    // Put the message in the retransmit queue
    retransmit_queue->Insert(pending_message);
    pending_message = nullptr;
}

bool Session::Transmit()
{
    // Check if there is a pending message
    if (pending_message != nullptr)
        return false;
    
    // Check if there are messages to be retransmitted or transmitted
    if (!retransmit_queue->IsEmpty())
        pending_message = retransmit_queue->Extract();
    else if (!transmit_queue->IsEmpty())
        pending_message = transmit_queue->Extract();

    // When there is no pending message we do not have to transmit anything
    if (pending_message == nullptr)
        return false;

    // Send the message
    if (!send_message(*pending_message))
    {
        // We could not send the message
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not send message when trying to transmit.");
        delete_pending_message();
        return false;
    }
    return true;
}

unsigned int Session::GetTransmitQueueCount()
{
    return transmit_queue->Size();
}

unsigned int Session::GetRetransmitQueueCount()
{
    return retransmit_queue->Size();
}