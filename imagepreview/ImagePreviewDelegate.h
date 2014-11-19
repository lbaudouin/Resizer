#ifndef IMAGEPREVIEWDELEGATE_H
#define IMAGEPREVIEWDELEGATE_H

#include <QAbstractItemDelegate>
#include "ImagePreviewModel.h"
#include <QDebug>

class QPainter ;

class ImagePreviewDelegate : public QAbstractItemDelegate
{
   Q_OBJECT 
   Q_PROPERTY( int borderSize READ borderSize WRITE setBorderSize )
   
   int m_borderSize ;
   
public :
    explicit ImagePreviewDelegate( QObject * parent = 0);
    void paint( QPainter * painter, const QStyleOptionViewItem &option, const QModelIndex &index ) const;
    QSize sizeHint( const QStyleOptionViewItem &option , const QModelIndex &index ) const;
    int borderSize() const;
    
public slots :
    void setBorderSize( const int & size );
    
private :
    QBrush createBrush( const int & alpha = 255 ) const;
    
} ;

#endif // IMAGEPREVIEWDELEGATE_H
