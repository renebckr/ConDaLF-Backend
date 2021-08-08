#include "session_manager.hpp"

#include <common/logging/logging.h>
#include <algorithm>
#include <iostream>
#include <mutex>
#include <unordered_set>

using namespace condalf::service;

COAP_RESPONSE_HANDLER(response_handler)
{
    // Check for block1 (then this was a data transmit for the relay)
    coap_block_t block1;
    if (!coap_get_block(sent, COAP_OPTION_BLOCK1, &block1))
    {
        // Block mode ought to be enabled -> return error
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Reponse Handler caught PDU without block1.");
        return COAP_RESPONSE_FAIL;
    }

    // Notify the session
    auto session_manager = &SessionManager::getInstance();
    Session* s = session_manager->FindSession(session);
    s->NotifySuccess();
    return COAP_RESPONSE_OK;
}

/**
 * @brief Handles pong messages. Resets retry count for the session reconnection attempts
 * 
 */
COAP_PONG_HANDLER(pong_handler)
{
    // Search for session
    auto session_manager = &SessionManager::getInstance();
    Session* relay_session = session_manager->FindSession(session);
    if (relay_session != nullptr) // Found session
    {
        // NOT REQUIRED RIGHT NOW
        // Reset retries because the ping worked now
        // relay_session->retries = 0;
        return;
    }

    common::logging::log_information(std::cout, LINE_INFORMATION, std::string("Received Pong from unknown session ") + coap_session_str(session));
}

/**
 * @brief NACK Handler that handles all errors when a packet could not be ack'd.
 * 
 */
COAP_NACK_HANDLER(nack_handler)
{
    auto session_manager = &SessionManager::getInstance();
    Session* relay_session = session_manager->FindSession(session);
    auto session_str = coap_session_str(session);

    // Check for block1 (then this was a data transmit for the relay)
    coap_block_t block1;
    bool data_transmit = coap_get_block(sent, COAP_OPTION_BLOCK1, &block1);

    switch (reason)
    {
    case COAP_NACK_TOO_MANY_RETRIES:
        common::logging::log_warning(std::cout, 
                                     LINE_INFORMATION, 
                                     std::string("Too many retries for PDU on ") + session_str);
        if (relay_session != nullptr)
            relay_session->Disconnect();
        break;
    case COAP_NACK_NOT_DELIVERABLE: // Happens when we lose connection
        common::logging::log_warning(std::cout, 
                                     LINE_INFORMATION, 
                                     std::string("PDU not deliverable on ") + session_str);
        if (relay_session != nullptr)
            relay_session->Disconnect();
        break;
    case COAP_NACK_RST:
        common::logging::log_warning(std::cout, 
                                         LINE_INFORMATION, 
                                         std::string("Reset Packet for PDU on ") + session_str);
        break;
    case COAP_NACK_TLS_FAILED:
        common::logging::log_warning(std::cout, 
                                     LINE_INFORMATION, 
                                     std::string("TLS failed on ") + session_str);
        break;
    case COAP_NACK_ICMP_ISSUE:
        common::logging::log_warning(std::cout, 
                                     LINE_INFORMATION, 
                                     std::string("ICMP issue on ") + session_str);
        break;
    
    default:
        common::logging::log_warning(std::cout, 
                                     LINE_INFORMATION, 
                                     std::string("Could not send PDU on ") + session_str + std::string(", unknown issue."));
        break;
    }

    // Notify session when it was a data transmit
    if (data_transmit)
        relay_session->NotifyFailure();

    return;
}

void SessionManager::remove_all_sessions(common::CoAP::context_descriptor context)
{
    // Lock Ressources
    std::lock_guard guard_context_sessions_map(context_sessions_map_mutex);
    std::lock_guard guard_coap_session_map(coap_session_map_mutex);

    // Try to find the context_sessions
    auto it = context_sessions_map.find(context);
    if (it == context_sessions_map.end())
        return;
    
    // Remove every session from our data structures
    for (auto session : it->second)
    {
        // Try to find the session in our coap map and remove when found
        auto it_session = coap_session_map.find(session->GetRawSessionPtr());
        if (it_session != coap_session_map.end())
            coap_session_map.erase(it_session);
    }

    // Clear sessions and erase it
    it->second.clear();
    context_sessions_map.erase(it);
}

bool SessionManager::BindContext(common::CoAP::context_descriptor context)
{
    // Lock Ressources
    std::lock_guard guard_context_set(context_set_mutex);
    std::lock_guard guard_context_sessions_map(context_sessions_map_mutex);

    // Check if we already have this context
    if (context_set.find(context) != context_set.end())
    {
        common::logging::log_warning(std::cout, LINE_INFORMATION, "BindContext called on bound context.");
        return false;
    }

    // Get CoAP instance
    auto coap = &common::CoAP::getInstance();

    // Register Response Handler
    if (!coap->RegisterResponseHandler(context, response_handler))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "SessionManager could not register a Response handler.");
        return false;
    }

    // Register NACK Handler
    if (!coap->RegisterNackHandler(context, nack_handler))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "SessionManager could not register a NACK handler.");
        return false;
    }

    // Register Pong Handler
    if (!coap->RegisterPongHandler(context, pong_handler))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "SessionManager could not register a Pong handler.");
        return false;
    }

    // Insert context into data structures
    context_set.insert(context);
    if (context_sessions_map.find(context) == context_sessions_map.end())
        context_sessions_map[context] = std::unordered_set<Session*>();
    return true;
}

void SessionManager::UnbindContext(common::CoAP::context_descriptor context)
{
    // Lock Ressources
    std::lock_guard guard_context_set(context_set_mutex);

    // Find context
    auto it = context_set.find(context);
    if (it == context_set.end())
    {
        common::logging::log_warning(std::cout, LINE_INFORMATION, "UnbindContext called on unbound context.");
        return;
    }

    // Get CoAP Instance
    auto coap = &common::CoAP::getInstance();

    // Unbind handlers
    coap->RegisterNackHandler(context, nullptr);
    coap->RegisterPongHandler(context, nullptr);
    coap->RegisterResponseHandler(context, nullptr);
    
    // Remove context and sessions from data structures
    context_set.erase(it);
    remove_all_sessions(context);
    return;
}

bool SessionManager::ManageSession(common::CoAP::context_descriptor context, Session* session)
{
    // Lock Ressources
    std::lock_guard guard_context_set(context_set_mutex);
    std::lock_guard guard_context_sessions_map(context_sessions_map_mutex);
    std::lock_guard guard_coap_session_map(coap_session_map_mutex);

    // Check if the context is bound
    if (!std::binary_search(context_set.begin(), context_set.end(), context))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "A context has to be bound to the session manager before sessions may be managed by it.");
        return false;
    }

    // Get the appropriate session set
    auto it = context_sessions_map.find(context);
    if (it == context_sessions_map.end())
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Session bound but not in internal mapping structure. Internal error.");
        return false;
    }

    // Insert into data structures
    it->second.insert(session);
    coap_session_map[session->GetRawSessionPtr()] = session;
    return true;
}

Session* SessionManager::FindSession(common::CoAP::session_ptr session_ptr)
{
    // Lock Ressources
    std::lock_guard guard_coap_session_map(coap_session_map_mutex);

    auto it = coap_session_map.find(session_ptr);
    if (it != coap_session_map.end())
        return it->second;
    return nullptr;
}
