QT = core core-private
QT += serialport

CONFIG += c++11 console
CONFIG -= app_bundle

# The following define makes your compiler emit warnings if you use
# any feature of Qt which as been marked deprecated (the exact warnings
# depend on your compiler). Please consult the documentation of the
# deprecated API in order to know how to port your code away from it.
DEFINES += QT_DEPRECATED_WARNINGS

DEFINES += NRF_SD_BLE_API=6
DEFINES += ASIO_STANDALONE
DEFINES += HCI_LINK_CONTROL
DEFINES += SD_RPC_EXPORTS

# You can also make your code fail to compile if you use deprecated APIs.
# In order to do so, uncomment the following line.
# You can also select to disable deprecated APIs only up to a certain version of Qt.
#DEFINES += QT_DISABLE_DEPRECATED_BEFORE=0x060000    # disables all the APIs deprecated before Qt 6.0.0

SOURCES += main.cpp \
    qsoftdevices.cpp
libnrf-ble-driver-sd_api_v6.so.0.0.0
HEADERS += \
    qsoftdevices.h

INCLUDEPATH += $$PWD/../Nordic/pc-ble-driver/include/common
INCLUDEPATH += $$PWD/../Nordic/pc-ble-driver/include/common/config
INCLUDEPATH += $$PWD/../Nordic/pc-ble-driver/include/common/sdk_compat
INCLUDEPATH += $$PWD/../Nordic/pc-ble-driver/include/common/internal
INCLUDEPATH += $$PWD/../Nordic/pc-ble-driver/include/common/internal/transport
INCLUDEPATH += $$PWD/../Nordic/pc-ble-driver/include/sd_api_v6
INCLUDEPATH += $$PWD/../Nordic/pc-ble-driver/src/sd_api_common/sdk/components/libraries/util

DEPENDPATH += $$PWD/../Nordic/pc-ble-driver/include/common
DEPENDPATH += $$PWD/../Nordic/pc-ble-driver/include/common/config
DEPENDPATH += $$PWD/../Nordic/pc-ble-driver/include/common/sdk_compat
DEPENDPATH += $$PWD/../Nordic/pc-ble-driver/include/common/internal
DEPENDPATH += $$PWD/../Nordic/pc-ble-driver/include/common/internal/transport
DEPENDPATH += $$PWD/../Nordic/pc-ble-driver/include/sd_api_v6


unix:!macx: LIBS += -L/home/rychu/rychu_home/workspace/Nordic/pc-ble-driver/build/ -lnrf-ble-driver-sd_api_v6

INCLUDEPATH += $$PWD/.
DEPENDPATH += $$PWD/.
