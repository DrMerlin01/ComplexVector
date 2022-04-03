cmake_minimum_required(VERSION 3.10)

project(ComplexVector)

set(CMAKE_CXX_STANDARD 17)

if(CMAKE_SYSTEM_NAME MATCHES "^MINGW")
    set(SYSTEM_LIBS -lstdc++)
else()
    set(SYSTEM_LIBS)
endif()

if(CMAKE_CXX_COMPILER_ID MATCHES "MSVC")
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG}/JMC")
else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wall -Wextra -pedantic -Werror -Wno-unused-parameter -Wno-implicit-fallthrough")
endif()

set(INCLUDE_DIR inc)
set(SOURCE_DIR src)

set(FILES_VECTOR "${INCLUDE_DIR}/vector.h")
set(FILES_TESTS "${INCLUDE_DIR}/tests.h" "${SOURCE_DIR}/tests.cpp")
set(FILES_MAIN "${SOURCE_DIR}/main.cpp")

source_group("Vector" FILES ${FILES_VECTOR})
source_group("Tests" FILES ${FILES_TESTS})
source_group("Source" FILES ${FILES_MAIN})

add_executable("complex_vector" ${FILES_MAIN} ${FILES_TESTS} ${FILES_VECTOR})
target_link_libraries("complex_vector" ${SYSTEM_LIBS})
