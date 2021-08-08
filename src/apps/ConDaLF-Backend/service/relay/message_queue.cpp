#include "message_queue.hpp"
#include <mutex>

using namespace condalf::service;

MessageQueue::~MessageQueue()
{
    while (!IsEmpty())
    {
        auto msg = Extract();
        delete[] msg->data;
        delete msg;
    }
}

void MessageQueue::Insert(MessageQueue::Message* msg)
{
    // Lock and insert
    std::lock_guard guard(queue_mutex);
    messages.push(msg);
}

MessageQueue::Message* MessageQueue::Extract()
{
    std::lock_guard guard(queue_mutex);

    // When empty we do not have a message to return
    if (messages.empty())
        return nullptr;
    
    MessageQueue::Message* msg = messages.front();
    messages.pop();
    return msg;
}

bool MessageQueue::IsEmpty()
{
    // Lock and return empty check
    std::lock_guard guard(queue_mutex);
    return messages.empty();
}

unsigned int MessageQueue::Size()
{
    // Lock and return empty check
    std::lock_guard guard(queue_mutex);
    return messages.size();
}
