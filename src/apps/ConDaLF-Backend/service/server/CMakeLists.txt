set(CONDALF_SERVICE_HEADERS server.hpp)
set(CONDALF_SERVICE_SOURCES server.cpp)

add_library(condalf_service_server ${CONDALF_SERVICE_HEADERS} ${CONDALF_SERVICE_SOURCES})
target_link_libraries(condalf_service_server condalf_service_relay condalf_python common_service common_coap logging)