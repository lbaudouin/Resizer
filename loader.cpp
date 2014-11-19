#include "loader.h"

Loader::Loader(QObject *parent) :
               QObject(parent)
{
    this->setAutoDelete(true);
}

Loader::~Loader()
{
}

void Loader::setImageID(int id)
{
    m_id = id;
}

void Loader::setFileInfo(QFileInfo info)
{
    info_ = info;
}

void Loader::setNeedRotation(bool needRotation)
{
    needRotation_ = needRotation;
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

    ImageData imageData;
    imageData.image = small;

    if(needRotation_){
        int orientation = QExifImageHeader(info_.absoluteFilePath()).value(QExifImageHeader::Orientation).toShort();

        switch(orientation){
        case 6: imageData.rotation = CLOCKWISE; break;
        case 3: imageData.rotation = REVERSE; break;
        case 8: imageData.rotation = COUNTERCLOCKWISE; break;
        default: imageData.rotation = NO_ROTATION;
        }
    }else{
        imageData.rotation = NO_ROTATION;
    }

    emit this->imageLoaded(m_id,imageData);

}
