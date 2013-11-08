#ifndef LOADER_H
#define LOADER_H

#include <QRunnable>
#include <QThreadPool>

#include <QImage>
#include <QImageReader>

#include <QLabel>
#include <QFileInfo>

#include "qexifimageheader/qexifimageheader.h"

class Loader : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Loader();
    ~Loader();
    void run();

    void setFileInfo(QFileInfo info);
    void setLabel(QLabel *label);
    void setNeedRotation(bool rotation);

private:
    QFileInfo info_;
    bool needRotation_;

signals:
    void imageLoaded(QString absoluteFilePath, QImage img);
    
};

#endif // LOADER_H
