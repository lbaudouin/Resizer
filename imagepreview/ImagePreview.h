#ifndef IMAGEPREVIEW_H
#define IMAGEPREVIEW_H

#include <QTableWidgetItem>
#include <QStandardItemModel>
#include <QFileInfo>

#include "rotationstate.h"

class ImagePreview : public QObject
{
    Q_OBJECT
    Q_PROPERTY( QSize previewSize READ previewSize WRITE setPreviewSize NOTIFY previewSizeChanged ) 

    
public :
    explicit ImagePreview(const int &id, const QString &filepath, const QImage &image, const QSize& previewSize, QObject * parent = 0 );
    explicit ImagePreview(const int &id, const QString &filepath, const QPixmap &pixmap, const QSize& previewSize, QObject * parent = 0 );
    
    QVariant data( int role ) const;
    QImage image() const;
    QPixmap pixmap() const;

    void rotate(const RotationDirection &direction);
    void setRotation(const RotationState &rotation);

    QSize previewSize() const;

    void generatePreview();

private:
    int m_id;
    QSize m_previewSize;
    QPixmap m_previewImage ;
    QPixmap m_pixmap ;
    RotationState m_rotation;
    QString m_filepath;
        
public slots :
    void setPreviewSize(const QSize &size);
    void setImage(const QImage &image, const RotationState &rotation = NO_ROTATION);
    void setImage(const QPixmap &pixmap, const RotationState &rotation = NO_ROTATION);

signals :
    void loaded( bool ) ;
    void previewSizeChanged( QSize ) ;
    
} ;

#endif // IMAGEPREVIEW_H
