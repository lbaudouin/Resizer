#ifndef RESIZE_H
#define RESIZE_H

#include <QMainWindow>
#include <QDebug>
#include <QImageReader>
#include <QImageWriter>
#include <QImageIOHandler>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QLabel>
#include <QGridLayout>
#include <QProgressDialog>
#include <QPainter>
#include <QMessageBox>
#include <QDesktopServices>
#include <QProcess>
#include <QResizeEvent>
#include <QScrollBar>

#include <QSettings>

#include "qexifimageheader/qexifimageheader.h"

#include "positionselector.h"

#include <QThreadPool>
#include "loader.h"
#include "saver.h"

#include "mylabel.h"

#include "plugininterface.h"
#include <QPluginLoader>

extern "C"{
    struct UnityLauncherEntry;
}

namespace Ui {
class Resizer;
}

struct ImageInfo{
    QFileInfo fileinfo;
    MyLabel *label;
};

class Resizer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Resizer(QWidget *parent = 0);
    ~Resizer();

    inline void setVersion(QString version) { version_ = version; }

private:
    Ui::Resizer *ui;

    QMap<QString,ImageInfo*> mapImages;

    QProgressDialog *diag_;

    QString logoPath;

    QString version_;

    int nbColumns_;

protected:
    void resizeEvent(QResizeEvent *);

    void setList(QStringList paths);

    int readOrientation(QString filepath);

    void addFile(QString);
    void removeFile(QString);

    void repaintGrid();

    void readSettings();
    void writeSettings();

    void setLogo(QString path);


public slots:
    void pressOpenFolder();
    void pressOpenFiles();

    void addList(QStringList);

    void resizeAll();

    void handleMessage(const QString& message);

    void setRatioMode(bool);
    void setSizeMode(bool);

    void openLogo();

    void pressAbout();

    void restart(QString path);

    void displayLabelMenu(QPoint);

    void imageLoaded(QString absoluteFilePath, QImage img);
    void resizeFinished(QString absoluteFilePath);

    void removeImage(QString absoluteFilePath = QString());
    void deleteImage();

signals:
    void needToShow();
    void addFiles(QStringList);

    void finished();
    void updateProgressBar(int min, int max, int value);
    void updateNumber(int number);

};

#endif // RESIZE_H
