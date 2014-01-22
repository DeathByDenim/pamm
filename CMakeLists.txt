project(pamm)
cmake_minimum_required(VERSION 2.6)
find_package(Qt4 COMPONENTS QtCore QtGui QtNetwork REQUIRED)
find_package(QJSON REQUIRED)
include_directories(${QT_INCLUDES} ${QJSON_INCLUDE_DIR} ${CMAKE_CURRENT_BINARY_DIR})

set(PAMM_SRCS installedmod.cpp pamm.cpp main.cpp modmanager.cpp mod.cpp availablemod.cpp)
qt4_automoc(${PAMM_SRCS})
add_executable(pamm ${PAMM_SRCS})
target_link_libraries(pamm ${QT_QTCORE_LIBRARY} ${QT_QTGUI_LIBRARY} ${QT_QTNETWORK_LIBRARY} ${QJSON_LIBRARIES} -lzip)

add_custom_command(OUTPUT locale/nl.qm COMMAND ${QT_LRELEASE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/locale/nl.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/locale/nl.qm DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/locale/nl.qm)
add_custom_command(OUTPUT locale/de.qm COMMAND ${QT_LRELEASE_EXECUTABLE} ${CMAKE_CURRENT_SOURCE_DIR}/locale/de.ts -qm ${CMAKE_CURRENT_BINARY_DIR}/locale/de.qm DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/locale/de.qm)
