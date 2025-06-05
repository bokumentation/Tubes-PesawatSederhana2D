cmake_minimum_required(VERSION 3.11.0)
project(pesawat2d VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17) 
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF)

set(SOURCE_FILES
  src/main.cpp
  src/window.cpp
  src/player_data.cpp
)

set(RAYLIB_DIR ${CMAKE_CURRENT_SOURCE_DIR}/lib/raylib)
include_directories(${RAYLIB_DIR}/include)

add_executable(${PROJECT_NAME} ${SOURCE_FILES})
target_link_libraries(${PROJECT_NAME}
  ${RAYLIB_DIR}/lib/libraylib.a
  -lopengl32
  -lgdi32
  -lwinmm
  -lws2_32
  -lcomdlg32 
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Assets are copied to the build directory
add_custom_command(TARGET ${PROJECT_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
            ${CMAKE_CURRENT_SOURCE_DIR}/assets
            $<TARGET_FILE_DIR:${PROJECT_NAME}>/assets
    COMMENT "Copying assets to build directory"
)
