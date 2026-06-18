cmake_minimum_required(VERSION 3.15)

# Define project name and supported languages
project(DTLN_AEC C CXX)

# Set C++ standard (TFLite usually requires C++14 or C++17 depending on version)
set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_POSITION_INDEPENDENT_CODE ON)

# ==============================================================================
# 1. Architecture Detection (for output directory naming)
# ==============================================================================
if(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm64|aarch64|ARM64|AARCH64)$")
    set(ARCH_NAME "aarch64")
elseif(CMAKE_SYSTEM_PROCESSOR MATCHES "^(arm|ARM|armv7l|armv7a)$")
    set(ARCH_NAME "arm")
elseif(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(ARCH_NAME "x64")
else()
    set(ARCH_NAME "x86")
endif()

# Set default installation prefix to: <build_dir>/install/DTLN_AEC_<architecture>
if(CMAKE_INSTALL_PREFIX_INITIALIZED_TO_DEFAULT)
    set(CMAKE_INSTALL_PREFIX "${CMAKE_BINARY_DIR}/../${PROJECT_NAME}_${ARCH_NAME}" 
        CACHE PATH "Default install path" FORCE)
endif()

# ==============================================================================
# 2. Source and Header Files
# ==============================================================================
set(SOURCES
    kissfft/kfc.c
    kissfft/kiss_fft.c
    kissfft/kiss_fftnd.c
    kissfft/kiss_fftndr.c
    kissfft/kiss_fftr.c
    DTLN_AEC.cpp
)

set(HEADERS
    DTLN_AEC.h
    kissfft/kfc.h
    kissfft/kissfft.hh
    kissfft/kissfft_i32.hh
    kissfft/kiss_fft.h
    kissfft/kiss_fftnd.h
    kissfft/kiss_fftndr.h
    kissfft/kiss_fftr.h
    kissfft/kiss_fft_log.h
    kissfft/_kiss_fft_guts.h
)

# ==============================================================================
# 3. TensorFlow Lite Dependency Management (FetchContent)
# ==============================================================================
# Option to toggle between prebuilt binaries (Windows) and source build (Cross-compile)
option(BUILD_TFLITE_FROM_SOURCE "Build TensorFlow Lite from source" ON)

if(BUILD_TFLITE_FROM_SOURCE)
    message(STATUS "Downloading and building TensorFlow Lite from source...")
    include(FetchContent)
    set(FETCHCONTENT_QUIET FALSE)

    # [ACTION REQUIRED]: Change this tag to match your previous TFLite version!
    set(TFLITE_VERSION "v2.16.1" CACHE STRING "TensorFlow Lite version tag")
    
    FetchContent_Declare(
        tensorflow
        GIT_REPOSITORY https://gitee.com/mirrors/tensorflow.git
        GIT_TAG        ${TFLITE_VERSION}
        GIT_SHALLOW    TRUE
        GIT_PROGRESS   TRUE
    )
    
    FetchContent_GetProperties(tensorflow)
    if(NOT tensorflow_POPULATED)
        FetchContent_Populate(tensorflow)
        # We only add the TFLite C API subdirectory to save build time
        add_subdirectory(${tensorflow_SOURCE_DIR}/tensorflow/lite/c ${tensorflow_BINARY_DIR}/tensorflow/lite/c EXCLUDE_FROM_ALL)
    endif()
    
    # The target name from TFLite CMake is usually `tensorflow-lite-c`
    set(TFLITE_LIB_TARGET tensorflowlite_c)
    set(TFLITE_INCLUDE_DIR ${tensorflow_SOURCE_DIR})
else()
    message(STATUS "Using prebuilt TensorFlow Lite libraries...")
    set(TFLITE_LIB_TARGET tensorflowlite_c)
    set(TFLITE_INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/TensorFlowLite)
    link_directories(${CMAKE_SOURCE_DIR}/TensorFlowLite/lib/${ARCH_NAME}) # link_directories必须要放在依赖此目录中库的可执行文件的add_library或add_executable之前
    include_directories(${CMAKE_SOURCE_DIR}/TensorFlowLite)
endif()

# ==============================================================================
# 4. Target Definition
# ==============================================================================
# SHARED means we are generating a dynamic library (.dll / .so)
add_library(${PROJECT_NAME} SHARED ${SOURCES})

# Add postfix 'd' for Debug mode
set_target_properties(${PROJECT_NAME} PROPERTIES DEBUG_POSTFIX "d")

# Macro definitions
target_compile_definitions(${PROJECT_NAME} PRIVATE DTLNAEC_EXPORTS)
if(WIN32)
    target_compile_definitions(${PROJECT_NAME} PRIVATE _WINDOWS _USRDLL)
endif()

# Include directories
target_include_directories(${PROJECT_NAME} PRIVATE
    ${TFLITE_INCLUDE_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}/kissfft
    ${CMAKE_CURRENT_SOURCE_DIR}/models
)

# Link directories and libraries

if(WIN32)
    target_link_directories(${PROJECT_NAME} PRIVATE
        "$<$<CONFIG:Debug>:${CMAKE_CURRENT_SOURCE_DIR}/TensorFlowLite/lib/Debug>"
        "$<$<CONFIG:Release>:${CMAKE_CURRENT_SOURCE_DIR}/TensorFlowLite/lib/Release>"
    )
endif()
target_link_libraries(${PROJECT_NAME} PRIVATE ${TFLITE_LIB_TARGET})


# ==============================================================================
# 5. Installation Rules
# ==============================================================================
# Install the library target (dll/so/lib)
install(TARGETS ${PROJECT_NAME}
    RUNTIME DESTINATION bin        # .exe, .dll
    LIBRARY DESTINATION lib        # .so
    ARCHIVE DESTINATION lib        # .a, .lib
)

# Install headers maintaining directory structure
install(FILES DTLN_AEC.h DESTINATION include)

# Install tflite lib
if(BUILD_TFLITE_FROM_SOURCE)
    # Use $<TARGET_FILE> to auto catch dynamic lib file by TFLite ，and give the exec permission (PROGRAMS)
    install(PROGRAMS $<TARGET_FILE:${TFLITE_LIB_TARGET}> DESTINATION lib)

    # If on windows, copy the lib file
    if(WIN32)
        install(FILES $<TARGET_LINKER_FILE:${TFLITE_LIB_TARGET}> DESTINATION lib OPTIONAL)
    endif()
else()
    # If use the pre compile
    if(WIN32)
        install(PROGRAMS "${CMAKE_CURRENT_SOURCE_DIR}/TensorFlowLite/lib/$<CONFIG>/tensorflowlite_c.dll"
                DESTINATION bin OPTIONAL)
        install(FILES "${CMAKE_CURRENT_SOURCE_DIR}/TensorFlowLite/lib/$<CONFIG>/tensorflowlite_c.lib"
                DESTINATION lib OPTIONAL)
    else()
        install(PROGRAMS "${CMAKE_SOURCE_DIR}/TensorFlowLite/lib/${ARCH_NAME}/libtensorflowlite_c.so"
                DESTINATION lib OPTIONAL)
    endif()
endif()
