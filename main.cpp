#include <QCoreApplication>
#include <QSerialPort>
#include <QDebug>

#include "qsoftdevices.h"

int main(int argc, char *argv[])
{
    QCoreApplication a(argc, argv);

    QSerialPort serialPort;
    serialPort.setPortName("COM10");
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

    return a.exec();
}
