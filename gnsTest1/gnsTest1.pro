TARGET		 = gnsTest1
QT          += opengl

CFLAGS      -= UNICODE
DESTDIR     = $(SolutionDir)/bin/$(PlatformName)/$(ConfigurationName)
DEFINES		+= _CRT_SECURE_NO_WARNINGS
DEFINES     += _UNICODE

RESOURCES	+= gnsTest1.qrc
RC_FILE 	+= icon.rc

INCLUDEPATH	+= ../libCommon
INCLUDEPATH	+= ../gnsinterface
INCLUDEPATH	+= ../libGnsProxy


#HEADERS  += main.h
SOURCES	  += main.cpp

HEADERS   += gnsViewWidget.h
SOURCES	  += gnsViewWidget.cpp

FORMS 		+= mainWindow.ui
SOURCES		+= mainWindow.cpp
HEADERS		+= mainWindow.h




