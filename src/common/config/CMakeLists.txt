set(COMMON_CONFIG_HEADERS parser.h)
set(COMMON_CONFIG_SOURCES parser.cpp)

add_library(common_config ${COMMON_CONFIG_HEADERS} ${COMMON_CONFIG_SOURCES})
target_link_libraries(common_config logging)