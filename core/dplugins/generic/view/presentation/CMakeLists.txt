#
# Copyright (c) 2010-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.

include(MacroDPlugins)

if(HAVE_MEDIAPLAYER)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/audio)
endif()

if(HAVE_OPENGL)
    include_directories(${CMAKE_CURRENT_SOURCE_DIR}/opengl)
    include_directories($<TARGET_PROPERTY:Qt5::OpenGL,INTERFACE_INCLUDE_DIRECTORIES>)
endif()

include_directories(${CMAKE_CURRENT_SOURCE_DIR}/dialogs
                    ${CMAKE_CURRENT_SOURCE_DIR}/widgets
                    ${CMAKE_CURRENT_SOURCE_DIR}/common
)

include_directories($<TARGET_PROPERTY:Qt5::Widgets,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:Qt5::Core,INTERFACE_INCLUDE_DIRECTORIES>

                    $<TARGET_PROPERTY:KF5::ConfigCore,INTERFACE_INCLUDE_DIRECTORIES>
                    $<TARGET_PROPERTY:KF5::I18n,INTERFACE_INCLUDE_DIRECTORIES>
)

set(presentationplugin_SRCS
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_captionpage.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_advpage.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/presentationctrlwidget.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/widgets/presentationwidget.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/common/presentationcontainer.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/common/presentationloader.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentationdlg.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_mainpage.cpp

    ${CMAKE_CURRENT_SOURCE_DIR}/presentationmngr.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/presentationplugin.cpp
)

if(HAVE_OPENGL)
    set(presentationplugin_SRCS
        ${presentationplugin_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/presentationgl.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/presentationkb.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/kbeffect.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/opengl/kbimageloader.cpp
       )
endif()

if(HAVE_MEDIAPLAYER)
    set(presentationplugin_SRCS
        ${presentationplugin_SRCS}
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentationaudiolist.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentation_audiopage.cpp
        ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentationaudiowidget.cpp
    )

    ki18n_wrap_ui(presentationplugin_SRCS
                  ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentation_audiopage.ui
                  ${CMAKE_CURRENT_SOURCE_DIR}/audio/presentationaudiowidget.ui
    )
endif()

ki18n_wrap_ui(presentationplugin_SRCS
              ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_mainpage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_captionpage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/dialogs/presentation_advpage.ui
              ${CMAKE_CURRENT_SOURCE_DIR}/widgets/presentationctrlwidget.ui
)


if(HAVE_OPENGL)
    DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Presentation
                                        SOURCES ${presentationplugin_SRCS}
                                        DEPENDS Qt5::OpenGL
                                                ${OPENGL_LIBRARIES}
    )
else()
    DIGIKAM_ADD_GENERIC_PLUGIN(NAME    Presentation
                                        SOURCES ${presentationplugin_SRCS}
    )
endif()
