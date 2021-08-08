#include <cstring>
#include <functional>
#include <python/python_integration.hpp>
#include <common/logging/logging.h>
#include <apps/ConDaLF-Backend/service/relay/relay.hpp>
#include <sys/types.h>

#include "server.hpp"


using namespace condalf::service;

bool g_python_enabled = false;          // Python Processing enabled?
MessageQueue* g_msg_queue = nullptr;       // The Relay service itself

COAP_RESOURCE_HANDLER(handle_condalf_test_get)
{
    common::logging::log_information(std::cout, LINE_INFORMATION, "Received GET on /condalf/test");
    coap_pdu_set_code(response, COAP_RESPONSE_CODE_CONTENT);
    coap_add_data(response, 5, (const uint8_t *)"valid");
}

COAP_RESOURCE_HANDLER(handle_condalf_data_put)
{
    std::vector<uint8_t> data = common::CoAP::getInstance().ResourceBlockHandler(resource, session, request, response);
    std::string str(data.begin(), data.end());
    
    // We have a complete message
    if (data.size() != 0)
    {
        common::logging::log_information(std::cout, LINE_INFORMATION, std::string("Received PUT on /condalf/data with size ") + std::to_string(data.size()));
        // Relay if enabled
        if (g_msg_queue != nullptr)
        {
            // Copy data
            uint8_t* data_copy = new u_int8_t[data.size()];
            std::memcpy(data_copy, &data[0], data.size());

            g_msg_queue->Insert(new MessageQueue::Message {
                .type = COAP_MESSAGE_CON,
                .code = COAP_REQUEST_CODE_PUT,
                .uri = "condalf/data",
                .data = data_copy,
                .data_size = data.size()
            });
        }

        // Python Processing if available
        if (g_python_enabled)
            condalf::python_process_data(data);
    }
}

bool Server::enable_coap()
{
    // Get CoAP Instance
    common::CoAP *coap = &common::CoAP::getInstance();

    // Create a context
    coap_context = coap->CreateContext();
    if (coap->context_descriptor_invalid(coap_context))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not create context. Exiting.");
        return false;
    }

    // Create Endpoint and if failed -> return
    if (!coap->CreateEndpoint(coap_context, host, port))
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not create endpoint. Exiting.");
        return false;
    }

    // Create resource /condalf/data - failure -> return
    coap_condalf_data_res = coap->CreateResource("condalf/data");
    if (coap_condalf_data_res == COAP_INVALID_RVALUE)
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not create resource. Exiting.");
        return false;
    }

    // Create resource /condalf/data - failure -> return
    coap_condalf_test_res = coap->CreateResource("condalf/test");
    if (coap_condalf_test_res == COAP_INVALID_RVALUE)
    {
        common::logging::log_error(std::cerr, LINE_INFORMATION, "Could not create resource. Exiting.");
        return false;
    }

    // Register and assign resource to context
    coap->RegisterResourceHandler(coap_condalf_test_res, 
                                  COAP_REQUEST_GET, 
                                  handle_condalf_test_get);

    coap->RegisterResourceHandler(coap_condalf_data_res, 
                                  COAP_REQUEST_PUT, 
                                  handle_condalf_data_put);

    coap->AddResource(coap_context, coap_condalf_data_res);
    coap->AddResource(coap_context, coap_condalf_test_res);
    return true;
}

bool Server::disable_coap()
{
    // Get CoAP Instance
    common::CoAP *coap = &common::CoAP::getInstance();

    // Release our context -> will free everything associated with it
    coap->ReleaseContext(coap_context);
    return true;
}

bool Server::enable_python()
{
    g_python_enabled = false;
    // Initialize Python if it is enabled
    if (python_enabled)
    {
        if (condalf::initialize_python(python_script))
        {
            g_python_enabled = true;
            return true;
        }
        return false;
    }
    return true;
}

bool Server::disable_python()
{
    g_python_enabled = false;
    // Uninit Python when required
    if (python_enabled)
        condalf::uninitialize_python();
    return true;
}

void Server::run()
{
    // Get CoAP Instance
    common::CoAP *coap = &common::CoAP::getInstance();

    // Run IO with timeout of 1 second
    coap->IO(coap_context, 1000);
}

Server::Server() : Service()
{
    // Initialize all values
    this->service_name = "ConDaLF-Backend-Server";

    // Bind hooks for the service
    add_hook(
        std::bind(&Server::enable_coap, this),
        std::bind(&Server::disable_coap, this)
    );

    add_hook(
        std::bind(&Server::enable_python, this),
        std::bind(&Server::disable_python, this)
    );
}

bool Server::Start(const std::string& _host,
                   const std::string& _port,
                   MessageQueue* _msg_queue,
                   bool enable_python_script, 
                   const std::string& _script_file)
{
    this->host = _host;
    this->port = _port;
    this->python_enabled = enable_python_script;
    this->python_script = _script_file;
    g_msg_queue = _msg_queue;
    return common::Service::Start();
}


bool Server::Reload(const std::string& _host,
                    const std::string& _port,
                    MessageQueue* _msg_queue,
                    bool enable_python_script, 
                    const std::string& _script_file)
{
    this->host = _host;
    this->port = _port;
    this->python_enabled = enable_python_script;
    this->python_script = _script_file;
    g_msg_queue = _msg_queue;
    return common::Service::Reload();
}