cmake_minimum_required(VERSION 3.2 FATAL_ERROR)
project(eldstar_server)

set(CMAKE_CXX_STANDARD 17)

file(GLOB_RECURSE SOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/*.c
    ${CMAKE_SOURCE_DIR}/src/*.cpp
)

file(GLOB_RECURSE HEADER_FILES
    ${CMAKE_SOURCE_DIR}/src/*.h
    ${CMAKE_SOURCE_DIR}/src/*.hpp
)

file(GLOB_RECURSE RESOURCE_FILES
    ${CMAKE_SOURCE_DIR}/src/*.rc
)

include_directories(${CMAKE_SOURCE_DIR}/src/external)

execute_process(
    COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REV_COUNT
)

add_compile_definitions(GIT_REV_COUNT=${GIT_REV_COUNT})
message(STATUS "Version set to ${GIT_REV_COUNT}")

add_executable(${PROJECT_NAME} ${HEADER_FILES} ${SOURCE_FILES} ${RESOURCE_FILES})

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME} ${OPENGL_LIBRARIES})

IF(WIN32)
    target_link_libraries(${PROJECT_NAME} wsock32 ws2_32)
    find_package(glfw3 3.3 REQUIRED)  # For some reason GLFW will add a bad include flag if this is run, but not if you never find the package, yet still compiles.
ENDIF()

target_link_libraries(${PROJECT_NAME} glfw)

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${FREETYPE_LIBRARIES})

set(HARFBUZZ_INCLUDE_DIRS "" CACHE PATH "The /include/harfbuzz directory for harfbuzz.")
set(HARFBUZZ_LIBRARIES "" CACHE FILEPATH "The /lib/harfbuzz.(lib|a) file for harfbuzz.")

find_package(harfbuzz REQUIRED)
include_directories(${HARFBUZZ_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${HARFBUZZ_LIBRARIES})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME} ${PNG_LIBRARIES})

if(CMAKE_DL_LIBS)
    target_link_libraries(${PROJECT_NAME} ${CMAKE_DL_LIBS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries (${PROJECT_NAME} ${CMAKE_THREAD_LIBS_INIT})

set_property(TARGET ${PROJECT_NAME} PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}>")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME})

add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/framedump
    $<TARGET_FILE_DIR:${PROJECT_NAME}>/framedump
)
