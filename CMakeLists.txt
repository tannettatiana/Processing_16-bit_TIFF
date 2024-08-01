set(CMAKE_AUTOUIC ON)
project(42)
cmake_minimum_required(VERSION 3.27)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -std=c++11")

find_package(Qt5Core REQUIRED)
find_package(Qt5Widgets REQUIRED)
find_package(Qt5Gui REQUIRED)

set(SOURCE_FILES main.cpp mainwindow.cpp mylabel.cpp geoimage.cpp rpc.cpp accuracytable.cpp plot.cpp histogram.cpp progress.cpp mypicker.cpp barchartplot.cpp transformimagesdialog.cpp)
set(HEADERS matrix.h rastermatrix.h rpc.h geoimage.h mylabel.h mainwindow.h accuracytable.h plot.h histogram.h progress.h mypicker.h barchartplot.h transformimagesdialog.h)
QT5_WRAP_CPP(HEADERS_MOC1 rastermatrix.h)
QT5_WRAP_CPP(HEADERS_MOC2 matrix.h)
QT5_WRAP_CPP(HEADERS_MOC3 mylabel.h)
QT5_WRAP_CPP(HEADERS_MOC4 mainwindow.h)
QT5_WRAP_CPP(HEADERS_MOC5 geoimage.h)
QT5_WRAP_CPP(HEADERS_MOC6 rpc.h)
QT5_WRAP_CPP(HEADERS_MOC7 accuracytable.h)
QT5_WRAP_CPP(HEADERS_MOC8 plot.h)
QT5_WRAP_CPP(HEADERS_MOC9 histogram.h)
QT5_WRAP_CPP(HEADERS_MOC10 progress.h)
QT5_WRAP_CPP(HEADERS_MOC11 mypicker.h)
QT5_WRAP_CPP(HEADERS_MOC12 barchartplot.h)
QT5_WRAP_CPP(HEADERS_MOC13 transformimagesdialog.h)

add_executable(${PROJECT_NAME} ${HEADERS} ${SOURCE_FILES}
${HEADERS_MOC1} ${HEADERS_MOC2} ${HEADERS_MOC3} ${HEADERS_MOC4} ${HEADERS_MOC5} ${HEADERS_MOC6}
${HEADERS_MOC7} ${HEADERS_MOC8} ${HEADERS_MOC9} ${HEADERS_MOC10} ${HEADERS_MOC11} ${HEADERS_MOC12} ${HEADERS_MOC13}
mainwindow.ui accuracytable.ui histogram.ui transformimagesdialog.ui)

add_subdirectory(tiff)
target_link_libraries(${PROJECT_NAME} tiff)

include_directories(Qwt-6.1.4/include/)
target_link_libraries(${PROJECT_NAME} F:/!work/42/42/Qwt-6.1.4/lib/libqwt.a)

target_link_libraries(${PROJECT_NAME} Qt5::Core)
target_link_libraries(${PROJECT_NAME} Qt5::Widgets)
target_link_libraries(${PROJECT_NAME} Qt5::Gui)
