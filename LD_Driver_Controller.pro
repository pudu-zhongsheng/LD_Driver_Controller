QT       += core gui serialport charts multimedia

greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

CONFIG += c++17

RC_ICONS=logo/LD_Control.ico

SOURCES += \
    communication/drivergeneral.cpp \
    devices/driver/driver1ch/driver1ch.cpp \
    main.cpp \
    mainwindow.cpp \
    communication/driverprotocol.cpp \
    communication/eleload_itplus.cpp \
    communication/cl_twozerozeroacom.cpp \
    communication/protocol.cpp \
    devices/driver/driverbase.cpp \
    devices/driver/driverwidget.cpp \
    devices/driver/driver8ch/driver8ch.cpp \
    devices/driver/widgets/controlwidget.cpp \
    devices/driver/widgets/driverinfowidget.cpp \
    devices/driver/widgets/scanwidget.cpp \
    devices/driver/widgets/sliderwidget.cpp \
    devices/driver/widgets/paramtablewidget.cpp \
    devices/load/loadbase.cpp \
    devices/load/it8512plus/it8512plus_widget.cpp \
    devices/meter/meterbase.cpp \
    devices/meter/cl200a/cl200awidget.cpp \
    menu/drivemenu.cpp \
    serial/serialutil.cpp \
    splash/splashscreen.cpp \
    util/config.cpp \
    util/datamanager.cpp \
    util/errorhandler.cpp \
    util/logger.cpp


HEADERS += \
    communication/drivergeneral.h \
    devices/driver/driver1ch/driver1ch.h \
    mainwindow.h \
    communication/driverprotocol.h \
    communication/eleload_itplus.h \
    communication/cl_twozerozeroacom.h \
    communication/protocol.h \
    devices/driver/driverbase.h \
    devices/driver/driverwidget.h \
    devices/driver/driver8ch/driver8ch.h \
    devices/driver/widgets/controlwidget.h \
    devices/driver/widgets/driverinfowidget.h \
    devices/driver/widgets/scanwidget.h \
    devices/driver/widgets/sliderwidget.h \
    devices/driver/widgets/paramtablewidget.h \
    devices/meter/meterbase.h \
    devices/meter/cl200a/cl200awidget.h \
    menu/drivemenu.h \
    serial/serialutil.h \
    splash/splashscreen.h \
    util/config.h \
    util/datamanager.h \
    util/errorhandler.h \
    util/logger.h \
    util/ToastMessage.h \
    devices/load/load_base.h \
    devices/load/it8512plus/it8512plus_widget.h
    

FORMS += \
    mainwindow.ui

RESOURCES += \
    logo.qrc \
    resources.qrc

# Default rules for deployment.
qnx: target.path = /tmp/$${TARGET}/bin
else: unix:!android: target.path = /opt/$${TARGET}/bin
!isEmpty(target.path): INSTALLS += target
