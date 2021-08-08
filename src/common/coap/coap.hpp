/**
 * @file coap.hpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief CoAP Wrapper
 * @version 0.1
 * @date 2021-05-29
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <coap3/coap.h>
#include <iostream>
#include <vector>
#include <unordered_map>
#include <mutex>
#include <chrono>

#define COAP_INVALID_RVALUE nullptr
#define COAP_RESOURCE_BLOCK_TIMEOUT 60000 //60 seconds

#define COAP_RESOURCE_HANDLER(fnc_name)             \
    void fnc_name(struct coap_resource_t *resource, \
                  coap_session_t *session,          \
                  const coap_pdu_t *request,        \
                  const coap_string_t *query,       \
                  coap_pdu_t *response)

#define COAP_RESPONSE_HANDLER(fnc_name)                     \
    coap_response_t fnc_name(coap_session_t * session,      \
                             const coap_pdu_t *sent,        \
                             const coap_pdu_t *received,    \
                             const coap_mid_t mid)

#define COAP_NACK_HANDLER(fnc_name)                 \
    void fnc_name(coap_session_t* session,          \
                  const coap_pdu_t* sent,           \
                  const coap_nack_reason_t reason,  \
                  const coap_mid_t mid)

#define COAP_PONG_HANDLER(fnc_name)             \
    void fnc_name(coap_session_t* session,      \
                  const coap_pdu_t* received,   \
                  const coap_mid_t mid)

namespace common
{
    class CoAP
    {
    public:
        using context_descriptor = int;
        using resource_ptr = struct coap_resource_t *;
        using session_ptr = struct coap_session_t *;

        /**
         * @brief Get the Singleton Instance
         * 
         * @return CoAP& 
         */
        static CoAP &getInstance()
        {
            static CoAP instance;
            return instance;
        }

    private:
        struct block_cache_entry
        {
            std::chrono::high_resolution_clock::time_point last_modified;
            coap_block_t last_block; // We expect in order
            coap_mid_t first_block_mid;
            std::vector<uint8_t> data;
        };

        /**
         * @brief When accessing data structures we need to be carefull when multiple threads are running
         */
        std::mutex coap_lock;

        /**
         * @brief Context list containing all coap contexts
         * 
         */
        std::vector<coap_context_t *> context_list;

        /**
         * @brief key is the session name + uri
         */
        std::unordered_map<std::string, block_cache_entry> block_cache;

        /**
         * @brief Construct a new CoAP object (private because it's a Singleton)
         * 
         */
        CoAP();

        /**
         * @brief Destroy the Singleton at the end of the application
         * 
         */
        ~CoAP();

    public:
        /**
         * @brief Deleted move constructor
         * 
         */
        CoAP(CoAP const &) = delete;

        /**
         * @brief Deleted assign operator
         * 
         */
        void operator=(CoAP const &) = delete;


        /**
         * @brief Checks if the descriptor is valid
         * 
         * @param context descriptor
         * @return true Valid
         * @return false Invalid
         */
        bool context_descriptor_invalid(context_descriptor context);

        /**
         * @brief Create a Context 
         * 
         * @param use_libcoap_block_mode Use Libcoap Block mode?
         * @param keep_alive_timeout    After how many seconds should the next ping be send
         * @param listen_addr           Address to listen to (can also be handed over by creating your own endpoint)
         * @return context_descriptor   Descriptor used by this class
         */
        context_descriptor CreateContext(bool use_libcoap_block_mode = false, unsigned int keep_alive_timeout = 0, const coap_address_t *listen_addr = nullptr);

        /**
         * @brief Releases a context
         * 
         * @param descriptor 
         */
        void ReleaseContext(context_descriptor descriptor);

        /**
         * @brief Create an Endpoint for a context
         * 
         * @param context Context to assign the endpoint to
         * @param host host address
         * @param port port
         * @return true Successfully added endpoint
         * @return false Failure
         */
        bool CreateEndpoint(context_descriptor context, const std::string &host, const std::string &port);

        /**
         * @brief Creates a client session
         * 
         * @param context Context to assign the session to
         * @param host host address
         * @param port port
         * @return session_ptr nullptr on failure
         */
        session_ptr CreateSession(context_descriptor context, const std::string &host, const std::string &port);

        /**
         * @brief Releases the session
         * 
         * @param session The session to be released
         */
        void ReleaseSession(session_ptr session);

        /**
         * @brief Send a PDU over this session. Note that you will have to run IO() to get it sent and handled.
         * 
         * @param session The session to which it will be sent
         * @param pdu     The PDU to be sent
         * @param large   Is it a large request?
         * @return coap_mid_t Message id. COAP_INVALID_MID if failed
         */
        coap_mid_t SendPDU(session_ptr session, struct coap_pdu_t* pdu, bool large = false);

        /**
         * @brief Register a response handler for a given context
         * 
         * @param context The context
         * @param handler The response handler
         * @return true On Success
         * @return false On Failure
         */
        bool RegisterResponseHandler(context_descriptor context, coap_response_handler_t handler);

        /**
         * @brief Register a NACK handler for a given context
         * 
         * @param context The context
         * @param handler The NACK handler
         * @return true On Success
         * @return false On Failure
         */
        bool RegisterNackHandler(context_descriptor context, coap_nack_handler_t handler);

        /**
         * @brief Register a Pong handler for a given context
         * 
         * @param context The context
         * @param handler The NACK handler
         * @return true On Success
         * @return false On Failure
         */
        bool RegisterPongHandler(context_descriptor context, coap_pong_handler_t handler);

        /**
         * @brief Handles block-wise communication
         * 
         * @param resource The CoAP resource
         * @param session This session used to identify to which client the blocks belong to
         * @param request The CoAP request
         * @param response CoAP response (will be modified according to the standard)
         * @return std::vector<uint8_t> Data
         */
        std::vector<uint8_t> ResourceBlockHandler(resource_ptr resource, coap_session_t *session, const coap_pdu_t *request, coap_pdu_t *response);

        //maybe put these following together later
        resource_ptr CreateResource(const std::string &URI, int flags = 0);
        bool RegisterResourceHandler(resource_ptr res, coap_request_t type, coap_method_handler_t handler);
        bool AddResource(context_descriptor context, resource_ptr res);
        // ------

        /**
         * @brief Runs basic CoAP IO Loop
         * TODO: Maybe do this a bit better
         */
        void IO_Loop(context_descriptor context);

        /**
         * @brief Runs IO
         * 
         * @param context Which context the IO runs on
         * @param timeout How long to wait for messages until returning
         */
        void IO(context_descriptor context, size_t timeout = COAP_IO_WAIT);
    };
}