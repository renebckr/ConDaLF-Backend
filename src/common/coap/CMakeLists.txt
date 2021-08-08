set(COMMON_COAP_HEADERS coap.hpp)
set(COMMON_COAP_SOURCES coap.cpp)

add_library(common_coap ${COMMON_COAP_HEADERS} ${COMMON_COAP_SOURCES})
target_link_libraries(common_coap coap-3 logging)