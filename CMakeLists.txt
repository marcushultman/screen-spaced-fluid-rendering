cmake_minimum_required(VERSION 3.16)

project(screen-spaced-fluid-rendering)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_BUILD_TYPE release)
list(APPEND CXX_FLAGS "-stdlib=libc++")

find_package(OpenGL REQUIRED)
find_package(GLEW REQUIRED)
find_package(glfw3 REQUIRED)
find_package(DevIL REQUIRED)

set(FILES
  Box.cpp
  FirstPersonCamera.cpp
  FluidParticleSystem.cpp
  Model.cpp
  OrbitCamera.cpp
  Plane.cpp
  ShaderProgram.cpp
  SkyBox.cpp
  main.cpp
  textfile.cpp
)

foreach(f ${FILES})
  list(APPEND SOURCES "screen-spaced-fluid-rendering/src/${f}")
endforeach()

add_executable(ssfr ${SOURCES})

target_include_directories(
  ssfr
  PUBLIC
  ${OPENGL_INCLUDE_DIRS}
  ${GLEW_INCLUDE_DIRS}
  ${IL_INCLUDE_DIR}
)
target_link_libraries(
  ssfr
  ${OPENGL_LIBRARIES}
  /usr/local/lib/libGLEW.a
  glfw
  /usr/local/lib/libassimp.dylib
  ${IL_LIBRARIES}
)

target_include_directories(
  ssfr
  PUBLIC
  "${CMAKE_CURRENT_SOURCE_DIR}/screen-spaced-fluid-rendering/include"
)
