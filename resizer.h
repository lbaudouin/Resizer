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

#include <QSettings>

#include "qexifimageheader/qexifimageheader.h"

#include "positionselector.h"

#include <QThreadPool>
#include "loader.h"
#include "saver.h"

#include "mylabel.h"

namespace Ui {
class Resizer;
}

struct ImageInfo{
    QPixmap preview;
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

protected:
    void setList(QStringList paths);

    int readOrientation(QString filepath);

    void addList(QStringList);
    void addFile(QString);
    void removeFile(QString);

    void repaintGrid();

    void readSettings();
    void writeSettings();

    void setLogo(QString path);


public slots:
    void pressOpenFolder();
    void pressOpenFiles();

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

    void removeImage();

signals:
    void needToShow();

};

#endif // RESIZE_H
