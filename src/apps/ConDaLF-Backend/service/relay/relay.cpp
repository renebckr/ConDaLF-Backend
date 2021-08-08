#include <cstring>
#include <iostream>
#include <sstream>
#include <common/logging/logging.h>
#include <common/config/parser.h>
#include <common/coap/coap.hpp>

#include "relay.hpp"
#include "session_manager.hpp"

using namespace condalf::service;

void Relay::configuration_line_handler(const std::string& line)
{
    if (line.size() == 0)
        return;
    
    std::string host, port;
    port = "5683";

    // Check if port is given else we assume standard port
    std::size_t pos = line.find_first_of(':', 0);
    if (pos != std::string::npos)
    {
        host = line.substr(0, pos);
        port = line.substr(pos + 1);
    }
    else
        host = line;
    
    // Skip if we already have read this before
    if (configuration_data.find(host + ":" + port) != configuration_data.end())
        return;

    // Create session and check for failure
    Session* session = new Session();
    if (!session->Connect(coap_context, host, port))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, std::string("Could not create relay session to ") + host + ":" + port);
        delete session;
        return;
    }

    // Add this session to our sessions and manage it with the session manager
    auto session_manager = &SessionManager::getInstance();
    if (!session_manager->ManageSession(coap_context, session))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Session Manager refused session.");
        delete session;
        return;
    }

    sessions.push_back(session);
}

bool Relay::enable_coap()
{
    // Create Context
    auto coap = &common::CoAP::getInstance();
    coap_context = coap->CreateContext(true, CONDALF_RELAY_KEEP_ALIVE_TIMEOUT);
    if (coap->context_descriptor_invalid(coap_context))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Relay could not create a coap context.");
        return false;
    }

    // Bind Context
    auto session_manager = &SessionManager::getInstance();
    if (!session_manager->BindContext(coap_context))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Relay could not bind coap context to session manager.");
        coap->ReleaseContext(coap_context);
        coap_context = -1;
        return false;
    }
    return true;
}

bool Relay::disable_coap()
{
    // Unbind Context
    auto session_manager = &SessionManager::getInstance();
    session_manager->UnbindContext(coap_context);

    // Release Context
    auto coap = &common::CoAP::getInstance();
    coap->ReleaseContext(coap_context);
    coap_context = -1;
    return true;
}

bool Relay::enable_relay()
{
    // Parse Config
    std::function<void(const std::string&)> line_handler(std::bind(&Relay::configuration_line_handler, this, std::placeholders::_1));
    try
    {
        common::config::parse(configuration_file, line_handler);
    }
    catch(const std::exception& e)
    {
        common::logging::log_warning(std::cout, LINE_INFORMATION, e.what());
    }

    // Clear configuration data used as a cache
    configuration_data.clear();
    return true;
}

bool Relay::disable_relay()
{
    // Get CoAP Instance
    auto coap = &common::CoAP::getInstance();

    // Remove all the relayed sessions
    for (auto session : sessions)
        delete session;
    sessions.clear();
    return true;
}

void Relay::run()
{
    // Get CoAP and SessionManager Instance
    auto coap = &common::CoAP::getInstance();
    auto session_manager = &SessionManager::getInstance();

    // Enqueue every message into all sessions
    while (!msg_queue->IsEmpty())
    {
        // Get the message and enqueue it
        MessageQueue::Message* msg = msg_queue->Extract();
        for (auto session : sessions)
        {
            session->EnqueueMessage(*msg);
            delete[] msg->data;
            delete msg;
        }
    }

    // Transmit messages
    for (auto session : sessions)
    {
        if (!session->IsConnected())
            session->Reconnect();
        session->Transmit(); // we are doing nothing with the rvalue yet
    }
    
    // Do IO with timeout of 500ms
    coap->IO(coap_context, 100);
}

Relay::Relay(MessageQueue* _msg_queue) : Service()
{
    this->service_name = "ConDaLF-Backend-Relay";
    this->msg_queue = _msg_queue;

    add_hook(std::bind(&Relay::enable_coap, this),
             std::bind(&Relay::disable_coap, this)
    );

    add_hook(std::bind(&Relay::enable_relay, this),
             std::bind(&Relay::disable_relay, this)
    );
}


bool Relay::Start(const std::string& _configuration_file)
{
    this->configuration_file = _configuration_file;
    return common::Service::Start();
}

bool Relay::Reload(const std::string& _configuration_file)
{
    this->configuration_file = _configuration_file;
    return common::Service::Reload();
}