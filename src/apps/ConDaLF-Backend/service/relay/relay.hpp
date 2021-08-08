/**
 * @file relay.hpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief Relay Service for ConDaLF
 * @version 0.1
 * @date 2021-06-16
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <queue>
#include <unordered_set>
#include <unordered_map>
#include <common/coap/coap.hpp>
#include <common/service/service.hpp>

#include "session.hpp"

#define CONDALF_RELAY_KEEP_ALIVE_TIMEOUT 10 // seconds

namespace condalf::service
{
    class Relay : private common::Service
    {
        public:
            using common::Service::IsActive;
            using common::Service::Stop;
            using common::Service::Reload;

        private:
            /**
             * @brief The coap context being used for the coap client
             */
            common::CoAP::context_descriptor coap_context;

            /**
             * @brief Relay configuration file containing all relay endpoints
             */
            std::string configuration_file;

            /**
             * @brief Used as a cache while reading the configuration to avoid duplicates
             */
            std::unordered_set<std::string> configuration_data;

            /**
             * @brief The Message queue used for queueing messages.
             */
            MessageQueue* msg_queue;

            /**
             * @brief All the sessions that this relay has.
             */
            std::vector<Session*> sessions;

            /**
             * @brief Handles a read line from the configuration.
             * 
             * @param line Line that got read
             */
            void configuration_line_handler(const std::string& line);

            /**
             * @brief Inits CoAP for the Server
             * 
             * @return true On success 
             * @return false On failure
             */
            bool enable_coap();

            /**
             * @brief Destroys CoAP for the Server
             * 
             * @return true On success
             * @return false On failure
             */
            bool disable_coap();

            /**
             * @brief Inits the relay itself
             * 
             * @return true On success
             * @return false On failure
             */
            bool enable_relay();

            /**
             * @brief Destroys the relay
             * 
             * @return true On success
             * @return false On failure
             */
            bool disable_relay();

        protected:
            /**
             * @brief Run function for the server service
             * 
             */
            void run();

        public:
            /**
             * @brief Construct the Relay object
             */
            Relay(MessageQueue* _msg_queue);
            
            /**
             * @brief Deleted move construction
             */
            Relay(const Relay&) = delete;

            /**
             * @brief Destroy the Relay object
             */
            ~Relay() {}

            /**
             * @brief Start the Relay Service
             * 
             * @param _configuration_file The relay configuration
             * @return true On success
             * @return false On failure
             */
            bool Start(const std::string& _configuration_file);

            /**
             * @brief Reloads the Relay Service
             * 
             * @param _configuration_file The relay configuration
             * @return true On success
             * @return false On failure
             */
            bool Reload(const std::string& _configuration_file);
    };
}