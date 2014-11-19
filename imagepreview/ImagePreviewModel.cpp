#include "ImagePreviewModel.h"

/*!
 * 
 */
ImagePreviewModel::ImagePreviewModel(const QSize& previewSize, QObject* parent) :
    QAbstractListModel(parent) ,
    m_previewSize(previewSize)
{
    m_loadImage.load(":images/loading");

    //TODO : loading
    /*if(image.isNull()){
        if(!m_movie ){
            m_movie = new QMovie(":image/loading");
            m_movie->start();
            loadImage = m_movie->currentImage();
        }
    }else{
        if(!m_movie ){
            m_movie->stop();
        }
    }*/
}

/*!
 * Returns the number of previews.
 */
int ImagePreviewModel::rowCount(const QModelIndex& /*parent*/) const
{
    return m_buffer.size() ;
}

/*!
 * 
 */
QVariant ImagePreviewModel::data(const QModelIndex& index, int role) const
{
    if(!index.isValid())
        return QVariant() ;
    
    ImagePreview * item = m_buffer.at(index.row()) ;
    return item->data(role) ;
}

/*!
 *
 */
QVariant ImagePreviewModel::data(const int& id, int role) const
{
    int row = m_bufferIDs.indexOf(id);
    if(row<0)
        return QVariant() ;

    return m_buffer.at(row)->data(role) ;
}

/*!
 *
 */
QSize ImagePreviewModel::previewSize(int row) const
{

    if(row<0 || row>=m_buffer.size())
        return m_previewSize;
    return m_buffer.at(row)->previewSize();
}

/*!
 * 
 */
QSize ImagePreviewModel::previewSize() const
{
    return m_previewSize ;
}

/*!
 * 
 */
void ImagePreviewModel::setPreviewSize(const QSize& size)
{
    if(size.isValid()){
        foreach(ImagePreview *preview ,m_buffer){
            preview->setPreviewSize(size);
        }
        
        if ( m_buffer.isEmpty() )
            m_previewSize = size ;
        else 
            m_previewSize = m_buffer.first()->previewSize() ;

        emit previewSizeChanged( m_previewSize );
        emit layoutChanged();
    }
}

/*!
 * 
 */
bool ImagePreviewModel::addImage(const int &id, const QString& filepath)
{
    int row = rowCount();
    beginInsertRows( QModelIndex() , row , row );
    ImagePreview * litem = new ImagePreview( id, filepath, m_loadImage , m_previewSize , this ) ;
    m_buffer.append(litem);
    m_bufferIDs.append(id);
    endInsertRows();
    //QModelIndex index = createIndex( row , 0 ) ;
    //emit dataChanged(index,index) ;
    emit layoutChanged();
    return true ;
}

/*!
 *
 */
bool ImagePreviewModel::setImage(const int &id, const QImage &image, const RotationState &rotation)
{
    if(m_bufferIDs.contains(id)){
        int row = m_bufferIDs.indexOf(id);
        m_buffer.at(row)->setImage(image,rotation);
        //QModelIndex index = createIndex( row , 0 ) ;
        //emit dataChanged(index,index) ;
        emit layoutChanged();
        return true;
    }
    return false;
}

/*!
 *
 */
bool ImagePreviewModel::rotate(const int &id, const RotationDirection &direction)
{
    if(m_bufferIDs.contains(id)){
        int row = m_bufferIDs.indexOf(id);
        m_buffer.at(row)->rotate(direction);

        //QModelIndex index = createIndex(row,0) ;
        //emit dataChanged(index,index) ;
        emit layoutChanged();
        return true;
    }
    return false;
}

/*!
 *
 */
bool ImagePreviewModel::setRotation(const int &id, const RotationState &rotation)
{
    if(m_bufferIDs.contains(id)){
        int row = m_bufferIDs.indexOf(id);
        m_buffer.at(row)->setRotation(rotation);

        //QModelIndex index = createIndex(row,0) ;
        //emit dataChanged(index,index) ;
        emit layoutChanged();
        return true;
    }
    return false;
}

/*!
 * 
 */
void ImagePreviewModel::removeImage(const int& id)
{
    if(m_bufferIDs.contains(id)){
        int row = m_bufferIDs.indexOf(id);
        beginRemoveRows( QModelIndex() , row , row );
        m_buffer.removeAt(row);
        m_bufferIDs.removeAt(row);
        endRemoveRows();
        emit layoutChanged();
    }
}

/*!
 * 
 */
void ImagePreviewModel::removeImage( const QModelIndex & index )
{
    beginRemoveRows( QModelIndex(), index.row() , index.row() );
    m_buffer.removeAt( index.row() );
    emit dataChanged( index, index );
    endRemoveRows();
    //emit layoutChanged();
}

/*!
 *
 */
bool ImagePreviewModel::contains( const QString &filepath ) const
{
    foreach(ImagePreview *ip, m_buffer) {
        if(ip->data(Qt::UserRole+1)==filepath)
            return true;
    }
    return false;
}

/*!
 * 
 */
ImagePreview* ImagePreviewModel::findImage(const int& id) const
{
    int row = m_bufferIDs.indexOf(id);
    if(row<0)
        return NULL;
    else
        return m_buffer.at(row);
}

/*!
 * 
 */
bool ImagePreviewModel::checkImageDimension(const QImage& image) const
{
    if(m_buffer.isEmpty()) return true;

    QSize prevSize = m_buffer.first()->image().size();
    return( prevSize == image.size() );
}

/*!
 * 
 */
ImagePreview* ImagePreviewModel::at(const QModelIndex& index) const
{
    if(!index.isValid()) return 0 ;
    Q_ASSERT( index.row() < m_buffer.size() );
    
    return m_buffer.at( index.row() );
}

/*!
 *
 */
QList<ImageInfo> ImagePreviewModel::getImageList() const
{
    QList<ImageInfo> list;

    foreach(ImagePreview* ip, m_buffer){
        ImageInfo info;
        info.fileinfo = ip->data(Qt::UserRole+1).toString();
        info.rotation = ip->data(Qt::UserRole+2).value<RotationState>();
        list << info;
    }

    return list;
}

QString ImagePreviewModel::getImageFilepath(const int& id)
{
    int row = m_bufferIDs.indexOf(id);
    if(row<0)
        return "";
    return data(id,Qt::UserRole+1).toString();
}

RotationState ImagePreviewModel::getImageRotation(const int& id)
{
    int row = m_bufferIDs.indexOf(id);
    if(row<0)
        return INVALID_ROTATION;
    return data(id,Qt::UserRole+2).value<RotationState>();
}
