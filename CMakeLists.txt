cmake_minimum_required(VERSION 3.15)
project(python-ofiq LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

find_package(Python3 REQUIRED COMPONENTS Interpreter Development.Module)

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c
            "import pybind11; print(pybind11.get_include())"
    OUTPUT_VARIABLE PYBIND11_INCLUDE_DIR
    OUTPUT_STRIP_TRAILING_WHITESPACE
    RESULT_VARIABLE PYBIND11_RESULT
)
if(NOT PYBIND11_RESULT EQUAL 0)
    message(FATAL_ERROR "Failed to find pybind11. Install it with: pip install pybind11")
endif()
message(STATUS "pybind11 include dir: ${PYBIND11_INCLUDE_DIR}")

execute_process(
    COMMAND "${Python3_EXECUTABLE}" -c
            "import tomllib; print(tomllib.load(open('pyproject.toml','rb'))['project']['version'])"
    WORKING_DIRECTORY "${CMAKE_SOURCE_DIR}"
    OUTPUT_VARIABLE OFIQPY_VERSION
    OUTPUT_STRIP_TRAILING_WHITESPACE
)
if(NOT OFIQPY_VERSION)
    set(OFIQPY_VERSION "0.0.0")
endif()
message(STATUS "python-ofiq version: ${OFIQPY_VERSION}")

if(DEFINED ENV{OFIQ_ROOT})
    set(OFIQ_ROOT "$ENV{OFIQ_ROOT}")
else()
    set(OFIQ_ROOT "/opt/OFIQ-Project/install_x86_64_linux/Release")
endif()

set(OFIQ_INCLUDE_DIR "${OFIQ_ROOT}/include")
set(OFIQ_LIB_DIR "${OFIQ_ROOT}/lib")

if(DEFINED ENV{OFIQ_SOURCE_DIR})
    set(OFIQ_SOURCE_INCLUDE_DIR "$ENV{OFIQ_SOURCE_DIR}/OFIQlib/include")
else()
    set(OFIQ_SOURCE_INCLUDE_DIR "/opt/OFIQ-Project/OFIQlib/include")
endif()

message(STATUS "OFIQ_ROOT: ${OFIQ_ROOT}")
message(STATUS "OFIQ_INCLUDE_DIR: ${OFIQ_INCLUDE_DIR}")
message(STATUS "OFIQ_LIB_DIR: ${OFIQ_LIB_DIR}")

add_library(_core MODULE csrc/ofiq_pybind.cpp)

target_include_directories(_core PRIVATE
    ${PYBIND11_INCLUDE_DIR}
    ${OFIQ_INCLUDE_DIR}
    ${OFIQ_SOURCE_INCLUDE_DIR}
)

target_link_directories(_core PRIVATE ${OFIQ_LIB_DIR})

target_compile_definitions(_core PRIVATE OFIQPY_VERSION="${OFIQPY_VERSION}")

target_link_libraries(_core PRIVATE Python3::Module ofiq_lib)

set_target_properties(_core PROPERTIES
    PREFIX ""
    BUILD_WITH_INSTALL_RPATH TRUE
    INSTALL_RPATH "${OFIQ_LIB_DIR}"
    INSTALL_RPATH_USE_LINK_PATH TRUE
)

install(TARGETS _core DESTINATION ofiq)
