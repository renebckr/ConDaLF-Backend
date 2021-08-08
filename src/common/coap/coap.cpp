/**
 * @file coap.cpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief 
 * @version 0.1
 * @date 2021-06-01
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#include "coap.hpp"

#include <chrono>
#include <common/logging/logging.h>
#include <cstdint>
#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <fstream>
#include <sys/types.h>

using namespace common;

bool resolve_address(const std::string &host, const std::string &port, coap_address_t *destination)
{
    struct addrinfo *addresses, *address_information;
    struct addrinfo hints = {0};
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_family = AF_UNSPEC;
    *destination = {0};

    int result = getaddrinfo(host.c_str(), port.c_str(), &hints, &addresses);
    if (result != 0)
    {
        logging::log_error(std::cerr, LINE_INFORMATION, gai_strerror(result));
        return false;
    }

    for (address_information = addresses; address_information != NULL; address_information = address_information->ai_next)
    {
        if (address_information->ai_family == AF_INET || address_information->ai_family == AF_INET6)
        {
            destination->size = address_information->ai_addrlen;
            memcpy(&destination->addr.sin6, address_information->ai_addr, destination->size);

            freeaddrinfo(addresses);
            return true;
        }
    }
    freeaddrinfo(addresses);
    return false;
}

bool CoAP::context_descriptor_invalid(context_descriptor context)
{
    std::lock_guard guard(coap_lock); // Thread-safety

    // Check if in bound
    if (context == -1 || context >= context_list.size())
        return true;
    
    // Check if not invalid
    if (context_list.at(context) == COAP_INVALID_RVALUE)
        return true;
    
    // It is valid
    return false;
}

CoAP::CoAP()
{
    logging::log_information(std::cout, LINE_INFORMATION, "CoAP Startup");
    coap_startup();
}

CoAP::~CoAP()
{
    std::lock_guard guard(coap_lock);

    // Free every context
    for (auto context : context_list)
        if (context != COAP_INVALID_RVALUE)
            coap_free_context(context);
    context_list.clear();

    logging::log_information(std::cout, LINE_INFORMATION, "CoAP Cleanup");
    coap_cleanup();
}

CoAP::context_descriptor CoAP::CreateContext(bool use_libcoap_block_mode, unsigned int keep_alive_timeout, const coap_address_t *listen_addr)
{
    CoAP::context_descriptor descriptor = -1;
    coap_context_t *context = coap_new_context(listen_addr);

    // Return an invalid context_descriptor when call failed
    if (context == nullptr)
    {
        logging::log_error(std::cerr, LINE_INFORMATION, "Context could not be created.");
        return descriptor;
    }

    // Set Blockmode
    if (use_libcoap_block_mode)
        coap_context_set_block_mode(context, COAP_BLOCK_USE_LIBCOAP);
    
    // Set keep-alive
    if (keep_alive_timeout != 0)
        coap_context_set_keepalive(context, keep_alive_timeout);

    // Thread safety
    std::lock_guard guard(coap_lock);

    // Check if we have a free spot
    for (unsigned int i = 0; i < context_list.size(); i++)
    {
        if (context_list[i] == COAP_INVALID_RVALUE)
        {
            // We can insert it
            descriptor = i;
            context_list[descriptor] = context;
            return descriptor;
        }
    }

    // Otherwise add to context list and return descriptor
    descriptor = context_list.size();
    context_list.push_back(context);
    return descriptor;
}

void CoAP::ReleaseContext(context_descriptor descriptor)
{
    // Check if valid
    if (context_descriptor_invalid(descriptor))
        return;
    
    // Thread-safety
    std::lock_guard guard(coap_lock);

    // Free and invalidate
    auto context = context_list.at(descriptor);
    coap_free_context(context);
    context_list[descriptor] = COAP_INVALID_RVALUE;
    return;
}

bool CoAP::CreateEndpoint(context_descriptor context, const std::string &host, const std::string &port)
{
    // Check for invalid context
    if (context_descriptor_invalid(context))
        return false;

    // Resolve the address
    coap_address_t coap_addr;
    if (!resolve_address(host, port, &coap_addr))
    {
        logging::log_error(std::cerr, LINE_INFORMATION, "Could not resolve address.");
        return false;
    }
    
    // Thread-safety
    std::lock_guard guard(coap_lock);

    // New endpoint and check if valid
    if (coap_new_endpoint(context_list.at(context), &coap_addr, COAP_PROTO_UDP) == nullptr)
    {
        logging::log_error(std::cerr, LINE_INFORMATION, "Could not create endpoint.");
        return false;
    }
    return true;
}

CoAP::session_ptr CoAP::CreateSession(context_descriptor context, const std::string &host, const std::string &port)
{
    // Check for invalid context
    if (context_descriptor_invalid(context))
        return nullptr;
    
    // Resolve the address
    coap_address_t coap_addr;
    if (!resolve_address(host, port, &coap_addr))
    {
        logging::log_error(std::cerr, LINE_INFORMATION, "Could not resolve address.");
        return nullptr;
    }
    
    // Thread-safety
    std::lock_guard guard(coap_lock);

    // Create Session
    session_ptr session = coap_new_client_session(context_list.at(context), nullptr, &coap_addr, COAP_PROTO_UDP);
    if (session == COAP_INVALID_RVALUE)
        logging::log_error(std::cerr, LINE_INFORMATION, "Could not create session.");
    return session;
}

coap_mid_t CoAP::SendPDU(session_ptr session, struct coap_pdu_t* pdu, bool large)
{
    if (session == COAP_INVALID_RVALUE)
        return COAP_INVALID_MID;
    
    if (large)
        return coap_send_large(session, pdu);
    return coap_send(session, pdu);
}

void CoAP::ReleaseSession(session_ptr session)
{
    // Check if session valid
    if (session != COAP_INVALID_RVALUE)
        coap_session_release(session);
}

bool CoAP::RegisterResponseHandler(context_descriptor context, coap_response_handler_t handler)
{
    // Check for invalid context
    if (context_descriptor_invalid(context))
        return false;
    
    // Thread-safety
    std::lock_guard guard(coap_lock);
    
    coap_register_response_handler(context_list.at(context), handler);
    return true;
}

bool CoAP::RegisterNackHandler(context_descriptor context, coap_nack_handler_t handler)
{
    // Check for invalid context
    if (context_descriptor_invalid(context))
        return false;
    
    // Thread-safety
    std::lock_guard guard(coap_lock);
    
    coap_register_nack_handler(context_list.at(context), handler);
    return true;
}

bool CoAP::RegisterPongHandler(context_descriptor context, coap_pong_handler_t handler)
{
    // Check for invalid context
    if (context_descriptor_invalid(context))
        return false;
    
    // Thread-safety
    std::lock_guard guard(coap_lock);
    
    coap_register_pong_handler(context_list.at(context), handler);
    return true;
}


std::vector<uint8_t> CoAP::ResourceBlockHandler(resource_ptr resource,
                                                coap_session_t *session,
                                                const coap_pdu_t *request,
                                                coap_pdu_t *response)
{
    coap_block_t block1;
    size_t len = 0;
    uint8_t *data = nullptr;

    {
        // Thread-safety
        std::lock_guard guard(coap_lock);

        // Clear Cache of timeouts
        for (auto it = block_cache.begin(); it != block_cache.end();)
        {
            if (std::chrono::duration_cast<std::chrono::seconds>(
                std::chrono::high_resolution_clock::now() - it->second.last_modified).count() > COAP_RESOURCE_BLOCK_TIMEOUT)
            {
                block_cache.erase(it);
            }
            else
                it++;
        }
    }

    // Get Content
    if (!coap_get_data(request, &len, (const uint8_t **)&data))
    {
        logging::log_warning(std::cout, LINE_INFORMATION, "Empty Message Retrieved. Data might have been expected here.");
        return std::vector<uint8_t>();
    }

    // Put data into a vector
    std::vector<uint8_t> data_vector = std::vector<uint8_t>(data, data + len);

    // Check for block
    if (!coap_get_block(request, COAP_OPTION_BLOCK1, &block1))
        return data_vector; // Return whole data if there is no block

    // Generate key for the Block Cache
    coap_str_const_t *uri = coap_resource_get_uri_path(resource);
    std::string key = coap_session_str(session);
    key.append((const char *)uri->s, uri->length);

    // Create entry structure
    block_cache_entry entry = {
        .last_modified = {},
        .last_block = {},
        .first_block_mid = 0,
        .data = std::vector<uint8_t>()};
    
     // Add Block1 opt to ack
    unsigned char buf[4] = {};
    coap_add_option(response,
                    COAP_OPTION_BLOCK1,
                    coap_encode_var_safe(buf,
                                         sizeof(buf),
                                         ((block1.num << 4) | (block1.m << 3) | block1.szx)),
                    buf);

    // Thread-safety
    std::lock_guard guard(coap_lock);

    // Get the first message id for this block
    coap_mid_t first_mid = coap_pdu_get_mid(request) - block1.num;

    // Find the last block-wise transfer
    auto cache_entry = block_cache.find(key);

    // Set response code
    coap_pdu_set_code(response, block1.m ? COAP_RESPONSE_CODE_CONTINUE : COAP_RESPONSE_CODE_CHANGED);

    // We have the start of a new block-wise transfer
    if (block1.num == 0)
    {
        // Check if it has already been received before
        if (cache_entry != block_cache.end())
        {
            // We should not process the data again - An ACK probably went missing
            if (cache_entry->second.first_block_mid == first_mid)
                return std::vector<uint8_t>();
    
            // Erase last cache entry
            block_cache.erase(cache_entry);
        }

        // Create new Cache Entry
        entry.last_block = block1;
        entry.last_modified = std::chrono::high_resolution_clock::now();
        entry.first_block_mid = first_mid;
        entry.data.insert(entry.data.end(), data_vector.begin(), data_vector.end());
        block_cache[key] = entry;

        // Return data when there is no more data
        return block1.m ? std::vector<uint8_t>() : entry.data;
    }
    else if (cache_entry == block_cache.end())
    {
        // The block1.num is not 0 
        // -- but cannot find an entry in the cache
        // -> We cannot really do anything here. We can only request the client to resend the whole request
        logging::log_warning(std::cout, LINE_INFORMATION, "Received first block with non-zero id. Request incomplete.");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_INCOMPLETE);
        return std::vector<uint8_t>();
    }
    else if (cache_entry->second.first_block_mid != first_mid)
    {
        // block1.num > 0 && cache entry found 
        // -- but mid is different
        // -> We need to request the client to resend whole request
        logging::log_warning(std::cout, LINE_INFORMATION, "Cannot append data because the first\nmessage id of the cache entry is different. Request incomplete.");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_INCOMPLETE);
        return std::vector<uint8_t>();
    }
    else if (cache_entry->second.last_block.num == block1.num)
    {
        // block1.num > 0 && cache entry found && mid matches
        // -- but last block num is current num
        // -> We simply ACK it as the ACK went missing
        logging::log_information(std::cout, LINE_INFORMATION, "A client sent the same block because an ACK went missing. Ignoring block.");
        return std::vector<uint8_t>();
    }
    else if (cache_entry->second.last_block.num != block1.num - 1)
    {
        // block1.num > 0 && cache entry found && mid matches && num not the same
        // -- but block num differs from the expected one
        // -> We request the client to retransmit the request as something must've gone terribly wrong
        logging::log_information(std::cout, LINE_INFORMATION, "A client sent a block in an unexpected order. Request incomplete");
        coap_pdu_set_code(response, COAP_RESPONSE_CODE_INCOMPLETE);
        return std::vector<uint8_t>();
    }
    else if (!cache_entry->second.last_block.m && !block1.m)
    {
        // block1.num > 0 && cache entry found && mid matches && num correct
        // -- but this request has two ends?
        // -> We will have to ignore this block to preserve data integrity
        logging::log_information(std::cout, LINE_INFORMATION, "A client sent two blocks marked as end in a request. Ignoring block.");

        // Update cache entry data
        cache_entry->second.last_block = block1;
        return std::vector<uint8_t>();
    }
    else
    {
        // block1.num > 0 && cache entry found && mid matches && num correct && not a double end
        // -> We can insert this block into our cache entry

        // Append and update cache entry data
        cache_entry->second.last_block = block1;
        cache_entry->second.last_modified = std::chrono::high_resolution_clock::now();
        cache_entry->second.data.insert(cache_entry->second.data.end(), data_vector.begin(), data_vector.end());

        // Return the data when valid
        return block1.m ? std::vector<uint8_t>() : cache_entry->second.data;
    }

    // Cannot be called
    return std::vector<uint8_t>();
}

CoAP::resource_ptr CoAP::CreateResource(const std::string &URI, int flags)
{
    CoAP::resource_ptr res = coap_resource_init(coap_make_str_const(URI.c_str()), flags);
    if (res == NULL)
        logging::log_error(std::cerr, LINE_INFORMATION, "CoAP Resource could not be initialized.");
    return res;
}

bool CoAP::RegisterResourceHandler(resource_ptr res, coap_request_t type, coap_method_handler_t handler)
{
    if (res == nullptr || handler == nullptr)
        return false;

    coap_register_handler(res, type, handler);
    return true;
}

bool CoAP::AddResource(context_descriptor context, resource_ptr res)
{
    if (context_descriptor_invalid(context))
        return false;
    
    // Thread-safety
    std::lock_guard guard(coap_lock);

    coap_add_resource(context_list.at(context), res);
    return true;
}

void CoAP::IO_Loop(context_descriptor context)
{
    if (context_descriptor_invalid(context))
        return;
    
    coap_lock.lock();
    auto coap_context = context_list.at(context);
    coap_lock.unlock();

    while (true)
    {
        coap_io_process(coap_context, COAP_IO_WAIT);
    }
}

void CoAP::IO(context_descriptor context, size_t timeout)
{
    if (context_descriptor_invalid(context))
        return;

    coap_lock.lock();
    auto coap_context = context_list.at(context);
    coap_lock.unlock();

    coap_io_process(coap_context, timeout);
}