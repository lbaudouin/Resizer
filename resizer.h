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
#include <QDropEvent>
#include <QScrollBar>
#include <QUrl>

#include <QSettings>

#include "imagepreview/ImagePreviewModel.h"
#include "imagepreview/ImagePreviewDelegate.h"

#include "qexifimageheader/qexifimageheader.h"

#include "positionselector.h"

#include <QThreadPool>
#include "loader.h"
#include "saver.h"

#include "mylabel.h"

#include "plugininterface.h"
#include <QPluginLoader>
#include <QTimer>


namespace Ui {
class Resizer;
}

class Resizer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Resizer(QWidget *parent = 0);
    ~Resizer();

    inline void setVersion(QString version) { version_ = version; }

private:
    Ui::Resizer *ui;

    QProgressDialog *diag_;
    QString logoPath;
    QString version_;
    int nbColumns_;

    ImagePreviewModel *m_model;
    ImagePreviewDelegate *m_delegate;

    int m_idCount;

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent( QDragEnterEvent *event );
    void dragMoveEvent( QDragMoveEvent *event );
    void dragLeaveEvent( QDragLeaveEvent *event );

    void addFile(QString);

    void readSettings();
    void writeSettings();

    void setLogo(QString path);

    void setToolButtonsEnabled(bool enabled);
    QList<int> selectedIDs();


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

    void imageLoaded(int id, ImageData imageData);
    void resizeFinished(int id);

    void removeImage();
    void deleteImage();
    void detectRotation();
    void resetRotation();
    void rotateLeft();
    void rotateRight();

    void removeImage(QList<int>);
    void deleteImage(QList<int>);
    void detectRotation(QList<int>);
    void resetRotation(QList<int>);
    void rotateLeft(QList<int>);
    void rotateRight(QList<int>);

    void selectionChanged();

signals:
    void needToShow();
    //void addFiles(QStringList);

    void finished();
    void updateProgressBar(int min, int max, int value);
    void updateNumber(int number);

};

#endif // RESIZE_H
