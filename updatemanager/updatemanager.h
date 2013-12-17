#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QCoreApplication>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>
#include <QDesktopServices>

#include <QFile>
#include <QFileInfo>
#include <QDir>

#include <QProcess>

#include <QMessageBox>

#include <QSettings>

#include <QDateTime>

#include "qprogressbardialog.h"

class UpdateManager : public QWidget
{
    Q_OBJECT
public:
    explicit UpdateManager(QWidget *parent = 0);
    void setVersion(QString version);

    inline void setMessageUrl(QString url) {messageUrl = url.trimmed();}
    inline void setVersionUrl(QString url) {versionUrl = url.trimmed();}
    inline void setExecUrl(QString url) {execUrl = url.trimmed();}

    static int getVersionID(QString version);
    static bool isNewer(QString v1, QString v2);
    static bool isValidVersion(QString);

private:
    QNetworkAccessManager *manager;
    QProgressBarDialog *progress;

    QString currentVersion;

    QString messageUrl;
    QString versionUrl;
    QString execUrl;

    bool discretUpdate;

signals:
    void status(QString);
    void restart(QString);

public slots:
    void getMessage(QString url = QString());
    void displayMessage();

    void startUpdate(QString url = QString());

    void checkVersion();

    void getExec(QString url = QString());
    void saveExec();

    void downloadProgress(qint64,qint64);
    void abort();

    void setDiscret(bool);
};

#endif // UPDATEMANAGER_H
