#include <QDebug>

#include "qsoftdevices.h"

QSoftDevices::QSoftDevices(QSerialPort *serialPort, QObject *parent) :
    QObject(parent),
    m_serialPort(serialPort)
{
    connect(m_serialPort, &QSerialPort::readyRead, this, &QSoftDevices::handleReadyRead);
}

void QSoftDevices::handleReadyRead()
{
    qDebug() << m_serialPort->readAll();
}
