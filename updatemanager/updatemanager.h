#ifndef UPDATEMANAGER_H
#define UPDATEMANAGER_H

#include <QCoreApplication>
#include <QWidget>
#include <QNetworkAccessManager>
#include <QNetworkRequest>
#include <QNetworkReply>

#if QT_VERSION < QT_VERSION_CHECK(5, 0, 0)
#include <QDesktopServices>
#else
#include <QStandardPaths>
#endif
#include <QDir>

#include <QFile>
#include <QFileInfo>

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
    //inline void setZipUrl(QString url) {zipUrl = url.trimmed();}
    //inline void setLzmaUrl(QString url) {lzmaUrl = url.trimmed();}

    static int getVersionID(QString version);

protected:
    bool isValidVersion(QString);

private:
    QNetworkAccessManager *manager;
    QProgressBarDialog *progress;
    QNetworkReply *execReply;

    QString currentVersion;

    QString messageUrl;
    QString versionUrl;
    QString execUrl;
    //QString zipUrl;
    //QString lzmaUrl;

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
