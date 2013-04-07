#include <QtGui/QApplication>
#include "qtsingleapplication.h"
#include "resize.h"

int main(int argc, char *argv[])
{

    QtSingleApplication instance("Resize", argc, argv);

    QString message;
    for(int a = 1; a < argc; ++a) {
        message += QString::fromUtf8(argv[a]);
        if (a < argc-1)
            message += " ";
    }

    if(instance.sendMessage(message))
        return 0;

    Resize w;
    w.handleMessage(message);
    w.show();

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)), &w, SLOT(handleMessage(const QString&)));

    instance.setActivationWindow(&w, false);

    QObject::connect(&w, SIGNAL(needToShow()), &instance, SLOT(activateWindow()));


    return instance.exec();
}
