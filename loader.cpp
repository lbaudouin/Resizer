#include "loader.h"

Loader::Loader()
{
    this->setAutoDelete(true);
}

Loader::~Loader()
{
}

void Loader::setFileInfo(QFileInfo info)
{
    info_ = info;
}

void Loader::setNeedRotation(bool rotation)
{
    needRotation_ = rotation;
}

void Loader::run()
{
    QImage small;
    QImageReader reader(info_.absoluteFilePath());
    QSize imageSize = reader.size();

    if(imageSize.isValid()){
        imageSize.scale(320,320,Qt::KeepAspectRatio);
        reader.setScaledSize(imageSize);
        small = reader.read();
    }else{
        small = QImage(info_.absoluteFilePath()).scaled(320,320,Qt::KeepAspectRatio);
    }

    QTransform transform;
    if(needRotation_){
        int orientation = QExifImageHeader(info_.absoluteFilePath()).value(QExifImageHeader::Orientation).toShort();

        switch(orientation){
        case 6: transform.rotate(90); break;
        case 3: transform.rotate(180); break;
        case 8: transform.rotate(270); break;
        default: needRotation_ = false;
        }
    }

    if(needRotation_){
        QImage rotated = small.transformed(transform);
        emit this->imageLoaded(info_.absoluteFilePath(),rotated);
    }else{
        emit this->imageLoaded(info_.absoluteFilePath(),small);
    }
}
