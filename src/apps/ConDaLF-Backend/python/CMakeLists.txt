set(CONDALF_PYTHON_HEADERS python_integration.hpp)
set(CONDALF_PYTHON_SOURCES python_integration.cpp)

add_library(condalf_python ${CONDALF_PYTHON_HEADERS} ${CONDALF_PYTHON_SOURCES})
target_link_libraries(condalf_python Python3::Python logging)