cmake_minimum_required(VERSION 3.11.0)
project(pesawat2d VERSION 0.1.0 LANGUAGES C CXX)

# Set the C++ standard (important for modern C++ features)
set(CMAKE_CXX_STANDARD 17) # You can use 11, 14, 20 based on your preference
set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
set(CMAKE_CXX_EXTENSIONS OFF) # Optional: ensures standard compliance

set(SOURCE_FILES
  src/main.cpp
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
  -lcomdlg32 # Often needed for file dialogs etc.
)

set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
