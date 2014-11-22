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

#include "qexifimageheader/qexifimageheader.h"

#include "positionselector.h"

#include <QtConcurrentMap>
#include <QFutureWatcher>
#include <QThreadPool>
#include "loader.h"

#include "imagelabel.h"

#include "plugininterface.h"
#include <QPluginLoader>
#include <QTimer>

namespace Ui {
class Resizer;
}

struct ImageInfo{
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
    QProgressDialog *diag_;
    QProgressDialog *m_resizeProgressDialog;
    QString m_logoPath;
    QString m_version;
    int nbColumns_;

    QMovie m_loadingMovie;

protected:
    void dropEvent(QDropEvent *event);
    void dragEnterEvent( QDragEnterEvent *event );
    void dragMoveEvent( QDragMoveEvent *event );
    void dragLeaveEvent( QDragLeaveEvent *event );

    void resizeEvent(QResizeEvent *);

    void addFile(QString);
    void removeFile(QString);

    void repaintGrid();

    void readSettings();
    void writeSettings();

    void setLogo(QString path);

    void setToolButtonsEnabled(bool enabled);

    QStringList selected();

    QFutureWatcher<bool> *imageSaver;
    static bool save(ImageInfo info);

public slots:
    void pressOpenFolder();
    void pressOpenFiles();

    void addList(QStringList);

    void resizeAll();
    void resizeFinished();

    void handleMessage(const QString& message);

    void setRatioMode(bool);
    void setSizeMode(bool);

    void openLogo();

    void pressAbout();

    void restart(QString path);

    void displayLabelMenu(QPoint);

    void imageLoaded(QString absoluteFilePath, ImageData imageData);

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

signals:
    void needToShow();
    //void addFiles(QStringList);

    void finished();
    void updateProgressBar(int min, int max, int value);
    void updateNumber(int number);

};

#endif // RESIZE_H
