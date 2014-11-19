#ifndef IMAGEPREVIEWMODEL_H
#define IMAGEPREVIEWMODEL_H

#include "ImagePreview.h"
#include <algorithm>
#include <QAbstractListModel>
#include <QSize>
#include <QMovie>

#include <QDebug>

#include "rotationstate.h"

class ImagePreviewModel : public QAbstractListModel
{
    Q_OBJECT
    Q_PROPERTY( QSize previewSize READ previewSize WRITE setPreviewSize NOTIFY previewSizeChanged ) 

public :
    ImagePreviewModel( const QSize & previewSize , QObject * parent = 0 );
    
    void removeImage(const int& id);
    void removeImage(const QModelIndex &id);
    bool addImage(const int &id, const QString &filepath);
    bool setImage(const int &id, const QImage &image, const RotationState &rotation = NO_ROTATION);

    bool rotate(const int &id, const RotationDirection &direction);
    bool setRotation(const int &id, const RotationState &rotation);

    int rowCount(const QModelIndex &parent = QModelIndex()) const ;
    QSize previewSize(int row) const ;
    QSize previewSize() const ;
    QVariant data(const int &id, int role = Qt::DisplayRole) const;
    QVariant data(const QModelIndex &index, int role = Qt::DisplayRole) const ;    
    ImagePreview * findImage(const int& id) const ;
    ImagePreview * at(const QModelIndex & index) const ;

    bool contains(const QString &filepath) const;

    QList<ImageInfo> getImageList() const;
    QString getImageFilepath(const int& id);
    RotationState getImageRotation(const int& id);
    
private:
    QList<ImagePreview*> m_buffer;
    QList<int> m_bufferIDs;

    QSize m_previewSize ;
    QMovie *m_movie;
    QImage m_loadImage;

public slots :
    void setPreviewSize( const QSize & size ) ;
    
private :
    bool checkImageDimension( const QImage & image ) const ;
    
signals :
    void previewSizeChanged( QSize ) ;
    void imageLoaded( const QModelIndex & index , bool loaded ) ;
} ;

#endif // IMAGEPREVIEWMODEL_H
