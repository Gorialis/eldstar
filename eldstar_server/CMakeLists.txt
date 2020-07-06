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

include_directories(${CMAKE_SOURCE_DIR}/src/external)

execute_process(
    COMMAND git rev-list --count HEAD
    WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
    OUTPUT_VARIABLE GIT_REV_COUNT
)

add_compile_definitions(GIT_REV_COUNT=${GIT_REV_COUNT})
message(STATUS "Version set to ${GIT_REV_COUNT}")

add_executable(${PROJECT_NAME}_tassafe ${HEADER_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/resources_tassafe.rc)
add_executable(${PROJECT_NAME}_cheats ${HEADER_FILES} ${SOURCE_FILES} ${CMAKE_SOURCE_DIR}/src/resources_cheats.rc)

target_compile_definitions(${PROJECT_NAME}_tassafe PRIVATE -DELDSTAR_IS_TASSAFE)
target_compile_definitions(${PROJECT_NAME}_cheats PRIVATE -DELDSTAR_IS_CHEATS)

set(OpenGL_GL_PREFERENCE GLVND)

find_package(OpenGL REQUIRED)
include_directories(${OPENGL_INCLUDE_DIR})
target_link_libraries(${PROJECT_NAME}_tassafe ${OPENGL_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_cheats ${OPENGL_LIBRARIES})

IF(WIN32)
    target_link_libraries(${PROJECT_NAME}_tassafe wsock32 ws2_32)
    target_link_libraries(${PROJECT_NAME}_cheats wsock32 ws2_32)
    find_package(glfw3 3.3 REQUIRED)  # For some reason GLFW will add a bad include flag if this is run, but not if you never find the package, yet still compiles.
ENDIF()

target_link_libraries(${PROJECT_NAME}_tassafe glfw)
target_link_libraries(${PROJECT_NAME}_cheats glfw)

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

find_package(Freetype REQUIRED)
include_directories(${FREETYPE_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_tassafe ${FREETYPE_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_cheats ${FREETYPE_LIBRARIES})

set(HARFBUZZ_INCLUDE_DIRS "" CACHE PATH "The /include/harfbuzz directory for harfbuzz.")
set(HARFBUZZ_LIBRARIES "" CACHE FILEPATH "The /lib/harfbuzz.(lib|a) file for harfbuzz.")

find_package(harfbuzz REQUIRED)
include_directories(${HARFBUZZ_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_tassafe ${HARFBUZZ_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_cheats ${HARFBUZZ_LIBRARIES})

find_package(PNG REQUIRED)
include_directories(${PNG_INCLUDE_DIRS})
target_link_libraries(${PROJECT_NAME}_tassafe ${PNG_LIBRARIES})
target_link_libraries(${PROJECT_NAME}_cheats ${PNG_LIBRARIES})

if(CMAKE_DL_LIBS)
    target_link_libraries(${PROJECT_NAME}_tassafe ${CMAKE_DL_LIBS})
    target_link_libraries(${PROJECT_NAME}_cheats ${CMAKE_DL_LIBS})
endif()

set(THREADS_PREFER_PTHREAD_FLAG ON)
find_package(Threads REQUIRED)
target_link_libraries (${PROJECT_NAME}_tassafe ${CMAKE_THREAD_LIBS_INIT})
target_link_libraries (${PROJECT_NAME}_cheats ${CMAKE_THREAD_LIBS_INIT})

set_property(TARGET ${PROJECT_NAME}_tassafe PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}_tassafe>")
set_property(TARGET ${PROJECT_NAME}_cheats PROPERTY VS_DEBUGGER_WORKING_DIRECTORY "$<TARGET_FILE_DIR:${PROJECT_NAME}_cheats>")
set_property(DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR} PROPERTY VS_STARTUP_PROJECT ${PROJECT_NAME}_tassafe)

add_custom_command(TARGET ${PROJECT_NAME}_tassafe POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}_tassafe>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/framedump
    $<TARGET_FILE_DIR:${PROJECT_NAME}_tassafe>/framedump
)

add_custom_command(TARGET ${PROJECT_NAME}_cheats POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/resources
    $<TARGET_FILE_DIR:${PROJECT_NAME}_cheats>/resources
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/framedump
    $<TARGET_FILE_DIR:${PROJECT_NAME}_cheats>/framedump
)
