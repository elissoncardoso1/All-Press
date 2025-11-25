cmake_minimum_required(VERSION 3.20)
project(AllPressCore 
    VERSION 1.1.0
    DESCRIPTION "High-Performance Print Management System"
    LANGUAGES CXX C)

# C++ Standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Build type
if(NOT CMAKE_BUILD_TYPE)
    set(CMAKE_BUILD_TYPE Release)
endif()

# Compiler flags
set(CMAKE_CXX_FLAGS_DEBUG "-g -O0 -DDEBUG -Wall -Wextra")
set(CMAKE_CXX_FLAGS_RELEASE "-O3 -DNDEBUG -march=native")

# Conan dependencies
find_package(Boost REQUIRED)
find_package(OpenSSL REQUIRED)
find_package(nlohmann_json REQUIRED)
find_package(spdlog REQUIRED)
find_package(Crow REQUIRED)
find_package(Threads REQUIRED)

# CUPS (macOS/Linux)
if(APPLE)
    # On macOS, CUPS is usually installed via Homebrew or system
    find_library(CUPS_LIBRARY cups PATHS /opt/homebrew/lib /usr/local/lib /usr/lib)
    find_path(CUPS_INCLUDE_DIR cups/cups.h PATHS /opt/homebrew/include /usr/local/include /usr/include)
    
    if(CUPS_LIBRARY AND CUPS_INCLUDE_DIR)
        set(CUPS_FOUND TRUE)
        message(STATUS "CUPS found: ${CUPS_LIBRARY}")
    else()
        message(WARNING "CUPS not found - printing functionality will be limited")
    endif()
elseif(UNIX)
    find_package(PkgConfig)
    if(PkgConfig_FOUND)
        pkg_check_modules(CUPS cups)
        if(CUPS_FOUND)
            message(STATUS "CUPS found: ${CUPS_VERSION}")
        else()
            message(WARNING "CUPS not found - printing functionality will be limited")
        endif()
    endif()
endif()

# SQLite3
find_library(SQLITE3_LIBRARY sqlite3 REQUIRED)
find_path(SQLITE3_INCLUDE_DIR sqlite3.h REQUIRED)

# Include directories
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/include)
include_directories(${CMAKE_CURRENT_SOURCE_DIR}/external)
include_directories(${SQLITE3_INCLUDE_DIR})

if(CUPS_FOUND)
    if(APPLE)
        include_directories(${CUPS_INCLUDE_DIR})
    else()
        include_directories(${CUPS_INCLUDE_DIRS})
    endif()
    add_definitions(-DHAVE_CUPS)
endif()

# 🆕 Protocol Library
set(PROTOCOL_SOURCES
    src/protocols/hpgl_generator.cpp
    src/protocols/postscript_generator.cpp
    src/protocols/compatibility_matrix.cpp
    src/protocols/protocol_factory.cpp
)

# Create protocol library
add_library(all_press_protocols ${PROTOCOL_SOURCES})
target_include_directories(all_press_protocols PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/include)
target_link_libraries(all_press_protocols PRIVATE spdlog::spdlog)

# Source files
set(CORE_SOURCES
    src/core/printer_manager.cpp
    src/core/printer_manager_plotter.cpp
    src/core/job_queue.cpp
    src/core/job_queue_plotter.cpp
    src/core/color_manager.cpp
    
    src/network/cups_client.cpp
    src/network/ipp_client.cpp
    src/network/network_scanner.cpp
    
    src/conversion/file_processor.cpp
    src/conversion/pdf_processor.cpp
    src/conversion/image_processor.cpp
    
    src/api/http_server.cpp
    src/api/websocket_server.cpp
    src/api/endpoints.cpp
    src/api/rest_server.cpp
    src/api/plotter_endpoints.cpp
    
    src/database/sqlite_manager.cpp
    src/database/models.cpp
    
    src/utils/logger.cpp
    src/utils/config.cpp
    src/utils/file_utils.cpp
)

# Main executable
add_executable(all_press_server src/main.cpp ${CORE_SOURCES})

# Linking
target_link_libraries(all_press_server
    all_press_protocols
    Boost::boost
    OpenSSL::SSL
    OpenSSL::Crypto
    nlohmann_json::nlohmann_json
    spdlog::spdlog
    Crow::Crow
    ${SQLITE3_LIBRARY}
    Threads::Threads
)

# Platform-specific linking
if(CUPS_FOUND)
    if(APPLE)
        target_link_libraries(all_press_server ${CUPS_LIBRARY})
    else()
        target_link_libraries(all_press_server ${CUPS_LIBRARIES})
    endif()
endif()

if(WIN32)
    target_link_libraries(all_press_server winspool)
endif()

# Install
install(TARGETS all_press_server DESTINATION bin)
install(DIRECTORY include/ DESTINATION include)
install(DIRECTORY config/ DESTINATION /etc/all_press)

# Tests (opcional - descomente para habilitar)
# enable_testing()
# add_subdirectory(tests)
