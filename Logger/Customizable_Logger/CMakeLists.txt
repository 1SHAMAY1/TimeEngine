cmake_minimum_required(VERSION 3.10)
project(Customizable_Logger)

# Use C++17
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Force dynamic runtime (/MD, /MDd)
foreach(flag_var 
    CMAKE_C_FLAGS_DEBUG CMAKE_C_FLAGS_RELEASE 
    CMAKE_C_FLAGS_RELWITHDEBINFO CMAKE_C_FLAGS_MINSIZEREL
    CMAKE_CXX_FLAGS_DEBUG CMAKE_CXX_FLAGS_RELEASE 
    CMAKE_CXX_FLAGS_RELWITHDEBINFO CMAKE_CXX_FLAGS_MINSIZEREL)
    string(REPLACE "/MT" "/MD" ${flag_var} "${${flag_var}}")
endforeach()

# Include headers
include_directories(${PROJECT_SOURCE_DIR}/Include)

# Source and header files
file(GLOB_RECURSE LOGGER_SRC "${PROJECT_SOURCE_DIR}/Source/*.cpp")
file(GLOB_RECURSE LOGGER_HEADERS "${PROJECT_SOURCE_DIR}/Include/*.hpp")

# Create static library
add_library(Customizable_Logger STATIC
    ${LOGGER_SRC}
    ${LOGGER_HEADERS}
)

# Name the output correctly to match 'Customizable_Logger.lib'
set_target_properties(Customizable_Logger PROPERTIES
    OUTPUT_NAME "Customizable_Logger"
    ARCHIVE_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    LIBRARY_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/lib"
    RUNTIME_OUTPUT_DIRECTORY "${PROJECT_BINARY_DIR}/bin"
)

# Optional IDE visual structure
source_group(TREE "${PROJECT_SOURCE_DIR}/Include" PREFIX "Header Files" FILES ${LOGGER_HEADERS})
source_group(TREE "${PROJECT_SOURCE_DIR}/Source"  PREFIX "Source Files" FILES ${LOGGER_SRC})
