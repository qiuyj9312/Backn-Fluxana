cmake_minimum_required(VERSION 3.10)
project(FlattenEvent)

# Find ROOT package
find_package(ROOT REQUIRED)
include_directories(
    ${ROOT_INCLUDE_DIRS}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/src
)
link_directories(${ROOT_LIBRARY_DIR})

# Set C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Find nlohmann/json library for rdataframe_analysis
find_path(NLOHMANN_JSON_INCLUDE_DIR
    NAMES nlohmann/json.hpp
    PATHS
        /usr/include
        /usr/local/include
        ${CMAKE_INSTALL_PREFIX}/include
        $ENV{HOME}/.local/include
    DOC "nlohmann/json include directory"
)

if(NOT NLOHMANN_JSON_INCLUDE_DIR)
    message(WARNING
        "nlohmann/json library not found. rdataframe_analysis will not be built.\n"
        "Install it with: sudo apt-get install nlohmann-json3-dev (Ubuntu/Debian)\n"
        "               or: sudo yum install json-devel (CentOS/RHEL)"
    )
endif()

# Generate ROOT dictionary
ROOT_GENERATE_DICTIONARY(
    WNSEventDict
    WNSEvent.h
    LINKDEF include/WNSEventLinkDef.h
)

# Add executable
add_executable(flatten_event
    flatten_event.cpp
    src/WNSEvent.cxx
    ${CMAKE_CURRENT_BINARY_DIR}/WNSEventDict.cxx
)

# Link ROOT libraries
target_link_libraries(flatten_event ${ROOT_LIBRARIES})

# Set output directory
set_target_properties(flatten_event PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
)

# Copy PCM file to bin directory for runtime
add_custom_command(TARGET flatten_event POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_CURRENT_BINARY_DIR}/libWNSEventDict_rdict.pcm
        ${CMAKE_BINARY_DIR}/bin/libWNSEventDict_rdict.pcm
    COMMENT "Copying PCM file to bin directory"
)

# Add rdataframe_analysis executable if nlohmann/json is available
if(NLOHMANN_JSON_INCLUDE_DIR)
    include_directories(${NLOHMANN_JSON_INCLUDE_DIR})
    
    add_executable(rdataframe_analysis
        rdataframe_analysis.cpp
        src/RDataFrameAnalysis.cxx
        src/ConfigReader.cxx
    )
    
    # Link ROOT libraries
    target_link_libraries(rdataframe_analysis ${ROOT_LIBRARIES})
    
    # Set output directory
    set_target_properties(rdataframe_analysis PROPERTIES
        RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin
    )
    
    message(STATUS "Building rdataframe_analysis: YES")
    message(STATUS "nlohmann/json include: ${NLOHMANN_JSON_INCLUDE_DIR}")
else()
    message(STATUS "Building rdataframe_analysis: NO (nlohmann/json not found)")
endif()

# Print configuration
message(STATUS "========================================")
message(STATUS "Configuration Summary")
message(STATUS "========================================")
message(STATUS "ROOT include dirs: ${ROOT_INCLUDE_DIRS}")
message(STATUS "ROOT libraries: ${ROOT_LIBRARIES}")
message(STATUS "C++ standard: ${CMAKE_CXX_STANDARD}")
message(STATUS "========================================")
