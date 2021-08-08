/**
 * @file session_manager.hpp
 * @author your name (you@domain.com)
 * @brief CoAP Session Manager for relays
 * @version 0.1
 * @date 2021-06-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <common/coap/coap.hpp>
#include <cstdint>
#include <set>
#include <unordered_map>
#include <unordered_set>
#include <type_traits>

#include "session.hpp"

// Evil Singleton but required because CoAP handlers have to be static
namespace condalf::service
{
    class SessionManager
    {
        public:
            /**
             * @brief Get the Singleton Instance
             * 
             * @return SessionManager&
             */
            static SessionManager &getInstance()
            {
                static SessionManager instance;
                return instance;
            }

        private:
            /**
             * @brief Mutex for the context_set.
             */
            std::mutex context_set_mutex;

            /**
             * @brief Ordered set of all bound context.
             */
            std::set<common::CoAP::context_descriptor> context_set;

            /**
             * @brief Mutex for the context_sessions_map.
             */
            std::mutex context_sessions_map_mutex;

            /**
             * @brief Mapping of contexts to their sessions.
             */
            std::unordered_map<common::CoAP::context_descriptor, std::unordered_set<Session*>> context_sessions_map;

            /**
             * @brief Mutex for the coap_session_map
             */
            std::mutex coap_session_map_mutex;

            /**
             * @brief Ordered set of all managed sessions.
             */
            std::unordered_map<common::CoAP::session_ptr, Session*> coap_session_map;

            /**
             * @brief Construct a new SessionManager object
             */
            SessionManager() {}

            /**
             * @brief Destroy the SessionManager object
             */
            ~SessionManager() {}

            /**
             * @brief Removes all sessions that were from the given context
             * 
             * @param context The context
             */
            void remove_all_sessions(common::CoAP::context_descriptor context);
        public:
            /**
             * @brief Deleted move construction
             */
            SessionManager(const SessionManager&) = delete;

            /**
             * @brief Deleted move operator
             */
            SessionManager& operator=(const SessionManager&) = delete;


            bool BindContext(common::CoAP::context_descriptor context);
            void UnbindContext(common::CoAP::context_descriptor context);
            bool ManageSession(common::CoAP::context_descriptor context, Session* session);
            Session* FindSession(common::CoAP::session_ptr session_ptr);
    };
}