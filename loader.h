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
    int id;
    QImage image;
    RotationState rotation;
};

inline ImageData loadImage(const QString &filename)
{
    QImage small;
    QImageReader reader(filename);
    QSize imageSize = reader.size();

    if(imageSize.isValid()){
        imageSize.scale(320,320,Qt::KeepAspectRatio);
        reader.setScaledSize(imageSize);
        small = reader.read();
    }else{
        small = QImage(filename).scaled(320,320,Qt::KeepAspectRatio);
    }

    ImageData imageData;
    imageData.image = small;

    int orientation = QExifImageHeader(filename).value(QExifImageHeader::Orientation).toShort();

    switch(orientation){
        case 6: imageData.rotation = CLOCKWISE; break;
        case 3: imageData.rotation = REVERSE; break;
        case 8: imageData.rotation = COUNTERCLOCKWISE; break;
        default: imageData.rotation = NO_ROTATION;
    }

    return imageData;
}

class Loader : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Loader(QObject *parent = 0);
    ~Loader();
    void run();

    void setImageID(int id);
    void setFileInfo(QFileInfo info);
    void setNeedRotation(bool needRotation = true);

private:
    int m_id;
    QFileInfo info_;
    bool needRotation_;

signals:
    void imageLoaded(int id, QImage img);
    void imageLoaded(int id, ImageData imgData);
    
};

#endif // LOADER_H
