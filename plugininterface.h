#ifndef PLUGININTERFACE_H
#define PLUGININTERFACE_H

#include <QtPlugin>

class Interface : public QObject
{
    Q_OBJECT

public :
    virtual ~Interface() {}
    virtual QString version() = 0;

public slots:
    virtual void updateProgressBar(int min, int max, int val) = 0;
    virtual void updateNumber(int number) = 0;
    virtual void finished() = 0;

} ;

Q_DECLARE_INTERFACE( Interface , "resizer.Interface" )


#endif // PLUGININTERFACE_H
