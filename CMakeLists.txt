cmake_minimum_required(VERSION 3.8)
project(jcu-transport CXX C)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

set(INC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu/transport/error.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu/transport/transport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu/transport/ssl_engine.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu/transport/tcp_transport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu/transport/tls_transport.h
        ${CMAKE_CURRENT_SOURCE_DIR}/include/jcu/transport/openssl_ssl_engine.h
)

set(SRC_FILES
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tcp_transport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/tls_transport.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/src/openssl_ssl_engine.cpp
)

find_package(OpenSSL REQUIRED)
if(OpenSSL_FOUND)
    set(JCU_TRANSPORT_HAS_OPENSSL ON)
endif()

configure_file(${CMAKE_CURRENT_SOURCE_DIR}/src/jcu_transport_config.h.in ${CMAKE_CURRENT_BINARY_DIR}/build/jcu/transport/config.h @ONLY)

if(MSVC)
    add_definitions(-D_WINSOCKAPI_)
endif()

add_library(${PROJECT_NAME} ${SRC_FILES} ${INC_FILES} ${CMAKE_CURRENT_BINARY_DIR}/build/jcu/transport/config.h)
target_include_directories(${PROJECT_NAME}
        PUBLIC "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include;${CMAKE_CURRENT_BINARY_DIR}/build;${UVW_INCLUDE_DIR}>"
        $<INSTALL_INTERFACE:include>
        )

if(OpenSSL_FOUND)
    target_link_libraries(${PROJECT_NAME} OpenSSL::Crypto)
    target_link_libraries(${PROJECT_NAME} OpenSSL::SSL)
endif()

find_package(uv REQUIRED)
target_link_libraries(${PROJECT_NAME} uv)

# find_package(uvw REQUIRED)
target_include_directories(${PROJECT_NAME} PRIVATE ${UVW_INCLUDE_DIR})

