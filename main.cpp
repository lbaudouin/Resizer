#define CURRENT_VERSION "0.2.1"

#include <QtGui/QApplication>
#include <QTranslator>
#include <QLibraryInfo>
#include "qtsingleapplication.h"
#include "resizer.h"

#ifdef Q_OS_WIN
#include "updatemanager/updatemanager.h"
#endif

/** Main function
 * Test if one instance is already running
 * Translate in french if needed
 * Download update for WINDOWS
**/
int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName( "resizer" );
    QtSingleApplication instance("resizer", argc, argv);
    instance.setWindowIcon( QIcon(":/images/resizer" ) );

    QString message;
    if(argc)
    for(int a = 1; a < argc; ++a) {
        message += QString::fromLocal8Bit(argv[a]);
        if (a < argc-1)
            message += "\n";
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

    Resizer w;
    w.handleMessage(message);
    w.setVersion(CURRENT_VERSION);
    w.show();

    if(instance.arguments().contains("--open-folder")){
        w.pressOpenFolder();
    }
    if(instance.arguments().contains("--open-file")){
        w.pressOpenFiles();
    }

    QObject::connect(&instance, SIGNAL(messageReceived(const QString&)), &w, SLOT(handleMessage(const QString&)));

    instance.setActivationWindow(&w, false);

    QObject::connect(&w, SIGNAL(needToShow()), &instance, SLOT(activateWindow()));

#ifdef Q_OS_WIN
    UpdateManager *up = new UpdateManager;
    QObject::connect(up,SIGNAL(restart(QString)),&w,SLOT(restart(QString)));
    up->setVersion(CURRENT_VERSION);
    up->setMessageUrl("http://lbaudouin.fr/RESIZER_MESSAGE");
    up->setVersionUrl("http://lbaudouin.fr/RESIZER_VERSION");
    up->setExecUrl("http://lbaudouin.fr/Resizer-update.exe");
    //up->setZipUrl("http://lbaudouin.fr/Resizer-update.zip");
    up->getMessage();
    up->setDiscret(true);
    up->startUpdate();
#endif

    return instance.exec();
}
