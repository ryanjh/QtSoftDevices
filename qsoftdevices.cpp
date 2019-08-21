#include <QDebug> //TODO: use Qt logging categories
#include "qsoftdevices.h"

QSoftDevices::QSoftDevices(QIODevice *serialDevice, QObject *parent)
    : serialDevice(serialDevice)
{
    open(QIODevice::ReadWrite); //TODO: way to set mode
    QObject::connect( //TODO: remove debug only
        serialDevice, &QIODevice::bytesWritten,
        [](qint64 bytes) { qDebug() << "handleBytesWritten" << bytes; }
    );
    QObject::connect(
        serialDevice, &QIODevice::readyRead,
        [this, &serialDevice]() {
            Q_ASSERT(serialDevice);
            forever {
                if (!bytesAvailable())
                    return 0;
                emit readyRead();
            }
        }
    );
}

/*!
    \reimp
*/
qint64 QSoftDevices::readData(char *data, qint64 maxSize)
{
    Q_ASSERT(data);
    Q_ASSERT(serialDevice);
    char event[] = {0x00, 0x00, 0x00};
    if (serialDevice->peek(event, sizeof(event)) != sizeof(event))
        return 0;

    qint64 eventSize = event[0] + (event[1] << 8);
    //Q_ASSERT(event[2] == EVTOPCODE);
    if (serialDevice->bytesAvailable() < eventSize + 2)
        return 0;

    qint64 readSize = eventSize - 1;
    if (maxSize < readSize)
        return 0;

    Q_ASSERT(serialDevice->skip(sizeof(event)) == sizeof(event));
    Q_ASSERT(serialDevice->read(data, readSize) == readSize);
    qDebug() << "QSoftDevices::readData" << sizeof(event) + readSize;
    return readSize;
}

/*!
    \reimp
*/
qint64 QSoftDevices::writeData(const char *data, qint64 maxSize)
{
    Q_ASSERT(data);
    Q_ASSERT(serialDevice);
    char command[] = {(maxSize + 1) & 0xFF, ((maxSize + 1) >> 8) & 0xFF, CMDOPCODE};
    Q_ASSERT(serialDevice->write(command, sizeof(command)) == sizeof(command));
    Q_ASSERT(serialDevice->write(data, maxSize) == maxSize);
    qDebug() << "QSoftDevices::writeData" << sizeof(command) + maxSize;
    return maxSize;
}

/*!
    \reimp
*/
qint64 QSoftDevices::bytesAvailable() const
{
    Q_ASSERT(serialDevice);
    char event[] = {0x00, 0x00, 0x00};
    if (serialDevice->peek(event, sizeof(event)) != sizeof(event))
        return 0;

    qint64 eventSize = event[0] + (event[1] << 8);
    Q_ASSERT(eventSize); // at least one byte

    if (serialDevice->bytesAvailable() < eventSize + 2)
        return 0;

    return eventSize - 1;
}
