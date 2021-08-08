set(COMMON_SERVICE_HEADERS service.hpp)
set(COMMON_SERVICE_SOURCES service.cpp)

add_library(common_service ${COMMON_SERVICE_HEADERS} ${COMMON_SERVICE_SOURCES})
target_link_libraries(common_service logging pthread)