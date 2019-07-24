#include <QDebug> //TODO: use Qt logging categories
#include "qsoftdevices.h"

QSoftDevices::QSoftDevices(QIODevice *serialDevice, QObject *parent)
    : serialDevice(serialDevice)
{
    open(QIODevice::ReadWrite); //TODO: way to set mode
    QObject::connect(serialDevice, &QIODevice::readyRead, this, &QSoftDevices::handleReadyRead);
    QObject::connect( //TODO: remove debug only
        serialDevice, &QIODevice::bytesWritten,
        [](qint64 bytes) { qDebug() << "handleBytesWritten" << bytes; }
    );
    QObject::connect(
        this, &QSoftDevices::scheduleWrite,
        [this](QRingBuffer *commands) { transmitCommands(commands);}
    );
}

/*!
    \reimp
*/
qint64 QSoftDevices::readData(char *data, qint64 maxSize)
{
    Q_ASSERT(data);
    Q_ASSERT(serialDevice);
    return serialDevice->read(data, maxSize);
}

/*!
    \reimp
*/
qint64 QSoftDevices::writeData(const char *data, qint64 maxSize)
{
    Q_ASSERT(data);
    char command[] = {(maxSize + 1) & 0xFF, ((maxSize + 1) >> 8) & 0xFF, 0x00};
    commandBuffer.append(command, sizeof(command));
    commandBuffer.append(data, maxSize);
    emit scheduleWrite(&commandBuffer);
    // the scheduler will transmit all the buffered commands to device
    return maxSize;
}

/*!
    \reimp
*/
qint64 QSoftDevices::bytesAvailable() const
{
    Q_ASSERT(serialDevice);
    return serialDevice->bytesAvailable();
}

qint64 QSoftDevices::transmitCommands(QRingBuffer *commands)
{
    Q_ASSERT(commands);
    Q_ASSERT(serialDevice);
    qint64 writeSize = 0;
    //TODO: check size of written and rollback m_commands, then emit scheduleWrite
    if (!commands->isEmpty()) {
        writeSize = serialDevice->write(commands->read());
        qDebug() << "transmitCommands" << writeSize;
    }
    return writeSize;
}

qint64 QSoftDevices::receiveEvents(QRingBuffer *events)
{
    Q_ASSERT(events);
    Q_ASSERT(serialDevice);
    if (serialDevice->bytesAvailable() >= COMMANDSIZE) {
        // remove HCI
    }
    qDebug() << "receiveEvents";

    return 0;
}

void QSoftDevices::handleReadyRead()
{
    qDebug() << "handleReadyRead" << bytesAvailable();
    // ----------------------------------------------------------
    // TODO: better option
    if (bytesAvailable() >= 3) {
        // remove HCI
    }
    // ----------------------------------------------------------
    QByteArray event = readAll();
    //TODO: buffer event and emit receivedEvent
    emit receivedEvent(event);
}
