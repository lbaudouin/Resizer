#include "saver.h"

Saver::Saver(QObject *parent) :
    QObject(parent)
{
    this->setAutoDelete(true);
}

void Saver::setFileInfo(QFileInfo info)
{
    info_ = info;
}

void Saver::setOutputSubfolder(QString subfolder)
{
    subfolder_ = subfolder;
}

void Saver::setRotation(RotationState rotation)
{
    rotation_ = rotation;
}

void Saver::setNoResize(bool noResize)
{
    noResize_ = noResize;
}

void Saver::setUseRatio(bool useRatio)
{
    useRatio_ = useRatio;
}

void Saver::setAddLogo(bool addLogo)
{
    addLogo_ = addLogo;
}

void Saver::setLogo(QImage logo)
{
    logo_ = logo;
}

void Saver::setSizeMax(int sizeMax)
{
    sizeMax_ = sizeMax;
}

void Saver::setRatio(double ratio)
{
    ratio_ = ratio;
}

void Saver::setLogoPosition(PositionSelector::POSITION position, int xShift, int yShift)
{
    position_ = position;
    xShift_ = xShift;
    yShift_ = yShift;
}

void Saver::run()
{

    /*QExifImageHeader exif(img.fileinfo.absoluteFilePath());

    QList<QExifImageHeader::ImageTag> list1 = exif.imageTags();
    QList<QExifImageHeader::ExifExtendedTag> list2 = exif.extendedTags();
    QList<QExifImageHeader::GpsTag> list3 = exif.gpsTags();

    for(int i=0;i<list1.size();i++){
        qDebug() << exif.value(list1[i]).toString();
    }
    for(int i=0;i<list2.size();i++){
        qDebug() << exif.value(list2[i]).toString();
    }
    for(int i=0;i<list3.size();i++){
        qDebug() << exif.value(list3[i]).toString();
    }*/

    QString output = info_.absoluteDir().absolutePath() + QDir::separator() + subfolder_ + QDir::separator() + info_.fileName();

    QDir dir(info_.absoluteDir());
    if(!dir.exists() || !dir.mkpath(subfolder_)){
        return;
    }

    QImage small;
    QImageReader reader(info_.absoluteFilePath());
    QSize imageSize = reader.size();

    if(noResize_){
        small.load(info_.absoluteFilePath());
    }else{
        if(imageSize.isValid()){
            if(useRatio_){
                imageSize *= ratio_;
            }else{
                imageSize.scale(sizeMax_,sizeMax_,Qt::KeepAspectRatio);
            }
            reader.setScaledSize(imageSize);
            small = reader.read();
        }else{
            QImage original(info_.absoluteFilePath());
            imageSize = original.size();
            if(useRatio_){
                imageSize *= ratio_;
            }else{
                imageSize.scale(sizeMax_,sizeMax_,Qt::KeepAspectRatio);
            }
            small = original.scaled(imageSize,Qt::KeepAspectRatio);
        }
    }

    if(rotation_!=NO_ROTATION){
        QTransform transform;
        switch(rotation_){
        case CLOCKWISE: transform.rotate(90); break;
        case REVERSE: transform.rotate(180); break;
        case COUNTERCLOCKWISE: transform.rotate(270); break;
        default: transform.reset();
        }
        small = small.transformed(transform);
    }

    if(addLogo_ && !logo_.isNull()){
        switch(position_){
        case PositionSelector::TOP_LEFT:
            break;
        case PositionSelector::TOP_RIGHT:
            xShift_ = small.width() - logo_.width() - xShift_;
            break;
        case PositionSelector::BOTTOM_LEFT:
            yShift_ = small.height() - logo_.height() - yShift_;
            break;
        case PositionSelector::BOTTOM_RIGHT:
            xShift_ = small.width() - logo_.width() - xShift_;
            yShift_ = small.height() - logo_.height() - yShift_;
            break;
        case PositionSelector::CENTER:
            xShift_ = small.width()/2.0 - logo_.width()/2.0 + xShift_;
            yShift_ = small.height()/2.0 - logo_.height()/2.0 + yShift_;
            break;
        default: break;
        }

        QPainter painter(&small);
        painter.drawImage(xShift_,yShift_,logo_);
        painter.end();
    }

    small.save(output);

    //exif.setValue(QExifImageHeader::Orientation,0);
    //exif.setValue(QExifImageHeader::ImageWidth,small.width());
    //exif.setValue(QExifImageHeader::ImageLength,small.height());

    emit this->resizeFinished(m_id);


}
