
#====== Global Configurations ==================#

cmake_minimum_required(VERSION 3.9)
set(CMAKE_CXX_STANDARD 17)

project(SampleQTWidgetsUI)

# OPTION(FORCE_CLANG OFF)

SET(FORCE_CLANG ON)

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

if(WIN32)
    set(CMAKE_PREFIX_PATH "C:/Qt/5.12.2/mingw73_64/lib/cmake")
endif()


# Find includes in corresponding build directories
set(CMAKE_INCLUDE_CURRENT_DIR ON)
# Instruct CMake to run moc automatically when needed
set(CMAKE_AUTOMOC ON)
# Create code from a list of Qt designer ui files
set(CMAKE_AUTOUIC ON)
# Enable Resource Compiler
set(CMAKE_AUTORCC ON)

# Find the QtWidgets library
find_package(Qt5Widgets CONFIG REQUIRED)
find_package(Qt5UiTools CONFIG REQUIRED)
find_package(Qt5Qml     CONFIG REQUIRED)
find_package(Qt5Script  CONFIG REQUIRED)
find_package(Qt5Network CONFIG REQUIRED)

#=================== TARGET CONFIGURATIONS =========================#

# Static Link MINGW Dependencies for avoiding missing DLLs errors
# Static link libgcc (MinGW C-Runtime Library) and LIBSTDC (MinGW C++ Runtime Library)
# Reference: https://stackoverflow.com/questions/19450722/include-mingw-libraries-in-compiled-file
IF(MINGW)
  set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++ -static")
ENDIF()

# Populate a CMake variable with the sources
set(helloworld_SRCS
    src/helloworld/mainwindow.ui
    src/helloworld/mainwindow.cpp
    src/helloworld/main.cpp
)

qt5_widgets_app(helloworld "${helloworld_SRCS}")

qt5_widgets_app(imageview1 "src/imageview1/imageview1.cpp")

# ----  EXPERIMENT: Using function  qt5_widgets_app ----------#

# SET(imageView2_src src/imageview2/imageview2.cpp src/imageview2/imageview2.qrc)
SET(imageView2_src src/imageview2/imageview2.cpp )
qt5_widgets_app(imageview2 "${imageView2_src}")


#-------------------------------------------------#
qt5_add_resources(forms1_resources src/form1/forms1.qrc)
message(" [DEBUG] forms1_resources = ${forms1_resources}")
qt5_widgets_app(forms1 "src/form1/forms1.cpp;${forms1_resources}"  "Qt5::UiTools")

#qt5_add_resources(forms1 forms1.qrc)

qt5_widgets_app(JSEngineTest "jsenginetest.cpp"  "Qt5::Qml")


qt5_widgets_app(sigslots "src/sigslot/sigslots.cpp")

qt5_widgets_app(FormBuilder "src/formbuilder/formbuilder.cpp")


qt5_widgets_app(qscript "qscript.cpp" "Qt5::Script")


qt5_widgets_app(network1 "network1.cpp" "Qt5::Network")


qt5_widgets_app(databinding "databinding.cpp")

if(false)
configure_file(
  "./form1.ui"
  "${CMAKE_CURRENT_BINARY_DIR}/" COPYONLY
)
endif()



#=========== Install Target ==================#

install(
    TARGETS      imageview2
    DESTINATION  ${CMAKE_CURRENT_LIST_DIR}/bin
    )


