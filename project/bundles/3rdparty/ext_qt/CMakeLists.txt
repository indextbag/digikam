# Script to build Qt for digiKam bundle.
#
# Copyright (c) 2015-2019, Gilles Caulier, <caulier dot gilles at gmail dot com>
#
# Redistribution and use is allowed according to the terms of the BSD license.
# For details see the accompanying COPYING-CMAKE-SCRIPTS file.
#

SET(EXTPREFIX_qt "${EXTPREFIX}")

IF(NOT ENABLE_QTWEBENGINE)
    SET(DROP_QTWEBENGINE_DEPS
        -skip qtwebengine                 # No need Chromium browser support (QtWebkit instead)
        -skip qtwebchannel                # QtWebChannel support ==> QWebEngine dependency
        -skip qtquickcontrols             # QtQuick support      ==> QWebEngine dependency
        )
ENDIF()

ExternalProject_Add(ext_qt
    DOWNLOAD_DIR ${EXTERNALS_DOWNLOAD_DIR}
    URL https://download.qt.io/official_releases/qt/5.11/5.11.3/single/qt-everywhere-src-5.11.3.tar.xz
    URL_MD5 02b353bfe7a40a8dc4274e1d17226d2b

    PATCH_COMMAND ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-appimage-support.patch &&
                  ${PATCH_COMMAND} -p1 -i ${CMAKE_CURRENT_SOURCE_DIR}/qt-appimage-glibc.patch

    CMAKE_ARGS -DOPENSSL_LIBS='-L${EXTPREFIX_qt}/lib -lssl -lcrypto'

    CONFIGURE_COMMAND <SOURCE_DIR>/configure
                  -prefix ${EXTPREFIX_qt}
                  -verbose
                  -release
                  -opensource
                  -confirm-license
                  -sql-sqlite                       # Compile Sqlite SQL plugin
                  -sql-mysql                        # Compile Mysql SQL plugin
                  -fontconfig
                  -iconv                            # iconv is used instead icu
                  -system-freetype                  # Use system font rendering lib https://doc.qt.io/qt-5/qtgui-attribution-freetype.html
                  -openssl-linked                   # hard link ssl libraries
                  -nomake tests                     # Do not build test codes
                  -nomake examples                  # Do not build basis example codes
                  -no-compile-examples              # Do not build extra example codes
                  -no-qml-debug
                  -no-mtdev
                  -no-journald
                  -no-syslog
                  -no-tslib
                  -no-directfb
                  -no-linuxfb
                  -no-libproxy
                  -no-pch
                  -no-icu                           # iconv is used instead icu
                  -qt-zlib
                  -qt-pcre
                  -qt-harfbuzz
                  -qt-xcb
                  -qt-xkbcommon-x11
                  -skip qt3d                        # 3D core
                  -skip qtactiveqt                  # No need ActiveX support
                  -skip qtandroidextras             # For embeded devices only
                  -skip qtwinextras                 # For Windows devices only
                  -skip qtmacextras                 # For MacOS devices only
                  -skip qtcanvas3d                  # 3D extensions
                  -skip qtcharts                    # No need data models charts support
                  -skip qtconnectivity              # For embeded devices only
                  -skip qtscript                    # No need scripting (deprecated)
                  -skip qtdatavis3d                 # no need 3D data visualizations support
                  -skip qtdoc                       # No need documentation
                  -skip qtenginio                   # No need backend-as-service support
                  -skip qtgamepad                   # No need gamepad hardware support.
                  -skip qtgraphicaleffects          # Advanced graphical effects in GUI
                  -skip qtlocation                  # No need geolocation
                  -skip qtquickcontrols2            # QtQuick support for QML
                  -skip qtmultimedia                # No need multimedia support (replaced by QtAV+ffmpeg)
                  -skip qtnetworkauth               # No need network authentification support.
                  -skip qtpurchasing                # No need in-app purchase of products support
                  -skip qtremoteobjects             # No need sharing QObject properties between processes support
                  -skip qtserialport                # No need serial port support
                  -skip qtscxml                     # No need SCXML state machines support
                  -skip qtsensors                   # For embeded devices only
                  -skip qtspeech                    # No need speech synthesis support
                  -skip qttranslations              # No need translation tools.
                  -skip qtvirtualkeyboard           # No need virtual keyboard support
                  -skip qtwayland                   # Specific to Linux
                  -skip qtwebsockets                # No need websocket support
                  -skip qtwebview                   # QML extension for QWebEngine
                  -skip qtwebglplugin               # No need browser OpenGL extention support
                  ${DROP_QTWEBENGINE_DEPS}


    # Exemple of code to install only one Qt module for hacking purpose (aka qtbase here)
    #BUILD_COMMAND cd <SOURCE_DIR> && $(MAKE) module-qtbase
    #INSTALL_COMMAND cd <SOURCE_DIR> && $(MAKE) module-qtbase-install_subtargets

    UPDATE_COMMAND ""
    BUILD_IN_SOURCE 1
    ALWAYS 0
)
