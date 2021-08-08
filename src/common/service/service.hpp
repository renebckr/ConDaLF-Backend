/**
 * @file service.hpp
 * @author René Pascal Becker (OneDenper@gmail.com)
 * @brief Basic Service Class
 * @version 0.1
 * @date 2021-06-13
 * 
 * @copyright Copyright (c) 2021
 * 
 */

#pragma once

#include <vector>
#include <tuple>
#include <functional>
#include <atomic>
#include <thread>
#include <condition_variable>
#include <mutex>
#include <iostream>

namespace common
{
    class Service
    {
        using hook = std::function<bool()>;

        private:
            /**
             * @brief Hooks that should be executed when initializing/destroying the service.
             * It's a vector of tuples that contain two hooks. The first is the init-hook and
             * the second is the destroy-hook (i.e std::tuple<hook, hook>(start, stop)).
             */
            std::vector<std::tuple<hook, hook>> hooks;

            /**
             * @brief True if the service is still running and false if it stopped.
             */
            std::atomic_bool service_running {};

            /**
             * @brief Mutex object for thread safety and signaling.
             */
            std::mutex service_mutex;

            /**
             * @brief Conditional variable to make the Start method block until the service started.
             */
            std::condition_variable service_notifier;

            /**
             * @brief The thread object for the service
             */
            std::thread service_thread;

            /**
             * @brief Looping function that calls run() indefinetly until service is supposed to stop.
             */
            void service_loop();

        protected:
            /**
             * @brief The Service name. Set this to the name of your service in your class.
             */
            std::string service_name = "main";

            /**
             * @brief Adds a hook to the startup and stop process.
             * 
             * @param start The start-hook i.e. construct methods
             * @param stop Corresponding stop-hook i.e destroy methods
             */
            void add_hook(hook start, hook stop);

            /**
             * @brief The run method that should be implemented for every service
             */
            virtual void run() = 0;

        public:
            /**
             * @brief Construct a new Service object
             */
            Service();

            /**
             * @brief Deleted copy constructor
             */
            Service(const Service&) = delete;

            /**
             * @brief Destroy the Service object
             */
            ~Service();

            /**
             * @brief Deleted move operator
             * 
             * @return Service& 
             */
            Service& operator=(const Service&) = delete;

            /**
             * @brief Starts the service and runs the hooks
             * 
             * @return true On success
             * @return false On failure
             */
            bool Start();

            /**
             * @brief Gets the status of the service
             * 
             * @return true Running
             * @return false Stopped
             */
            bool IsActive();

            /**
             * @brief Stops the service and runs stop hooks
             */
            void Stop();

            /**
             * @brief Reloads the service (Stops first and then starts again)
             * 
             * @return true On success
             * @return false On failure
             */
            bool Reload();
    };
}