cmake_minimum_required(VERSION 3.12 FATAL_ERROR)

project(prometheusTest)

set(QT_DIR "/Users/beeleph/Qt/5.15.2/clang_64/lib/cmake/Qt5")
set(Qt5_DIR "/Users/beeleph/Qt/5.15.2/clang_64/lib/cmake/Qt5")
set(QT_QMAKE_EXECUTABLE "/Users/beeleph/Qt/5.15.2/clang_64/bin/qmake")

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(QT NAMES Qt5 COMPONENTS Core Widgets SerialBus REQUIRED)
find_package(Qt${QT_VERSION_MAJOR} COMPONENTS Core Widgets SerialBus REQUIRED)


find_package(prometheus-cpp CONFIG REQUIRED)

#if(NOT DEFINED prometheus-cpp_VERSION)
#  message(FATAL_ERROR "prometheus-cpp_VERSION is not defined")
#endif()

message(STATUS "are we?")
add_executable(psTest psTest.cpp)
target_link_libraries(psTest PRIVATE prometheus-cpp::push)
target_link_libraries(psTest PRIVATE prometheus-cpp::pull)
target_link_libraries(psTest PRIVATE Qt${QT_VERSION_MAJOR}::Core)
target_link_libraries(psTest PRIVATE Qt${QT_VERSION_MAJOR}::Widgets)
target_link_libraries(psTest PRIVATE Qt${QT_VERSION_MAJOR}::SerialBus)


#if(PROMETHEUS_CPP_ENABLE_PUSH)
#  message(STATUS "prom cpp enable push")
#  add_executable(psTest psTest.cpp)
#  target_link_libraries(psTest  PRIVATE prometheus-cpp::push) #$<$<BOOL:${WIN32}>:Ws2_32>)
#endif()

#if(PROMETHEUS_CPP_ENABLE_PULL)
#  add_executable(sample-server sample_server.cc)
#  target_link_libraries(sample-server PRIVATE prometheus-cpp::pull)
#endif()
