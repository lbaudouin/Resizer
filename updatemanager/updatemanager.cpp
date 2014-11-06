#include "updatemanager.h"

UpdateManager::UpdateManager(QWidget *parent) :
    QWidget(parent), manager(new QNetworkAccessManager), discretUpdate(false)
{
    progress = new QProgressBarDialog;
    progress->setFormat("%v"+tr("Kb")+"/%m"+tr("Kb"));
    progress->setMinimum(0);
}

void UpdateManager::setVersion(QString version)
{
    version = version.trimmed();
    if(!isValidVersion(version)){
        //QMessageBox::warning(this,tr("Warning"),tr("Invalid version"));
        currentVersion = "0.0.0";
        return;
    }
    currentVersion = version;
}

bool UpdateManager::isValidVersion(QString version)
{
    QStringList list = version.split(".",QString::SkipEmptyParts);
    if(list.size()!=3) return false;
    bool ok;
    if(list.at(0).toInt(&ok)<0) return false;
    if(!ok) return false;
    if(list.at(1).toInt(&ok)<0) return false;
    if(!ok) return false;
    if(list.at(2).toInt(&ok)<0) return false;
    if(!ok) return false;
    return true;
}

void UpdateManager::getMessage(QString url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url.isEmpty()?messageUrl:url));
    connect(manager->get(request),SIGNAL(finished()),this,SLOT(displayMessage()));
}

void UpdateManager::displayMessage()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    if(reply->error()!=QNetworkReply::NoError){
        return;
    }
    QString message(reply->readAll());

    if(message.isEmpty())
        return;

    QSettings settings(QCoreApplication::applicationName(),"config");
    QDateTime previous = settings.value("message/date").toDateTime();

    QString header;

    if(message.startsWith(":date=")){
        message.remove(0,6);
        QString datetimeString = message.section(" ",0,0);
        QDateTime datetime = QDateTime::fromString(datetimeString,Qt::ISODate);

        if(previous.isValid()){
            if(datetime<=previous)
                return;
        }
        settings.setValue("message/date",datetime);

        message.remove(datetimeString);
        while(message.startsWith(' '))
            message.remove(0,1);

        header = tr("Date:") + QString(" %1\n\n").arg(datetime.toString(Qt::TextDate));
    }else{
        return;
    }

    QMessageBox mess;
    mess.setWindowTitle(tr("Message"));
    if(message.startsWith(":critical ")){
        mess.setIcon(QMessageBox::Critical);
        message.remove(0,10);
    }else if(message.startsWith(":warning ")){
        mess.setIcon(QMessageBox::Warning);
        message.remove(0,9);
    }else if(message.startsWith(":question ")){
        mess.setIcon(QMessageBox::Question);
        message.remove(0,10);
    }else if(message.startsWith(":information ")){
        mess.setIcon(QMessageBox::Critical);
        message.remove(0,13);
    }else{
        mess.setIcon(QMessageBox::NoIcon);
    }

    if(!header.isEmpty()){
        message.prepend(header);
    }

    mess.setText(message);
    mess.exec();
}

void UpdateManager::startUpdate(QString url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url.isEmpty()?versionUrl:url));
    connect(manager->get(request),SIGNAL(finished()),this,SLOT(checkVersion()));
}

void UpdateManager::checkVersion()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());

    if(reply->error()!=QNetworkReply::NoError && !discretUpdate){
        QMessageBox::warning(this,tr("Warning"),tr("Error while downloading version: %1").arg(reply->errorString()));
        return;
    }

    QString versionOnServer = reply->readAll();
    versionOnServer = versionOnServer.trimmed();

    if(!isValidVersion(versionOnServer)){
        if(!discretUpdate)
            QMessageBox::information(this,tr("Information"),tr("Invalid version format on server"));
        return;
    }

    if(currentVersion<versionOnServer){
        int button = QMessageBox::information(this,tr("Information"),tr("New version available : %1").arg(versionOnServer),tr("Don't download"),tr("Download"),"",1,0);
        if(button==1)
            getExec();
    }else{
        QString message = tr("No newer version available");
        emit this->status(message);
        if(!discretUpdate)
            QMessageBox::information(this,tr("Information"),message);
    }
}

void UpdateManager::getExec(QString url)
{
    QNetworkRequest request;
    request.setUrl(QUrl(url.isEmpty()?execUrl:url));
    execReply = manager->get(request);
    connect(execReply,SIGNAL(finished()),this,SLOT(saveExec()));
    connect(execReply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));

    connect(progress,SIGNAL(cancel()),this,SLOT(abort()));
    progress->show();
}

void UpdateManager::saveExec()
{
    QNetworkReply* reply = qobject_cast<QNetworkReply*>(sender());
    progress->close();

    if(reply->error()!=QNetworkReply::NoError){
        QMessageBox::warning(this,tr("Warning"),tr("Error while downloading update: %1").arg(reply->errorString()));
        return;
    }

    /*QString filename = QFileInfo(reply->url().toString()).fileName();
    if(!filename.endsWith(".exe")){
        QMessageBox::warning(this,tr("Warning"),tr("Error in filename: %1").arg(filename));
        return;
    }*/

    QString filename = "TabZ-update.exe";

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
    QDir dir(QDesktopServices::storageLocation(QDesktopServices::TempLocation));
    if(!dir.exists()){
        dir.mkpath(QDesktopServices::storageLocation(QDesktopServices::TempLocation));
    }
#else
    QDir dir(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    if(!dir.exists()){
        dir.mkpath(QStandardPaths::writableLocation(QStandardPaths::TempLocation));
    }
#endif

    QFile file(dir.absoluteFilePath(filename));
    file.open(QFile::WriteOnly);
    file.write(reply->readAll());
    file.close();

    //Restart software
    QMessageBox mess;
    mess.setText(tr("Please restart software to use the new version"));
    mess.addButton(tr("Restart"),QMessageBox::AcceptRole);
    mess.addButton(tr("Do not restart"),QMessageBox::RejectRole);
    mess.setIcon(QMessageBox::Question);
    mess.setDetailedText(tr("%1Kb downloaded").arg((int)(file.size()/1000)));

    int button = mess.exec();
    if(button==QMessageBox::AcceptRole){
        emit restart(QFileInfo(file).absoluteFilePath());
    }
    return;
}

void UpdateManager::downloadProgress(qint64 done, qint64 total)
{
    progress->setMaximum(total/1000);
    progress->setValue(done/1000);
}

void UpdateManager::abort()
{
    if(execReply)
        execReply->abort();
    progress->close();
}

void UpdateManager::setDiscret(bool discret)
{
    discretUpdate = discret;
}

int UpdateManager::getVersionID(QString version)
{
    QStringList n = version.split(".");
    if(n.size()!=3) return 0;
    return n.at(0).toInt()*100*100 + n.at(1).toInt()*100 + n.at(2).toInt();
}
