#include <QDebug> //TODO: use Qt logging categories
#include "qsoftdevices.h"

QSoftDevices::QSoftDevices(QIODevice *serialDevice, QObject *parent)
    : serialDevice(serialDevice)
{
    // NOTE: QIODevice::Unbuffered is needed or softDevice.read(event, bytes)
    // will read 16384. [qiodevice.cpp:1129]
    open(QIODevice::ReadWrite | QIODevice::Unbuffered); //TODO: way to set mode
    QObject::connect( //TODO: remove debug only
        serialDevice, &QIODevice::bytesWritten,
        [](qint64 bytes) { qDebug() << "handleBytesWritten" << bytes; }
    );
    QObject::connect(this, &QSoftDevices::scheduleWrite, &QSoftDevices::transmitCommands);
    QObject::connect(
        serialDevice, &QIODevice::readyRead,
        [this]() { receiveEvents(&eventBuffer); }
    );
}

/*!
    \reimp
*/
qint64 QSoftDevices::readData(char *data, qint64 maxSize)
{
    Q_ASSERT(data);
    Q_ASSERT(serialDevice);
    qint64 readSize = serialDevice->read(data, maxSize);
    return readSize; // TODO: remove HCI
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

    qint64 available = 0;
    qint64 eventSize = 0;
    char event[] = {0x00, 0x00, 0x00};
    forever {
        available = serialDevice->bytesAvailable();
        //qDebug() << "receiveEvents(peek)" << serialDevice->peek(available).toHex() << available;
        if (serialDevice->peek(event, sizeof(event)) != sizeof(event))
            return 0;

        qint64 eventSize = event[0] + (event[1] << 8) + 2;
        if (available < eventSize || event[2] != EVTOPCODE)
            return 0;

        emit receivedEvent(eventSize);
    }

    return 0;
}
