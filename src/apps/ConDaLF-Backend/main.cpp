#include <cstring>
#include <cstdlib>
#include <cstdio>
#include <iostream>
#include <sys/types.h>
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <sstream>
#include <common/logging/logging.h>

#include <service/server/server.hpp>
#include <service/relay/relay.hpp>

bool python_enabled = false;
bool python_initialized = false;

condalf::service::Server* coap_server = nullptr;
condalf::service::MessageQueue* msg_queue = nullptr;
condalf::service::Relay* relay = nullptr;

void cleanup()
{
    std::cout << "Closing ConDaLF..." << std::endl;

    // Stop CoAP Server Service
    if (coap_server != nullptr)
        coap_server->Stop();
    
    // Stop Relay Service
    if (relay != nullptr)
    {
        relay->Stop();
        delete relay;
    }

    // Remove Message Queue
    if (msg_queue)
        delete msg_queue;
}

void close_handler(sig_t s)
{
    cleanup();
    exit(1);
}

void argument_usage()
{
    std::cout << "###########################################" << std::endl;
    std::cout << "#      C O N D A L F - B A C K E N D      #" << std::endl;
    std::cout << "#                U S A G E                #" << std::endl;
    std::cout << "###########################################" << std::endl;
    std::cout << "#                                         #" << std::endl;
    std::cout << "#   'h': Host                             #" << std::endl;
    std::cout << "#        Address to listen to.            #" << std::endl;
    std::cout << "#            -h 0.0.0.0                   #" << std::endl;
    std::cout << "#                                         #" << std::endl;
    std::cout << "#   'p': Port                             #" << std::endl;
    std::cout << "#        Port that the server should      #" << std::endl;
    std::cout << "#        run on.                          #" << std::endl;
    std::cout << "#            -p 5683                      #" << std::endl;
    std::cout << "#                                         #" << std::endl;
    std::cout << "#   'r': Relay                            #" << std::endl;
    std::cout << "#        When specified the Server will   #" << std::endl;
    std::cout << "#        relay requests. You can pass     #" << std::endl;
    std::cout << "#        a custom config here.            #" << std::endl;
    std::cout << "#            -r config_file               #" << std::endl;
    std::cout << "#                                         #" << std::endl;
    std::cout << "#   's': Python                           #" << std::endl;
    std::cout << "#        Set the python module to         #" << std::endl;
    std::cout << "#        execute in the python folder.    #" << std::endl;
    std::cout << "#        Will run process_data            #" << std::endl;
    std::cout << "#            -s user_script               #" << std::endl;
    std::cout << "#                                         #" << std::endl;
    std::cout << "###########################################" << std::endl;
}

int main(int argc, char* argv[])
{
    signal(SIGINT, (sighandler_t)close_handler);
    setvbuf(stdout, NULL, _IOLBF, 0);
    setvbuf(stderr, NULL, _IOLBF, 0);

    std::string host = "0.0.0.0";
    std::string port = "5683";
    bool relay_enabled = false;
    bool python_enabled = false;
    std::string relay_config = "";
    std::string python_script = "";

    // Check all arguments
    // condalf_backend [-h Host] [-p Port] [-r Relay config] [-s Python module]
    int opt = 0;
    while ((opt = getopt(argc, argv, "h:p:r:s:")) != -1)
    {
        switch (opt)
        {
            case 'h': // Host Option
                host = std::string(optarg);
                break;
            case 'p': // Port Option
                port = std::string(optarg);
                break;
            case 'r': // Relay Option
                relay_enabled = true;
                relay_config = std::string(optarg);
                break;
            case 's': // Script Option
                python_enabled = true;
                python_script = std::string(optarg);
                break;
            default: // Invalid argument
                argument_usage();
                return EXIT_FAILURE;
        }
    }

    // Print options
    std::stringstream options;
    options << "Host: " << host << std::endl
            << "Port: " << port << std::endl << std::endl;
    if (relay_enabled)
    {
        options << "Relay is enabled." << std::endl
                << "Relay Configuration file: " << relay_config << std::endl << std::endl;
    }
    if (python_enabled)
    {
        options << "Python script is enabled." << std::endl
                << "Python script module: " << python_script << std::endl;
    }

    common::logging::log_information(std::cout, "Startup options:\n", options.str());
    options.clear();

    // Start Relay
    if (relay_enabled)
    {
        msg_queue = new condalf::service::MessageQueue();
        relay = new condalf::service::Relay(msg_queue);
        if (!relay->Start(relay_config))
        {
            common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not start Relay Service.");
            delete relay;
            delete msg_queue;
            return EXIT_FAILURE;
        }
    }

    // Start Server
    coap_server = &condalf::service::Server::getInstance();
    if (!coap_server->Start(host, port, msg_queue, python_enabled, python_script))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not start CoAP Server Service.");
        return EXIT_FAILURE;
    }

    while (true)
    {
        // Do this with unix ui perhaps?
        std::string line;
        std::getline(std::cin, line);

        if (line.compare("status") == 0)
        {
            common::logging::log_information(std::cout,
                                             LINE_INFORMATION,
                                             std::string("The Server Service is ") 
                                            + (coap_server->IsActive() ? "" : "not ") 
                                            + "running");
        }
        else if (line.compare("start") == 0)
        {
            if (relay != nullptr)
                if (!relay->Start(relay_config))
                    common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not start Relay Service.");

            if (!coap_server->Start(host, port, msg_queue, python_enabled, python_script))
                common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not start CoAP Server Service.");
        }
        else if (line.compare("stop") == 0)
        {
            coap_server->Stop();
            relay->Stop();
        }
        else if (line.compare("reload") == 0)
        {
            coap_server->Reload();
            relay->Reload(relay_config);
        }
        else if (line.compare("quit") == 0 || line.compare("q") == 0)
        {
            break;
        }
        else
        {
            // TODO: Make this pretty
            std::cout << "Unknown command \"" << line << "\" try status, start, stop or reload." << std::endl;
        }
    }

    cleanup();
    return EXIT_SUCCESS;
}