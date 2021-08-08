set(CONDALF_SERVICE_HEADERS relay.hpp message_queue.hpp session_manager.hpp session.hpp)
set(CONDALF_SERVICE_SOURCES relay.cpp message_queue.cpp session_manager.cpp session.cpp)

add_library(condalf_service_relay ${CONDALF_SERVICE_HEADERS} ${CONDALF_SERVICE_SOURCES})
target_link_libraries(condalf_service_relay common_service common_config common_coap logging)