#ifndef LOADER_H
#define LOADER_H

#include <QRunnable>

#include <QImage>
#include <QImageReader>

#include <QLabel>
#include <QFileInfo>

#include <QDebug>

#include "qexifimageheader/qexifimageheader.h"

#include "rotationstate.h"

struct ImageData{
    QImage image;
    RotationState rotation;
};

class Loader : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Loader(QObject *parent = 0);
    ~Loader();
    void run();

    void setFileInfo(QFileInfo info);
    void setNeedRotation(bool needRotation = true);

private:
    QFileInfo info_;
    bool needRotation_;

signals:
    void imageLoaded(QString absoluteFilePath, QImage img);
    void imageLoaded(QString absoluteFilePath, ImageData imgData);
    
};

#endif // LOADER_H
