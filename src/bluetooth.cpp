#include "bluetooth.h"
#include <QFile>
#include <QDateTime>
#include <QMetaEnum>
#include <QBluetoothLocalDevice>

bluetooth::bluetooth(bool logs, QString deviceName, bool noWriteResistance, bool noHeartService, uint32_t pollDeviceTime, bool noConsole) : QObject(nullptr)
{
    QLoggingCategory::setFilterRules(QStringLiteral("qt.bluetooth* = true"));
    filterDevice = deviceName;
    this->noWriteResistance = noWriteResistance;
    this->noHeartService = noHeartService;
    this->pollDeviceTime = pollDeviceTime;
    this->noConsole = noConsole;

    if(logs)
    {
        debugCommsLog = new QFile("debug-" + QDateTime::currentDateTime().toString() + ".log");
        debugCommsLog->open(QIODevice::WriteOnly | QIODevice::Unbuffered);
    }

#ifndef WIN32
    if(!QBluetoothLocalDevice::allDevices().count())
    {
        debug("no bluetooth dongle found!");
    }
    else
#endif
    {
        // Create a discovery agent and connect to its signals
        discoveryAgent = new QBluetoothDeviceDiscoveryAgent(this);
        connect(discoveryAgent, SIGNAL(deviceDiscovered(QBluetoothDeviceInfo)),
                this, SLOT(deviceDiscovered(QBluetoothDeviceInfo)));

        // Start a discovery
        discoveryAgent->start();
    }
}

void bluetooth::debug(QString text)
{
    QString debug = QDateTime::currentDateTime().toString() + " " + QString::number(QDateTime::currentMSecsSinceEpoch()) + " " + text + '\n';
    if(debugCommsLog)
    {
        debugCommsLog->write(debug.toLocal8Bit());
        debugCommsLog->flush();
        qDebug() << debug;
    }
}

void bluetooth::deviceDiscovered(const QBluetoothDeviceInfo &device)
{
    debug("Found new device: " + device.name() + " (" + device.address().toString() + ')' + " " + device.majorDeviceClass() + ":" + device.minorDeviceClass());
    /* only on qt 5.12
    foreach(quint16 i, device.manufacturerIds())
    {
        debug("manufacturer id: " + QString::number(i) + " -> " + device.manufacturerData(i));
    }*/

    bool filter = true;
    if(filterDevice.length())
    {
        filter = (device.name().compare(filterDevice, Qt::CaseInsensitive) == 0);
    }

    if(device.name().startsWith("Domyos-Bike") && !device.name().startsWith("DomyosBridge") && filter)
    {
        discoveryAgent->stop();
        domyosBike = new domyosbike(noWriteResistance, noHeartService);
        emit(deviceConnected());
        connect(domyosBike, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(domyosBike, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        domyosBike->deviceDiscovered(device);
    }
    else if(device.name().startsWith("Domyos") && !device.name().startsWith("DomyosBridge") && filter)
    {
        discoveryAgent->stop();
        domyos = new domyostreadmill(this->pollDeviceTime, noConsole);
        emit(deviceConnected());
        connect(domyos, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(domyos, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        domyos->deviceDiscovered(device);
    }
    else if((device.name().startsWith("TRX ROUTE KEY")) && filter)
    {
        discoveryAgent->stop();
        toorx = new toorxtreadmill();
        emit(deviceConnected());
        connect(toorx, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(toorx, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        toorx->deviceDiscovered(device);
    }
    else if((device.name().startsWith("TOORX")) && filter)
    {
        discoveryAgent->stop();
        trxappgateusb = new trxappgateusbtreadmill();
        emit(deviceConnected());
        connect(trxappgateusb, SIGNAL(disconnected()), this, SLOT(restart()));
        connect(trxappgateusb, SIGNAL(debug(QString)), this, SLOT(debug(QString)));
        trxappgateusb->deviceDiscovered(device);
    }
}

void bluetooth::restart()
{
    if(domyos)
    {
        delete domyos;
        domyos = 0;
    }
    if(domyosBike)
    {
        delete domyosBike;
        domyosBike = 0;
    }
    if(toorx)
    {
        delete toorx;
        toorx = 0;
    }
    if(trxappgateusb)
    {
        delete trxappgateusb;
        trxappgateusb = 0;
    }
    discoveryAgent->start();
}

bluetoothdevice* bluetooth::device()
{
    if(domyos)
        return domyos;
    else if(domyosBike)
        return domyosBike;
    else if(toorx)
        return toorx;
    else if(trxappgateusb)
        return trxappgateusb;
    return nullptr;
}
