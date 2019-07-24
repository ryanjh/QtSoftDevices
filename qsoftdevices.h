#ifndef QSOFTDEVICES_H
#define QSOFTDEVICES_H

#include <QIODevice>
#include <QVarLengthArray>
#include <private/qringbuffer_p.h>

class QSoftDevices : public QIODevice
{
    Q_OBJECT

public:
    QSoftDevices(QIODevice *serialDevice, QObject *parent = nullptr);
    virtual ~QSoftDevices() {} // TODO: disconnect

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 bytesAvailable() const override;

    inline void writeCommand(const QVarLengthArray<char> bytes)
    { writeData(bytes.constData(), bytes.size()); }
    //TODO: readEvent ?

    const quint32 COMMANDSIZE = 3;

protected:
    qint64 transmitCommands(QRingBuffer *commands);
    qint64 receiveEvents(QRingBuffer *events);

signals:
    void scheduleWrite(QRingBuffer *commands);
    void receivedEvent(QByteArray bytes);

private slots:
    void handleReadyRead();

private:
    QIODevice *serialDevice = nullptr;
    QRingBuffer commandBuffer;
    QRingBuffer eventBuffer;
};

#endif // QSOFTDEVICES_H
