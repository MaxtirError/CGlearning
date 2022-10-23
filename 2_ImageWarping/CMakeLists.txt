cmake_minimum_required(VERSION 3.14 FATAL_ERROR)

project(ImageWarping VERSION 1.0)
message(STATUS "[Project] ${PROJECT_NAME}")

include(cmake/InitUCMake.cmake)
set(CMAKE_PREFIX_PATH "D:/app/Qt5/5.15.2/msvc2019_64")
Ubpa_InitUCMake()

Ubpa_InitProject()

Ubpa_QtInit(COMPONENTS Widgets)

set(USE_ANN FALSE CACHE BOOL "use ANN")
if(USE_ANN)
  Ubpa_AddDep(UANN 0.2.0)
endif()

Ubpa_AddSubDirsRec(src)
