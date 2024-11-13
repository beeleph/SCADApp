cmake_minimum_required(VERSION 3.15)

project(promTesto LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt5 REQUIRED COMPONENTS Core SerialBus)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core SerialBus)
#set(prometheus-cpp_DIR "C:/Users/isjir/Documents/GitHub/SCADApp/build")
#set(civetweb_DIR "C:/Users/isjir/Documents/GitHub/SCADApp/build")
#set(OpenSSL_DIR "C:/Users/isjir/Documents/GitHub/SCADApp/build")
#set(ZLIB_DIR "C:/Users/isjir/Documents/GitHub/SCADApp/build")
#set(CURL_DIR "C:/Users/isjir/Documents/GitHub/SCADApp/build")
find_package(prometheus-cpp CONFIG REQUIRED)

add_executable(promTesto
  psTest.cpp
)
target_link_libraries(promTesto Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(promTesto Qt${QT_VERSION_MAJOR}::SerialBus)
#target_link_libraries(promTesto prometheus-cpp::push)
#target_link_libraries(promTesto prometheus-cpp::core)
target_link_libraries(promTesto prometheus-cpp::pull)
