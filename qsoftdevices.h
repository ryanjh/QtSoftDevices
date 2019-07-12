#ifndef QSOFTDEVICES_H
#define QSOFTDEVICES_H

#include <QSerialPort>

class QSoftDevices : public QObject
{
    Q_OBJECT

public:
    explicit QSoftDevices(QSerialPort *serialPort, QObject *parent = nullptr);

private slots:
    void handleReadyRead();

private:
    QSerialPort *m_serialPort = nullptr;
};

#endif // QSOFTDEVICES_H
