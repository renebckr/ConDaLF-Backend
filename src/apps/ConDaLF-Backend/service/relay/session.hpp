/**
 * @file session.hpp
 * @author your name (you@domain.com)
 * @brief Session wrapper for the relay
 * @version 0.1
 * @date 2021-06-25
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <common/coap/coap.hpp>

#include "message_queue.hpp"

/**
 * TODO: What about a maximum queue size?
 * 
 */

namespace condalf::service
{
    class Session
    {
        private:
            /**
             * @brief The raw session pointer.
             */
            common::CoAP::session_ptr session;

            /**
             * @brief Context in which this session was created.
             */
            common::CoAP::context_descriptor context;

            /**
             * @brief Buffer messages to be sent.
             */
            MessageQueue* transmit_queue;

            /**
             * @brief Messages that were not delivered successfully are put here.
             */
            MessageQueue* retransmit_queue;

            /**
             * @brief The last message sent. (if nullptr -> a new msg can be sent)
             */
             MessageQueue::Message* pending_message;

            /**
             * @brief Host address
             */
            std::string host;

            /**
             * @brief Port to connect to.
             */
            std::string port;

            /**
             * @brief Flag set when the session is disconnected
             */
            bool disconnected;

            /**
             * @brief Deletes the pending message and sets it to nullptr.
             * 
             */
            void delete_pending_message();

            /**
             * @brief Get the session string.
             * 
             * @return const char* Session string
             */
            const char* session_str();

            /**
             * @brief Send the message over the session.
             * 
             * @param msg The message
             * @return true On success
             * @return false On failure
             */
            bool send_message(const MessageQueue::Message& msg);

        public:
            /**
             * @brief Construct a new Session object.
             */ 
            Session();

            /**
             * @brief Deleted copy constructor.
             */
            Session(const Session&) = delete;
            
            /**
             * @brief Destroy the Session object.
             */
            ~Session();

            /**
             * @brief Connect to the given host and port on the context
             * 
             * @param _context Context the session will be used in.
             * @param _host Host address
             * @param _port Port to connect to
             * @return true Could resolve address and create a session.
             * @return false Could not resolve address or create a session.
             */
            bool Connect(common::CoAP::context_descriptor _context, const std::string& _host, const std::string& _port);

            /**
             * @brief Reconnects to the once given host and port.
             * 
             * @return true On success
             * @return false On failure of the Connect() method
             */
            bool Reconnect();

            /**
             * @brief Disconnect the session.
             */
            void Disconnect();

            /**
             * @brief Enqueues the message into the transmit queue
             * 
             * @param msg The message to be sent
             */
            void EnqueueMessage(const MessageQueue::Message& msg);

            /**
             * @brief Get the is connected flag
             * 
             * @return true When session is connected. Will not guarantee that it is reachable.
             * @return false When session is disconnected.
             */
            bool IsConnected();

            /**
             * @brief Get the Raw Session Pointer.
             * 
             * @return common::CoAP::session_ptr The session pointer.
             */
            common::CoAP::session_ptr GetRawSessionPtr();

            /**
             * @brief Notify to the session that the last sent message was successfully received.
             * The session will then try to transmit the next message.
             */
            void NotifySuccess();

            /**
             * @brief Notify to the session that the last sent message failed to be delivered.
             * The session will try to retransmit the Message.
             * 
             */
            void NotifyFailure();

            /**
             * @brief Transmit the next message if possible. (Will first try to retransmit)
             * 
             * @return true When a message got transmitted.
             * @return false When no message got transmitted.
             */
            bool Transmit();

            /**
             * @brief Get the Transmit Queue Count
             * 
             * @return unsigned int Message count of transmit queue
             */
            unsigned int GetTransmitQueueCount();

            /**
             * @brief Get the Retransmit Queue Count
             * 
             * @return unsigned int Message count of retransmit queue
             */
            unsigned int GetRetransmitQueueCount();
    };
}