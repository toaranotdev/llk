cmake_minimum_required(VERSION 3.9)

project(llk VERSION 0.1 LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_BUILD_TYPE RELEASE)

# find qt
find_package(Qt5 REQUIRED COMPONENTS Widgets Multimedia) 

set(PROJECT_SOURCES
        ../src/main.cpp
	../src/game.cpp
	../src/game.h
	../src/matrix.cpp
	../src/matrix.h
)

# resources files, i.e musics, textures, sound effects
qt5_add_big_resources(PROJECT_SOURCES resources/audio.qrc)
qt5_add_big_resources(PROJECT_SOURCES resources/images.qrc)

# Define target properties for Android with Qt 6 as:
#   	 set_property(TARGET ${PROJECT_NAME} APPEND PROPERTY QT_ANDROID_PACKAGE_SOURCE_DIR
#                 	${CMAKE_CURRENT_SOURCE_DIR}/android)
# For more information, see https://doc.qt.io/qt-6/qt-add-executable.html#target-creation
if(ANDROID)
        add_library(${PROJECT_NAME} SHARED
        ${PROJECT_SOURCES}
        )
# Define properties for Android with Qt 5 after find_package() calls as:
#    	set(ANDROID_PACKAGE_SOURCE_DIR "${CMAKE_CURRENT_SOURCE_DIR}/android")
else()
        add_executable(${PROJECT_NAME}
        ${PROJECT_SOURCES}
        )
endif()

target_link_libraries(${PROJECT_NAME} PRIVATE Qt${QT_VERSION_MAJOR}::Widgets Qt${QT_VERSION_MAJOR}::Multimedia)

set_target_properties(${PROJECT_NAME} PROPERTIES
    	MACOSX_BUNDLE_GUI_IDENTIFIER my.${PROJECT_NAME}.com
    	MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    	MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    	MACOSX_BUNDLE TRUE
    	WIN32_EXECUTABLE TRUE
)

install(TARGETS ${PROJECT_NAME}
    	BUNDLE DESTINATION .
    	LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR})

if(QT_VERSION_MAJOR EQUAL 6)
    	qt_finalize_executable(${PROJECT_NAME})
endif()
