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
#include <QMovie>

#include <QSettings>
#include <QStatusBar>

#include "qexifimageheader/qexifimageheader.h"

#include "positionselector.h"

#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QThreadPool>

#include "imagelabel.h"

#include "plugininterface.h"
#include <QPluginLoader>
#include <QTimer>

namespace Ui {
class Resizer;
}

struct ImageLoaderInfo{
    QImage image;
    QString filename;
};

struct ImageSaverInfo{
    QString filename;
    QString outputFolder;
    bool noResize;
    RotationState rotation;
    bool keepExif;
    bool useRatio;
    double ratio;
    int sizeMax;

    bool addLogo;
    QImage logo;
    PositionSelector::POSITION logoPosition;
    QPoint logoShifting;
};

class Resizer : public QMainWindow
{
    Q_OBJECT
    
public:
    explicit Resizer(QWidget *parent = 0);
    ~Resizer();

    inline void setVersion(QString version) { m_version = version; }

private:
    Ui::Resizer *ui;

    QMap<QString,ImageLabel*> mapImages;
    QProgressDialog *m_resizingProgressDialog;
    QProgressDialog *m_loadingProgressDialog;
    QString m_logoPath;
    QString m_version;
    int nbColumns_;

    QMovie m_loadingMovie;

    QFutureWatcher<ImageLoaderInfo> *m_imageLoader;
    QFutureWatcher<bool> *m_imageSaver;

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent( QDragEnterEvent *event );
    void dragMoveEvent( QDragMoveEvent *event );
    void dragLeaveEvent( QDragLeaveEvent *event );

    void resizeEvent(QResizeEvent *);

    void repaintGrid();

    void readSettings();
    void writeSettings();

    void setLogo(QString path);

    void setToolButtonsEnabled(bool enabled);

    QStringList selected();

    static ImageLoaderInfo load(QString filename);
    static bool save(ImageSaverInfo info);

public slots:
    void pressOpenFolder();
    void pressOpenFiles();

    void addFilesAndFolders(QStringList);
    void addFiles(QStringList);
    void showImage(int);
    void loadFinished();

    void resizeAll();
    void resizeFinished();

    void handleMessage(const QString& message);

    void setRatioMode(bool);
    void setSizeMode(bool);

    void openLogo();

    void pressAbout();

    void restart(QString path);

    void displayLabelMenu(QPoint);

    void removeImage();
    void deleteImage();
    void detectRotation();
    void resetRotation();
    void rotateLeft();
    void rotateRight();

    void removeImage(QString);
    void deleteImage(QString);

    void removeImage(QStringList);
    void deleteImage(QStringList);
    void detectRotation(QStringList);
    void resetRotation(QStringList);
    void rotateLeft(QStringList);
    void rotateRight(QStringList);

    void selectAll();
    void deselectAll();
    void selectionChanged();

    void updateStatus();

    void progressChanged(int value);

signals:
    void needToShow();

    void finished();
    void updateProgressBar(int min, int max, int value);
    void updateNumber(int number);

};

#endif // RESIZE_H
