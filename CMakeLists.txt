project(pamm)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
find_package(QJSON REQUIRED)
include_directories(${QT_INCLUDES} ${QJSON_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(PAMM_SRCS installedmod.cpp pamm.cpp main.cpp modmanager.cpp mod.cpp availablemod.cpp)

qt4_automoc(${PAMM_SRCS})

add_subdirectory("i18n")

add_executable(pamm ${PAMM_SRCS} ${QM})
target_link_libraries(pamm ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QJSON_LIBRARIES} -lzip)

