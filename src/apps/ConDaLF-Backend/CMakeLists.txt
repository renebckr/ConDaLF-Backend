add_subdirectory(service)
add_subdirectory(python)

#set(CONDALF_BACKEND_HEADERS )
#set(CONDALF_BACKEND_SOURCES  main.cpp)

#add_library(condalf_backend_lib ${CONDALF_BACKEND_HEADERS} ${CONDALF_BACKEND_SOURCES})
#target_link_libraries(condalf_backend_lib common_config coap-3 common_coap common_config common_service Python3::Python)

add_executable(condalf_backend main.cpp)
target_link_libraries(condalf_backend condalf_service_server condalf_service_relay logging)

install(TARGETS condalf_backend RUNTIME)
#install(TARGETS condalf_backend_lib LIBRARY)