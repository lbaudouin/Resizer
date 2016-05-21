#ifndef RESIZER_UNITY_H
#define RESIZER_UNITY_H

#define UNITY_PLUGIN_VERSION "0.1.0"

#include <unity/unity/unity.h>

#include "plugininterface.h"

#include <QEventLoop>
#include <QTimer>

class ResizerUnity : public Interface
{
    Q_OBJECT
    Q_INTERFACES(Interface)

public:
    ResizerUnity();
    QString version();

private:
    UnityLauncherEntry *unity;

signals:
    
public slots:
    void updateProgressBar(int min, int max, int val);
    void updateNumber(int number);
    void finished();
};

#endif // RESIZER_UNITY_H
