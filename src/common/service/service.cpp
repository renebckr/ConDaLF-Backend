#include "service.hpp"

#include <common/logging/logging.h>

using namespace common;

void Service::service_loop()
{
    // Service is now running
    service_running.store(true);

    // Notify to announce start of thread
    {
        std::unique_lock<std::mutex> lock(service_mutex);
        lock.unlock();
        service_notifier.notify_one();
    }

    // Run while we are active
    while (service_running.load())
        run();
}

void Service::add_hook(hook start, hook stop)
{
    hooks.push_back(std::tuple<hook, hook>(start, stop));
}

Service::Service()
{
    service_running.store(false);
}

Service::~Service()
{
    Stop();
    hooks.clear();
}

bool Service::Start()
{
    if (service_running.load())
    {
        common::logging::log_information(std::cout, LINE_INFORMATION, std::string("Service \"") + service_name + "\" already running.");
        return false;
    }
    
    common::logging::log_information(std::cout, LINE_INFORMATION, std::string("Starting service \"") + service_name + "\"");
    // Run start hooks
    unsigned int i = 0;
    bool success = true;
    for (; i < hooks.size(); i++)
    {
        if (!std::get<0>(hooks[i])())
        {
            success = false;
            break;
        }
    }

    // Run stop hooks on failure
    if (!success)
    {
        common::logging::log_error(std::cout, LINE_INFORMATION, std::string("A hook failed. Aborting start of service \"") + service_name + "\"");
        for (unsigned int j = i; j > 0; j--)
            std::get<1>(hooks[j - 1])();
        return false;
    }

    // Start thread
    service_thread = std::thread(&Service::service_loop, this);

    // Wait for the Service to respond
    {
        std::unique_lock<std::mutex> lock(service_mutex);
        service_notifier.wait(lock);
    }
    return true;
}

void Service::Stop()
{
    // Check if running
    if (!service_running.load())
        return;
    
    common::logging::log_information(std::cout, LINE_INFORMATION, std::string("Stopping service \"") + service_name + "\"");
    // Stop thread
    service_running.store(false);
    service_thread.join();

    // Run Stop hooks
    for (auto it = hooks.rbegin(); it != hooks.rend(); it++)
        std::get<1>(*it)();
}

bool Service::Reload()
{
    common::logging::log_information(std::cout, LINE_INFORMATION, std::string("Reloading service \"") + service_name + "\"");
    Stop();
    return Start();
}

bool Service::IsActive()
{
    return service_running.load();
}