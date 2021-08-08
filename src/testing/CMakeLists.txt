set(TESTING_HEADERS base.h)
set(TESTING_SOURCES testing.cpp)

add_library(testing ${TESTING_HEADERS} ${TESTING_SOURCES})
target_link_libraries(testing PUBLIC logging)