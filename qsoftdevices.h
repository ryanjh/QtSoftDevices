#ifndef QSOFTDEVICES_H
#define QSOFTDEVICES_H

#include <QIODevice>
#include <private/qringbuffer_p.h>

class QSoftDevices : public QIODevice
{
    Q_OBJECT

public:
    QSoftDevices(QIODevice *serialDevice, QObject *parent = nullptr);
    virtual ~QSoftDevices();

    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;
    qint64 bytesAvailable() const override;

    void sendCommand(const QByteArray bytes);

signals:
    void receivedEvent(QByteArray bytes);

private slots:
    void handleReadyRead();
    void handleBytesWritten(qint64 bytes);

private:
    QIODevice *m_serialDevice = nullptr;
    QRingBuffer m_commands;
    QRingBuffer m_events;
};

#endif // QSOFTDEVICES_H
