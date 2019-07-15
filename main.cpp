#include <QCoreApplication>
#include <QSerialPort>
#include <QDebug>

#include "qsoftdevices.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSerialPort serialPort;
    serialPort.setPortName("/dev/ttyACM0");
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

    QByteArray cmd3(std::begin<char>({0x03, 0x00, 0x00, 0x4c, 0x00}), 5);
    softDevice.sendCommand(cmd3);
    softDevice.sendCommand(cmd3);
    softDevice.sendCommand(cmd3);

    //echo -e '\x06\x00\x00\x02\x03\x04\x05\x06' > /dev/ttyACM0
    //cmd: 0300 00 4c 00
    //evt: 0700 01 4c 00 00000000
    QByteArray command(std::begin<char>({0x03, 0x00, 0x00, 0x4c, 0x00}), 5);
    softDevice.write(command);


    return a.exec();
}
