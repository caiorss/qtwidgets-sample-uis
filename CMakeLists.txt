
#====== Global Configurations ==================#

cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)

project(SampleQTWidgetsUI)

set(FORCE_CLANG OFF)
# Force compiler to Clang++
if(FORCE_CLANG)
    set(CMAKE_C_COMPILER clang)
    set(CMAKE_CXX_COMPILER clang++)
endif()


# set(CMAKE_C_COMPILER   gcc)
# set(CMAKE_CXX_COMPILER g++)


# https://gist.github.com/Rod-Persky/e6b93e9ee31f9516261b
macro(qt5_copy_dll APP DLL)
    # find the release *.dll file
    get_target_property(Qt5_${DLL}Location Qt5::${DLL} LOCATION)
    # find the debug *d.dll file
    get_target_property(Qt5_${DLL}LocationDebug Qt5::${DLL} IMPORTED_LOCATION_DEBUG)

    add_custom_command(TARGET ${APP} POST_BUILD
       COMMAND ${CMAKE_COMMAND} -E copy_if_different $<$<CONFIG:Debug>:${Qt5_${DLL}LocationDebug}> $<$<NOT:$<CONFIG:Debug>>:${Qt5_${DLL}Location}> $<TARGET_FILE_DIR:${APP}>)
endmacro()

function(qt5_widgets_app APP SOURCES)
    message(" [TRACE] APP = ${APP} ")
    message(" [TRACE] SOURCES = ${SOURCES}")
    IF(WIN32)
        add_executable(${APP} WIN32 "${SOURCES}")
    ELSE()
        add_executable(${APP} "${SOURCES}")
    ENDIF()

    SET(LIBS Qt5::Core Qt5::Gui Qt5::Widgets "${ARGV2}")
    message(" [TRACE] LIBS = ${LIBS}")

    #LIST(APPEND ${ARGV1} Qt5::Widgets)
    target_link_libraries(${APP} "${LIBS}")


    IF(WIN32)
       FOREACH(LIB  ${LIBS})
          STRING(REGEX REPLACE "^Qt5::" "" LIB_WITHOUT_PREFIX ${LIB})
          message(" [TRACE] LIB = ${LIB} LIB_P = ${LIB_WITHOUT_PREFIX}")
          qt5_copy_dll(${APP} ${LIB_WITHOUT_PREFIX})
       ENDFOREACH()
    ENDIF()

endfunction()

#=================================================================#

set(CMAKE_PREFIX_PATH "C:/Qt/5.12.2/mingw73_64/lib/cmake")


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
# find_package(Qt5Declarative CONFIG REQUIRED)
find_package(Qt5UiTools CONFIG REQUIRED)
# find_package(Qt5 COMPONENTS Core Gui)

#=================== TARGET CONFIGURATIONS =========================#

# Populate a CMake variable with the sources
set(helloworld_SRCS
    mainwindow.ui
    mainwindow.cpp
    main.cpp
)
qt5_widgets_app(helloworld "${helloworld_SRCS}")

qt5_widgets_app(imageview1 "imageview1.cpp")

# ----  EXPERIMENT: Using function  qt5_widgets_app ----------#

SET(imageView2_src imageview2.cpp imageview2.qrc)
qt5_widgets_app(imageview2 "${imageView2_src}")


#-------------------------------------------------#

qt5_widgets_app(forms1 "forms1.cpp"  "Qt5::UiTools")

configure_file(
  "./form1.ui"
  "${CMAKE_CURRENT_BINARY_DIR}/" COPYONLY
)

#=========== Install Target ==================#

install(
    TARGETS      imageview2
    DESTINATION  ${CMAKE_CURRENT_LIST_DIR}/bin
    )


