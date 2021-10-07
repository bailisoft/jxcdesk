#CONFIG += c++11

QT += core gui sql network printsupport

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TEMPLATE = app

DEFINES += QT_DEPRECATED_WARNINGS
DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000
##DEFINES += QT_MESSAGELOGCONTEXT     # for redirect debug info to a file

## message($$[QT_INSTALL_PREFIX])
## INCLUDEPATH += $$[QT_INSTALL_PREFIX]/src/3rdparty/zlib

HEADERS += \
    comm/pinyincode.h \
    comm/expresscalc.h \
    comm/bsflowlayout.h \
    comm/lxstringtablemodel.h \
    main/bsloading.h \
    main/bssocket.h \
    misc/bschat.h \
    misc/bswinreg.h \
    third/tinyAES/aes.h \
    main/bailicode.h \
    main/bailidata.h \
    main/bailifunc.h \
    main/bailiwins.h \
    main/bailigrid.h \
    main/bailiedit.h \
    main/bailisql.h \
    main/bsmain.h \
    tools/bsbarcodemaker.h \
    dialog/bsabout.h \
    dialog/bspapersizedlg.h \
    dialog/bsbatchbarcodesdlg.h \
    dialog/bsloginguide.h \
    dialog/bspickdatedlg.h \
    dialog/lxwelcome.h \
    dialog/bssetpassword.h \
    dialog/bscopyimportsheetdlg.h \
    misc/lxbzprinter.h \
    misc/lxbzprintsetting.h \
    misc/bshistorywin.h \
    misc/bsfielddefinedlg.h \
    misc/bsdebug.h

SOURCES += main/main.cpp \
    comm/pinyincode.cpp \
    comm/expresscalc.cpp \
    comm/bsflowlayout.cpp \
    comm/lxstringtablemodel.cpp \
    main/bsloading.cpp \
    main/bssocket.cpp \
    misc/bschat.cpp \
    misc/bswinreg.cpp \
    third/tinyAES/aes.c \
    main/bailicode.cpp \
    main/bailidata.cpp \
    main/bailifunc.cpp \
    main/bailiwins.cpp \
    main/bailigrid.cpp \
    main/bailiedit.cpp \
    main/bailisql.cpp \
    main/bsmain.cpp \
    dialog/bsabout.cpp \
    tools/bsbarcodemaker.cpp \
    dialog/bspapersizedlg.cpp \
    dialog/bsbatchbarcodesdlg.cpp \
    dialog/bsloginguide.cpp \
    dialog/bspickdatedlg.cpp \
    dialog/lxwelcome.cpp \
    dialog/bssetpassword.cpp \
    dialog/bscopyimportsheetdlg.cpp \
    misc/lxbzprinter.cpp \
    misc/lxbzprintsetting.cpp \
    misc/bshistorywin.cpp \
    misc/bsfielddefinedlg.cpp

RESOURCES += \
    resources/all.qrc

DISTFILES +=


############################ Below is platform difference ############################

#message($$QMAKESPEC)    #Used to show what's default spec when execute qmake without -spec option.

TARGET = BailiR17Desk

win32 {     #win32 means all windows platform not only win_x86
    RC_ICONS = resources/winlogo.ico

    SPECVALUE_X64FLAG = $$find(QMAKESPEC, _64)         #test to see $$QMAKESPEC's value
    isEmpty(SPECVALUE_X64FLAG) {
        #DESTDIR = $$PWD/../../BuildOuts/R17_distribute/win32
        message("WIN x32 compiler")
    }
    else {
        #DESTDIR = $$PWD/../../BuildOuts/R17_distribute/win64
        message("WIN x64 compiler")
    }
}
else {
    macx {
        #DESTDIR = /Users/roger/BailiR17Dist
        ICON = resources/maclogo.icns        
        message("Mac OS X compiler")
    }
    else {
        SPECVALUE_X64FLAG = $$find(QMAKESPEC, 64)            #test to see $$QMAKESPEC's value
        isEmpty(SPECVALUE_X64FLAG) {
            #DESTDIR = /home/roger/BailiR17Dist32
            message("UNIX x32 compiler")
        }
        else {
            #DESTDIR = /home/roger/BailiR17Dist64
            message("UNIX x64 compiler")
        }
    }
}

