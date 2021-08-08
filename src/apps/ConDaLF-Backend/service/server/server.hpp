/**
 * @file server.hpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief The Server Service
 * @version 0.1
 * @date 2021-06-15
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <common/service/service.hpp>
#include <common/coap/coap.hpp>
#include <apps/ConDaLF-Backend/service/relay/message_queue.hpp>

// TODO: Write Documentation

namespace condalf::service
{
    class Server : private common::Service
    {
        public:
            using common::Service::IsActive;
            using common::Service::Stop;
            using common::Service::Reload;

            /**
             * @brief Get the Singleton Instance
             * 
             * @return Server&
             */
            static Server &getInstance()
            {
                static Server instance;
                return instance;
            }

        private:
            /**
             * @brief True if python processing should be enabled and false if not
             */
            bool python_enabled;
            
            /**
             * @brief The python script module
             */
            std::string python_script;
            
            /**
             * @brief Host address for the server
             */
            std::string host;
            
            /**
             * @brief Port to listen to
             */
            std::string port;
            
            /**
             * @brief The coap context being used for the coap server
             */
            common::CoAP::context_descriptor coap_context;
            
            /**
             * @brief The /condalf/data resource
             */
            common::CoAP::resource_ptr coap_condalf_data_res;

            /**
             * @brief The /condalf/test resource
             */
            common::CoAP::resource_ptr coap_condalf_test_res;
            
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
             * @brief Inits Python Processing for the Server
             * 
             * @return true On success
             * @return false On failure
             */
            bool enable_python();

            /**
             * @brief Destroys Python Processing for the Server
             * 
             * @return true On success
             * @return false On failure
             */
            bool disable_python();

            /**
             * @brief Construct a new Server object
             * 
             */
            Server();

            /**
             * @brief Destroy the Server object
             */
            ~Server() {}
        
        protected:
            /**
             * @brief Run function for the server service
             * 
             */
            void run();

        public:
            /**
             * @brief Deleted move construction
             */
            Server(const Server&) = delete;

            /**
             * @brief Deleted move operator
             */
            Server& operator=(const Server&) = delete;

            /**
             * @brief Starts the Server Service
             * 
             * @param _host Host Address (usually 0.0.0.0)
             * @param _port Port to listen to
             * @param _msg_queue nullptr if we do not relay messages
             * @param enable_python_script True of python processing should be used
             * @param _script_file Script file for python processing
             * 
             * @return true On Success
             * @return false On failure
             */
            bool Start(const std::string& _host,
                       const std::string& _port = "5683",
                       MessageQueue* _msg_queue = nullptr,
                       bool enable_python_script = false, 
                       const std::string& _script_file = "");

            /**
             * @brief Reloads the Server
             * 
             * @param _host Host Address (usually 0.0.0.0)
             * @param _port Port to listen to
             * @param _msg_queue nullptr if we do not relay messages
             * @param enable_python_script True of python processing should be used
             * @param _script_file Script file for python processing
             * 
             * @return true On success
             * @return false On failure
             */
            bool Reload(const std::string& _host,
                        const std::string& _port = "5683",
                        MessageQueue* _msg_queue = nullptr,
                        bool enable_python_script = false, 
                        const std::string& _script_file = "");
    };
}