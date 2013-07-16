#include <QtGui/QApplication>
#include <QTranslator>
#include <QLibraryInfo>
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

    QString lang = QLocale::system().name().section('_', 0, 0);
    lang = lang.toLower();

    if(lang=="fr"){
        QTranslator *translator = new QTranslator();
        translator->load(QString(":/lang/lang_") + lang);
        qApp->installTranslator( translator );

        QTranslator *translatorQt = new QTranslator();
        translatorQt->load(QString("qt_") + lang, QLibraryInfo::location(QLibraryInfo::TranslationsPath));
        qApp->installTranslator( translatorQt );
    }

    Resize w;
    w.handleMessage(message);
    w.show();

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)), &w, SLOT(handleMessage(const QString&)));

    instance.setActivationWindow(&w, false);

    QObject::connect(&w, SIGNAL(needToShow()), &instance, SLOT(activateWindow()));


    return instance.exec();
}
