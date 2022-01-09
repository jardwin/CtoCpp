cmake_minimum_required (VERSION 3.0)

project ("ProjetEpitaSDL")

set (CMAKE_CXX_STANDARD 17)

IF(WIN32)
  message(STATUS "Building for windows")

  set(SDL2_INCLUDE_DIRS "SDL2/include")
  set(SDL2IMAGE_INCLUDE_DIRS "SDL2_image/include")

  # Support both 32 and 64 bit builds
  if (${CMAKE_SIZEOF_VOID_P} MATCHES 8)
    message(STATUS "Building for 64bit")
    set(SDL2_LINK_DIRS "SDL2/lib/x64")
    set(SDL2IMAGE_LINK_DIRS "SDL2_image/lib/x64")
  else ()
    message(STATUS "Building for 32bit")
    set(SDL2_LINK_DIRS "SDL2/lib/x86")
    set(SDL2IMAGE_LINK_DIRS "SDL2_image/lib/x86")
  endif ()

  include_directories(${SDL2_INCLUDE_DIRS})
  include_directories(${SDL2IMAGE_INCLUDE_DIRS})
  link_directories(${SDL2_LINK_DIRS}, ${SDL2IMAGE_LINK_DIRS})

  add_executable(ProjetEpitaSDL Project_SDL_Part1_base/main.cpp Project_SDL_Part1_base/Project_SDL1.cpp)
  target_link_libraries(ProjetEpitaSDL PUBLIC SDL2 SDL2main SDL2_image)
ELSE()
  message(STATUS "Building for Linux or Mac")

  find_package(SDL2 REQUIRED)
  include_directories(${SDL2_INCLUDE_DIRS})

  set(prefix "/usr")
  set(exec_prefix "${prefix}")
  set(libdir "${prefix}/lib/x86_64-linux-gnu")
  set(SDL2_IMAGE_PREFIX "/usr")
  set(SDL2_IMAGE_EXEC_PREFIX "/usr")
  set(SDL2_IMAGE_INCLUDE_DIRS "${prefix}/include/SDL2_image")
  set(SDL2_IMAGE_LIBRARIES "-lSDL2_image")
  link_directories("${prefix}/lib/x86_64-linux-gnu")
  add_executable(ProjetEpitaSDL Project_SDL_Part1_base/main.cpp Project_SDL_Part1_base/Project_SDL1.cpp)
  target_link_libraries(ProjetEpitaSDL ${SDL2_LIBRARIES} ${SDL2_IMAGE_LIBRARIES})
ENDIF()