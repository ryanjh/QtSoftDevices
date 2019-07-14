#include <QDebug>

#include "qsoftdevices.h"

QSoftDevices::QSoftDevices(QIODevice *serialDevice, QObject *parent)
    : m_serialDevice(serialDevice)
{
    connect(m_serialDevice, &QIODevice::readyRead, this, &QSoftDevices::handleReadyRead);
    connect(m_serialDevice, &QIODevice::bytesWritten, this, &QSoftDevices::handleBytesWritten);
    //connect(m_serialDevice, &QIODevice::errorOccurred, this, &QSoftDevices::handleError);
}

QSoftDevices::~QSoftDevices()
{
}

void QSoftDevices::handleReadyRead()
{
    qDebug() << "handleReadyRead" << m_serialDevice->bytesAvailable();
    qDebug() << m_serialDevice->readAll().toHex();
}

void QSoftDevices::handleBytesWritten(qint64 bytes)
{
    qDebug() << "handleBytesWritten" << bytes;
}

void QSoftDevices::handleError(QSerialPort::SerialPortError serialPortError)
{
    qDebug() << "handleError" << serialPortError;
}

void QSoftDevices::write(const QByteArray &writeData)
{
    const qint64 bytesWritten = m_serialDevice->write(writeData);
    if (bytesWritten == -1) {
        qDebug() << "Failed to write the data";
    } else if (bytesWritten != writeData.size()) {
        qDebug() << "Failed to write all the data";
    }
}

/*!
    \reimp
*/
qint64 QSoftDevices::readData(char *data, qint64 maxSize)
{
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return qint64(0);
}

/*!
    \reimp
*/
qint64 QSoftDevices::writeData(const char *data, qint64 maxSize)
{
    //Q_D(QSerialPort);
    //return d->writeData(data, maxSize);
    Q_UNUSED(data);
    Q_UNUSED(maxSize);
    return 0;
}


