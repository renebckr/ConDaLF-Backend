/**
 * @file message_queue.hpp
 * @author your name (you@domain.com)
 * @brief Message Queue for the relay
 * @version 0.1
 * @date 2021-06-24
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <common/coap/coap.hpp>
#include <queue>

namespace condalf::service
{
    class MessageQueue
    {
        public:
        /**
         * @brief Describes all required data for constructing a PDU
         * 
         */
        struct Message
        {
            coap_pdu_type_t type;
            coap_pdu_code_t code;
            std::string uri;
            uint8_t* data;
            size_t data_size;
        };

        private:
            /**
             * @brief Mutex for the queue
             */
            std::mutex queue_mutex;

            /**
             * @brief All the messages
             */
            std::queue<Message*> messages;

        public:
            /**
             * @brief Default constructor
             */
            MessageQueue() = default;

            /**
             * @brief Deleted copy constructor
             */
            MessageQueue(const MessageQueue&) = delete;

            /**
             * @brief Destroy the Message Queue object
             */
            ~MessageQueue();

            /**
             * @brief Insert a Message into the queue.
             * 
             * @param msg The Message object
             */
            void Insert(Message* msg);

            /**
             * @brief Extract a Message from the queue.
             * 
             * @return Message* The Message object
             */
            Message* Extract();

            /**
             * @brief Checks if the queue is empty.
             * 
             * @return true The queue is empty.
             * @return false The queue has at least one element.
             */
            bool IsEmpty();

            /**
             * @brief Returns the size of the queue.
             * 
             * @return unsigned int Amount of messages in the queue.
             */
            unsigned int Size();
    };
}