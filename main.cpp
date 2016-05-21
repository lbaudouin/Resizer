#define CURRENT_VERSION "0.2.7"

#include <QApplication>
#include <QTranslator>
#include <QLibraryInfo>

#include "resizer.h"

#ifdef Q_OS_WIN
#include "updatemanager/updatemanager.h"
#endif

#include <singleapplication.h>

/** Main function
 * Test if one instance is already running
 * Translate in french if needed
 * Download update for WINDOWS
**/
int main(int argc, char *argv[])
{
    QCoreApplication::setApplicationName( "resizer" );

    SingleApplication instance(argc, argv);
    instance.setWindowIcon( QIcon(":/images/resizer" ) );

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
    w.setVersion(CURRENT_VERSION);
    w.show();

    if(instance.arguments().contains("--open-folder")){
        w.pressOpenFolder();
    }
    if(instance.arguments().contains("--open-file")){
        w.pressOpenFiles();
    }

    QObject::connect( &instance, SIGNAL(newArguments(QStringList)), &w, SLOT(addFilesAndFolders(QStringList)));
    QObject::connect( &instance, &SingleApplication::showUp, &w, &Resizer::raise );

    QStringList args = instance.arguments();
    args.removeFirst();
    w.addFilesAndFolders( args );

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
