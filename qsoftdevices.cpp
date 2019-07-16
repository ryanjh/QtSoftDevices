#include <QDebug> //TODO: use Qt logging categories
#include "qsoftdevices.h"

QSoftDevices::QSoftDevices(QIODevice *serialDevice, QObject *parent)
    : m_serialDevice(serialDevice)
{
    this->open(QIODevice::ReadWrite); //TODO: way to set mode
    QObject::connect(m_serialDevice, &QIODevice::readyRead, this, &QSoftDevices::handleReadyRead);
    QObject::connect(m_serialDevice, &QIODevice::bytesWritten, this, &QSoftDevices::handleBytesWritten);
}

QSoftDevices::~QSoftDevices()
{
}

void QSoftDevices::sendCommand(const QByteArray bytes)
{
    m_commands.append(bytes);
    qDebug() << "> m_commands.nextDataBlockSize" << m_commands.nextDataBlockSize();
}

void QSoftDevices::handleReadyRead()
{
    qDebug() << "handleReadyRead" << this->bytesAvailable();
    qDebug() << this->readAll().toHex();
    //TODO: buffer event and emit receivedEvent
}

void QSoftDevices::handleBytesWritten(qint64 bytes)
{
    qDebug() << "handleBytesWritten" << bytes;
    //TODO: check size of written and rollback m_commands
    if (!m_commands.isEmpty())
        this->write(m_commands.read());
}

/*!
    \reimp
*/
qint64 QSoftDevices::readData(char *data, qint64 maxSize)
{
    Q_ASSERT(m_serialDevice);
    return m_serialDevice->read(data, maxSize);
}

/*!
    \reimp
*/
qint64 QSoftDevices::writeData(const char *data, qint64 maxSize)
{
    Q_ASSERT(m_serialDevice);
    return m_serialDevice->write(data, maxSize);
}

/*!
    \reimp
*/
qint64 QSoftDevices::bytesAvailable() const
{
    Q_ASSERT(m_serialDevice);
    return m_serialDevice->bytesAvailable();
}
