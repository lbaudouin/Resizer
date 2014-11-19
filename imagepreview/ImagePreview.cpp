/*
 * Copyright (C) 2014  Clément Fouque <clement.fouque@univ-bpclermont.fr>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 *
 */

#include "ImagePreview.h"

ImagePreview::ImagePreview(const int &id, const QString &filepath, const QImage& image, const QSize& previewSize, QObject* parent) :
    QObject(parent), m_id(id), m_rotation(INVALID_ROTATION), m_filepath(filepath)
{
    m_pixmap = QPixmap::fromImage(image);
    setPreviewSize(previewSize);
}


ImagePreview::ImagePreview(const int &id, const QString &filepath, const QPixmap& pixmap, const QSize& previewSize, QObject* parent) :
    QObject(parent), m_id(id), m_pixmap(pixmap), m_rotation(INVALID_ROTATION), m_filepath(filepath)
{
    setPreviewSize(previewSize);
}

QVariant ImagePreview::data(int role) const
{
    switch(role){
    case Qt::DisplayRole:  return m_pixmap;//.id(); break;
    case Qt::DecorationRole:  return m_previewImage; break;
    case Qt::TextAlignmentRole: return Qt::AlignCenter; break;
    case Qt::UserRole:  return m_id; break; //ID
    case Qt::UserRole+1:  return m_filepath; break; //Filepath
    case Qt::UserRole+2:  return m_rotation; break; //RotationState
    case Qt::UserRole+3:  return m_previewSize; break; //Preview size
    default: return QVariant();
    }
}

QImage ImagePreview::image() const
{
    return m_pixmap.toImage() ;
}

QPixmap ImagePreview::pixmap() const
{
    return m_pixmap ;
}

void ImagePreview::setImage(const QImage &image, const RotationState &rotation)
{
    setImage(QPixmap::fromImage(image),rotation);
}

void ImagePreview::setImage(const QPixmap &pixmap, const RotationState &rotation)
{
    m_rotation = rotation;

    if(qMax(pixmap.width(),pixmap.height())>1024){
        m_pixmap = pixmap.scaled(1024,1024,Qt::KeepAspectRatio);
    }else{
        m_pixmap = pixmap;
    }

    setPreviewSize(m_previewImage.size());

    generatePreview();
}

void ImagePreview::generatePreview()
{
    QTransform transform;
    switch(m_rotation){
    case NO_ROTATION: break;
    case CLOCKWISE: transform.rotate(90); break;
    case REVERSE: transform.rotate(180); break;
    case COUNTERCLOCKWISE: transform.rotate(-90); break;
    }

    m_previewImage = m_pixmap.scaled(m_previewSize,Qt::KeepAspectRatio).transformed(transform);
}

QSize ImagePreview::previewSize() const
{
    QSize size = m_previewImage.size();
    return size;
}

void ImagePreview::setPreviewSize(const QSize& size)
{
    if(size.isValid()){
        m_previewSize = size;
        generatePreview();
        emit previewSizeChanged( m_previewSize );
    } 
}

void ImagePreview::rotate(const RotationDirection &direction)
{
    QTransform transform;
    switch(direction) {
    case ROTATION_LEFT:
        transform.rotate(-90);
        switch(m_rotation){
        case NO_ROTATION: m_rotation = COUNTERCLOCKWISE; break;
        case CLOCKWISE: m_rotation = NO_ROTATION; break;
        case REVERSE: m_rotation = CLOCKWISE; break;
        case COUNTERCLOCKWISE: m_rotation = REVERSE; break;
        }

        break;
    case ROTATION_RIGHT:
        transform.rotate(90);
        switch(m_rotation){
        case NO_ROTATION: m_rotation = CLOCKWISE; break;
        case CLOCKWISE: m_rotation = REVERSE; break;
        case REVERSE: m_rotation = COUNTERCLOCKWISE; break;
        case COUNTERCLOCKWISE: m_rotation = NO_ROTATION; break;
        }
        break;
    }

    m_previewImage = m_previewImage.transformed(transform);
}

void ImagePreview::setRotation(const RotationState &rotation)
{
    QTransform transform;
    //Reset rotation
    switch(m_rotation){
    case NO_ROTATION: break;
    case CLOCKWISE: transform.rotate(-90); break;
    case REVERSE: transform.rotate(180); break;
    case COUNTERCLOCKWISE: transform.rotate(90); break;
    }
    m_previewImage = m_previewImage.transformed(transform);
    transform.reset();
    //Apply new transformation
    switch(rotation){
    case NO_ROTATION: break;
    case CLOCKWISE: transform.rotate(90); break;
    case REVERSE: transform.rotate(180); break;
    case COUNTERCLOCKWISE: transform.rotate(-90); break;
    }
    m_previewImage = m_previewImage.transformed(transform);
    m_rotation = rotation;
}
