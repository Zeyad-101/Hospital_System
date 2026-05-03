QT += core gui widgets

CONFIG += c++17

TARGET = AlNoorHospital

TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    dashboardpage.cpp \
    appointmentspage.cpp \
    patientspage.cpp \
    financialpage.cpp \
    schedulepage.cpp

HEADERS += \
    mainwindow.h \
    dashboardpage.h \
    appointmentspage.h \
    patientspage.h \
    financialpage.h \
    schedulepage.h \
    backend.h

RESOURCES += resources.qrc
