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
    QString text(reply->readAll());

    if(text.isEmpty())
        return;

    QSettings settings(QCoreApplication::applicationName(),"config");
    QDateTime previous = settings.value("message/date").toDateTime();

    QStringList messages = text.split("\n");

    foreach(QString message, messages){
        QString header;
        QDateTime datetime;
        QMessageBox::Icon icon = QMessageBox::NoIcon;
        QString minVersion;
        QString maxVersion;
        bool permanent = false;

        while(message.startsWith(":")){
            QString info = message.section(" ",0,0).trimmed();

            if(info.startsWith(":date=")){
                info.remove(0,QString(":date=").length());
                datetime = QDateTime::fromString(info,Qt::ISODate);

                if(datetime.isValid()){
                    settings.setValue("message/date",datetime);
                }

                header = tr("Date:") + QString(" %1\n\n").arg(datetime.toString("DD-MM-YYY HH:mm::ss"));
            }

            if(info.startsWith(":icon=")){
                info.remove(0,QString(":icon=").length());
                if(info=="critical")
                    icon = QMessageBox::Critical;
                if(info=="warning")
                    icon = QMessageBox::Warning;
                if(info=="information")
                    icon = QMessageBox::Information;
                if(info=="question")
                    icon = QMessageBox::Question;
            }

            if(info.startsWith(":min=")){
                info.remove(0,QString(":min=").length());
                minVersion = info;
            }
            if(info.startsWith(":max=")){
                info.remove(0,QString(":max=").length());
                maxVersion = info;
            }

            if(info.startsWith(":permanent")){
                permanent = true;
            }
            message = message.section(" ",1);
        }

        if(!permanent){
            if(previous.isValid() && datetime.isValid()){
                if(datetime<=previous)
                    continue;
            }
        }

        if(!minVersion.isEmpty()){
            if(getVersionID(currentVersion)<getVersionID(minVersion))
                continue;
        }

        if(!maxVersion.isEmpty()){
            if(getVersionID(currentVersion)>getVersionID(maxVersion))
                continue;
        }

        if(!header.isEmpty()){
            message.prepend(header);
        }

        if(message.isEmpty())
            continue;

        QMessageBox mess;
        mess.setWindowTitle(tr("Message"));
        mess.setIcon(icon);
        mess.setText(message);
        mess.exec();
    }
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

    if( isNewer(versionOnServer,currentVersion) ){
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
    QNetworkReply *reply = manager->get(request);
    connect(reply,SIGNAL(finished()),this,SLOT(saveExec()));
    connect(reply,SIGNAL(downloadProgress(qint64,qint64)),this,SLOT(downloadProgress(qint64,qint64)));

    progress->setReply(reply);
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

    QString filename = QFileInfo(execUrl).fileName();

    //QFile file(filename);
    QFile file(QDir::tempPath() + QDir::separator() + filename);
    QFileInfo info = QFileInfo(file);

    if(!file.open(QFile::WriteOnly)){
        QMessageBox::critical(this,tr("Warning"),tr("Can't write file: %1").arg(info.absoluteFilePath()));
        return;
    }

    file.write(reply->readAll());

    file.close();

    //Start external update installer
    QMessageBox mess;
    mess.setText(tr("Install new version now?"));
    mess.addButton(QMessageBox::Yes);
    mess.addButton(QMessageBox::No);
    mess.setIcon(QMessageBox::Question);
    mess.setDetailedText(tr("%1Kb downloaded\nFile path: %2").arg( QString::number((int)(file.size()/1000)), info.absoluteFilePath() ));

    int button = mess.exec();
    if(button==QMessageBox::Yes){
        emit restart(info.absoluteFilePath());
    }else{
         QDesktopServices::openUrl(QUrl(QString("file:///%1").arg(info.absoluteDir().path()), QUrl::TolerantMode));
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
    QNetworkReply *reply = progress->getReply();
    if(reply)
        reply->abort();
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

bool UpdateManager::isNewer(QString v1, QString v2)
{
    if(!isValidVersion(v1))
        return true;

    if(!isValidVersion(v2))
        return false;

    QStringList n1 = v1.split(".");
    QStringList n2 = v2.split(".");

    for(int i=0;i<3;i++)
        if(n2.at(i).toInt() > n1.at(i).toInt())
            return true;
    return false;

}
