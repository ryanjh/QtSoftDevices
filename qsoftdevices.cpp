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

void QSoftDevices::sendCommand(const QVarLengthArray<char> bytes)
{
    m_commands.append(QByteArray(bytes.constData(), bytes.size()));
    qDebug() << "> QVarLengthArray: m_commands.nextDataBlockSize" << m_commands.nextDataBlockSize();
}

void QSoftDevices::sendCommand(const QByteArray *bytes)
{
    Q_ASSERT(bytes);
    m_commands.append(*bytes);
    qDebug() << "> QByteArray: m_commands.nextDataBlockSize" << m_commands.nextDataBlockSize();
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
    //TODO: another way to trigger m_commands flush
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
    // ----------------------------------------------------------
    // TODO: better option
    QByteArray header;
    qint16 size = qint16(maxSize + 1); // "\x03\x00\x00\x4c\x00"
    header.append(char(size & 0xFF));
    header.append(char((size >> 8) & 0xFF));
    header.append(char('\x00'));
    // ----------------------------------------------------------
    if (m_serialDevice->write(header, header.size()) != header.size()) {
        qDebug() << "### Error: writeData header fail";
        return 0;
    }
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
