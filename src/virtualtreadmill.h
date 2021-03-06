#ifndef VIRTUALTREADMILL_H
#define VIRTUALTREADMILL_H

#include <QObject>

#include <QtBluetooth/qlowenergyadvertisingdata.h>
#include <QtBluetooth/qlowenergyadvertisingparameters.h>
#include <QtBluetooth/qlowenergycharacteristic.h>
#include <QtBluetooth/qlowenergycharacteristicdata.h>
#include <QtBluetooth/qlowenergydescriptordata.h>
#include <QtBluetooth/qlowenergycontroller.h>
#include <QtBluetooth/qlowenergyservice.h>
#include <QtBluetooth/qlowenergyservicedata.h>
#include <QtCore/qbytearray.h>
#ifndef Q_OS_ANDROID
#include <QtCore/qcoreapplication.h>
#else
#include <QtGui/qguiapplication.h>
#endif
#include <QtCore/qlist.h>
#include <QtCore/qloggingcategory.h>
#include <QtCore/qscopedpointer.h>
#include <QtCore/qtimer.h>

#include "treadmill.h"

class virtualtreadmill: public QObject
{
    Q_OBJECT
public:
    virtualtreadmill(treadmill* t);    
    bool connected();

private:
    QLowEnergyController* leController;
    QLowEnergyService* service;
    QLowEnergyService* serviceHR;
    QLowEnergyAdvertisingData advertisingData;
    QLowEnergyServiceData serviceData;
    QLowEnergyServiceData serviceDataHR;
    QTimer treadmillTimer;    
    treadmill* treadMill;

signals:
    void debug(QString string);

private slots:
    void characteristicChanged(const QLowEnergyCharacteristic &characteristic, const QByteArray &newValue);
    void treadmillProvider();
    void reconnect();
};

#endif // VIRTUALTREADMILL_H
