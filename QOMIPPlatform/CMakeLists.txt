cmake_minimum_required(VERSION 3.19)
project(QOMIPPlatform LANGUAGES CXX)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets QuickWidgets Multimedia)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(Qt6 REQUIRED COMPONENTS Core)
find_package(OpenCV REQUIRED)
qt_standard_project_setup()



qt6_add_resources(RESOURCE_FILES Qmlres.qrc)


qt_add_executable(QOMIPPlatform
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    CommonUtils.h
    worker.h worker.cpp
    basicviewwidget.h basicviewwidget.cpp basicviewwidget.ui
    Reader.h
    Reader.cpp
    frameprocessor.h frameprocessor.cpp
    algorithmlistmodel.h algorithmlistmodel.cpp
    algorithmmanagerdialog.h algorithmmanagerdialog.cpp algorithmmanagerdialog.ui
    algorithmitemdelegate.h algorithmitemdelegate.cpp
    algorithmdialog.h algorithmdialog.cpp
    algorithmselectiondialog.h algorithmselectiondialog.cpp
    detachedwindow.h detachedwindow.cpp
    resourceextractor.h resourceextractor.cpp
    Qmlres.qrc
    resources.qrc
    cameramanager.cpp cameramanager.h
)


# 添加这段配置
qt_add_resources(QOMIPPlatform "Qmlres"
    PREFIX "/"
    FILES
        Qml/AlgorithmModelListView.qml
        Qml/ListViewDelegation.qml
        Qml/ParameterEditDialog.qml
        Qml/NumberInputComponent.qml
        Qml/TextInputComponent.qml
        Qml/BooleanInputComponent.qml
        Qml/DescriptionComponent.qml
        Qml/NoParamsComponent.qml
)

add_subdirectory(Algorithms)
add_subdirectory(CustomerAlg)

target_link_libraries(QOMIPPlatform
    PRIVATE
        Qt::Core
        Qt::Widgets
        Qt6::QuickWidgets
        ${OpenCV_LIBS}
        Qt6::Multimedia
)
target_link_libraries(QOMIPPlatform PRIVATE Qt6::Core)
target_link_libraries(QOMIPPlatform PRIVATE Qt6::Core)
target_link_libraries(QOMIPPlatform PRIVATE Qt6::Core)


include(GNUInstallDirs)

install(TARGETS QOMIPPlatform
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET QOMIPPlatform
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
install(SCRIPT ${deploy_script})
