#ifndef RESIZE_H
#define RESIZE_H

#include <QMainWindow>
#include <QDebug>
#include <QImageWriter>
#include <QImageIOHandler>
#include <QFileDialog>
#include <QFileIconProvider>
#include <QLabel>
#include <QGridLayout>
#include <QProgressDialog>
#include <QPainter>
#include <QMessageBox>

#include "qexifimageheader/qexifimageheader.h"

#define VERSION "0.0.5"

namespace Ui {
class Resizer;
}

struct Image{
    QImage original;
    QPixmap preview;
    QString folder;
    QString filename;
    QFileInfo fileinfo;
};

class Resizer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Resizer(QWidget *parent = 0);
    ~Resizer();
    
private:
    Ui::Resizer *ui;
    QStringList files;

    QMap<QString,Image> mapImages;

    QList<Image> listImages;

    QString logoPath;

    //QGridLayout *gridPreview;

protected:
    void setList(QStringList paths);

    int readOrientation(QString filepath);

    void addList(QStringList);
    void addFile(QString);
    void removeFile(QString);

    void repaintGrid();


public slots:
    void pressOpenFolder();
    void pressOpenFiles();

    void resizeAll();

    void handleMessage(const QString& message);

    void setRatioMode(bool);
    void setSizeMode(bool);

    void openLogo();

    void pressAbout();

signals:
    void needToShow();

};

#endif // RESIZE_H
