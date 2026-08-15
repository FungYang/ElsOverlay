cmake_minimum_required(VERSION 3.19)
project(ElsOverlay LANGUAGES CXX)

find_package(Qt6 6.5 REQUIRED COMPONENTS Core Widgets)

qt_standard_project_setup()
qt_add_executable(ElsOverlay
    WIN32 MACOSX_BUNDLE
    main.cpp
    mainwindow.cpp
    mainwindow.h
    mainwindow.ui
    overlay.h overlay.cpp
    globalkeyboard.h globalkeyboard.cpp
    skillbox.h skillbox.cpp
    skilloverlay.h skilloverlay.cpp
    app.rc
    buffbox.h buffbox.cpp
    buffoverlay.h buffoverlay.cpp
    classselector.h classselector.cpp

    buffvisioncore.h buffvisioncore.cpp
    buffvisionoverlay.h buffvisionoverlay.cpp

    buffvisioncapturesetup.h buffvisioncapturesetup.cpp
    buffvisionmanager.h buffvisionmanager.cpp
    buffvisioncapture.h buffvisioncapture.cpp
    buffvisiondetector.h buffvisiondetector.cpp
    buffvisiondebug.h buffvisiondebug.cpp
    buffvisionconfig.h
    overlayroot.h overlayroot.cpp
    classconfigurationmanager.h classconfigurationmanager.cpp
    classbuffconfigwindow.h classbuffconfigwindow.cpp
    classbuffeditorwindow.h classbuffeditorwindow.cpp
    newbuffdialog.h newbuffdialog.cpp
    distanceguideconfiguration.h distanceguideconfiguration.cpp
    distanceguidemanager.h distanceguidemanager.cpp
    distanceguideconfigwindow.h distanceguideconfigwindow.cpp
    distanceguidegraphic.h distanceguidegraphic.cpp
    distanceguideline.h distanceguideline.cpp
    distanceguideoverlay.h distanceguideoverlay.cpp
    distanceguiderectangle.h distanceguiderectangle.cpp
    distanceguidecircle.h distanceguidecircle.cpp



)
target_link_libraries(ElsOverlay
    PRIVATE
        Qt::Core
        Qt::Widgets
)

install(TARGETS ElsOverlay
    BUNDLE  DESTINATION .
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
)

qt_generate_deploy_app_script(
    TARGET ElsOverlay
    OUTPUT_SCRIPT deploy_script
    NO_UNSUPPORTED_PLATFORM_ERROR
)
add_custom_command(TARGET ElsOverlay POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
    ${CMAKE_SOURCE_DIR}/images
    $<TARGET_FILE_DIR:ElsOverlay>/images
)
install(SCRIPT ${deploy_script})
