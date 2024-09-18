cmake_minimum_required(VERSION 3.10)
project(raven C)

# Set compiler flags
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -Wall -Wextra -Wpedantic")

# Find required packages
find_package(PkgConfig REQUIRED)
pkg_check_modules(RAYLIB REQUIRED raylib)
pkg_check_modules(GTK REQUIRED gtk+-3.0)
pkg_check_modules(LIBAVFORMAT REQUIRED libavformat)
pkg_check_modules(LIBAVUTIL REQUIRED libavutil)

# Include directories
include_directories(${RAYLIB_INCLUDE_DIRS} ${GTK_INCLUDE_DIRS} ${LIBAVFORMAT_INCLUDE_DIRS} ${LIBAVUTIL_INCLUDE_DIRS})

# Set source files
set(SRC_FILES main.c)

# Link libraries
link_libraries(${RAYLIB_LIBRARIES} ${GTK_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVUTIL_LIBRARIES} -lglfw -lm -ldl -lpthread)

# Add executable
add_executable(${PROJECT_NAME} ${SRC_FILES})

# Link to libraries
target_link_libraries(${PROJECT_NAME} ${RAYLIB_LIBRARIES} ${GTK_LIBRARIES} ${LIBAVFORMAT_LIBRARIES} ${LIBAVUTIL_LIBRARIES})
