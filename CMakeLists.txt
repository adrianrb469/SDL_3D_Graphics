cmake_minimum_required(VERSION 3.21)

project(spaceship VERSION 1.0)

# Enable C++20 features
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# find and include SDL2, SDL_image, and GLM
find_package(SDL2 REQUIRED)
include_directories(${SDL2_INCLUDE_DIRS})

find_package(glm REQUIRED)
include_directories(${GLM_INCLUDE_DIRS})

file(GLOB_RECURSE SOURCES CONFIGURE_DEPENDS
    "${PROJECT_SOURCE_DIR}/src/*.cpp"
)

# Add the source files to the project
set(SOURCES
    src/main.cpp
    src/Camera.cpp
    src/Renderer.cpp
    src/ObjLoader.cpp
)

add_executable(${PROJECT_NAME} ${SOURCES})

target_include_directories(${PROJECT_NAME}
    PRIVATE ${PROJECT_SOURCE_DIR}/include
    PUBLIC ${PROJECT_SOURCE_DIR}/src
)

# link libraries
target_link_libraries(${PROJECT_NAME} ${SDL2_LIBRARIES} ${GLM_LIBRARIES})
