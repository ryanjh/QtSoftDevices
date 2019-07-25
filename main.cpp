#include <QCoreApplication>
#include <QSerialPort>
#include <QDebug>

#include "qsoftdevices.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSerialPort serialPort;
    serialPort.setPortName("/dev/ttyACM0");
    //serialPort.setPortName("COM10");
    serialPort.setBaudRate(QSerialPort::Baud115200);
    serialPort.setDataBits(QSerialPort::Data8);
    serialPort.setParity(QSerialPort::NoParity);
    serialPort.setStopBits(QSerialPort::OneStop);
    serialPort.setFlowControl(QSerialPort::NoFlowControl);

    if (!serialPort.open(QIODevice::ReadWrite)) {
        qDebug() << "Failed to open";
        return 1;
    }

    QSoftDevices softDevice(&serialPort);
    QObject::connect(
        &softDevice, &QSoftDevices::readyRead,
        [&softDevice]() {
            QByteArray event = softDevice.read(softDevice.bytesAvailable());
            qDebug() << "QSoftDevices::receivedEvent" << event.toHex() << event.size();
        }
    );

    QVarLengthArray<char> varLenArray{0x4c, 0x00};
    softDevice.writeCommand(varLenArray);
    softDevice.writeCommand({0x4c, 0x00});
    // NOTE: can't get events after invalid commands
    //softDevice.writeCommand({0x4c}); // invalid command
    //softDevice.writeCommand({0x4c, 0x00, 0x00}); // invalid command
    //softDevice.writeCommand({0x4c, 0x00, 0x00, 0x00}); // invalid command

    QByteArray byteArray(std::begin<char>({0x4c, 0x00}), 2);
    softDevice.writeCommand({byteArray.begin(), byteArray.end()});
    softDevice.writeCommand(QVarLengthArray<char>(byteArray.begin(), byteArray.end()));

    //echo -e '\x06\x00\x00\x02\x03\x04\x05\x06' > /dev/ttyACM0
    //cmd: 0300 00 4c 00
    //evt: 0700 01 4c 00 00000000
    QByteArray command(std::begin<char>({0x4c, 0x00}), 2);
    softDevice.write(command);

    return a.exec();
}
