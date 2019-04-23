
#====== Global Configurations ==================#

cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)

project(SampleQTWidgetsUI)

set(FORCE_CLANG ON)
# Force compiler to Clang++
if(FORCE_CLANG)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
endif()

#========== Targets Configurations ============#


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)

# Populate a CMake variable with the sources
set(helloworld_SRCS
    mainwindow.ui
    mainwindow.cpp
    main.cpp
)
# Tell CMake to create the helloworld executable
add_executable(helloworld ${helloworld_SRCS})
# Use the Widgets module from Qt 5
target_link_libraries(helloworld Qt5::Widgets)

add_executable(imageview1 imageview1.cpp)
target_link_libraries(imageview1 Qt5::Widgets)


add_executable(imageview2 imageview2.cpp)
target_link_libraries(imageview2 Qt5::Widgets)


if(false)
 add_executable(addressgui addressgui.cpp)
 target_link_libraries(addressgui Qt5::Widgets)
endif()

#=========== Install Target ==================#

install(
    TARGETS      imageview2
    DESTINATION  ${CMAKE_CURRENT_LIST_DIR}/bin
    )


