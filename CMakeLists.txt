cmake_minimum_required(VERSION 3.15)
project(Interconnect-MP)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(Threads REQUIRED)

# Incluir el directorio de headers
include_directories(${CMAKE_SOURCE_DIR}/include)

# Buscar automáticamente todos los .cpp y .h
file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
        ${CMAKE_SOURCE_DIR}/src/*.cpp
        ${CMAKE_SOURCE_DIR}/src/*.mif
        ${CMAKE_SOURCE_DIR}/include/*.h
)

# Copy memory.mif to build directory
configure_file(${CMAKE_SOURCE_DIR}/src/RAM/memory.mif
        ${CMAKE_BINARY_DIR}/RAM/memory.mif
        COPYONLY)


message(STATUS "Archivos fuente encontrados:")
foreach(file ${SOURCES})
    message(STATUS "${file}")
endforeach()


# Crear el ejecutable con todos los archivos encontrados
add_executable(Interconnect-MP ${SOURCES}
        #src/RAM/FileMemory.cpp
        #include/RAM/memory.h
        include/RAM/FileMemory.h
        src/RAM/FileMemory.cpp)


target_link_libraries(Interconnect-MP PRIVATE Threads::Threads)



if(BUILD_TESTING)
    enable_testing()
    add_subdirectory(tests)
endif()
