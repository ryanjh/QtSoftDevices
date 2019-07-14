#ifndef QSOFTDEVICES_H
#define QSOFTDEVICES_H

#include <QSerialPort>

class QSoftDevices : public QIODevice
{
    Q_OBJECT

public:
    explicit QSoftDevices(QIODevice *serialDevice, QObject *parent = nullptr);
    virtual ~QSoftDevices();

    void write(const QByteArray &writeData);
    qint64 readData(char *data, qint64 maxSize) override;
    qint64 writeData(const char *data, qint64 maxSize) override;

private slots:
    void handleReadyRead();
    void handleBytesWritten(qint64 bytes);
    void handleError(QSerialPort::SerialPortError error);

private:
    QIODevice *m_serialDevice = nullptr;
};

#endif // QSOFTDEVICES_H
