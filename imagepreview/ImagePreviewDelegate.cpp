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

#include "ImagePreviewDelegate.h"
#include <QPainter>
#include <QPixmap>
#include <iostream>

/*!
 * Constructor
 */
ImagePreviewDelegate::ImagePreviewDelegate( QObject* parent ) :
    QAbstractItemDelegate(parent) ,
    m_borderSize(5)
{
}

/*!
 *
 */
void ImagePreviewDelegate::paint( QPainter* painter, const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    if(!painter && !index.isValid())
        return;

    QRect renderArea = option.rect.adjusted( borderSize() , borderSize() , -borderSize() , -borderSize() );

    if(option.state & QStyle::State_Selected){
      painter->setBrush( createBrush(180) );
      painter->setPen( Qt::NoPen );
      painter->drawRoundedRect( option.rect , borderSize() , borderSize() );
    }

    // Preview
    if(index.data(Qt::DecorationRole).canConvert(QVariant::Pixmap)){
        QPixmap pixmap = index.data(Qt::DecorationRole).value<QPixmap>();
        if(!pixmap.isNull()){
            int max = qMax(renderArea.size().height(),renderArea.size().width());
            QSize size = pixmap.size();
            size.scale(max,max,Qt::KeepAspectRatio);
            QRect rect(renderArea.topLeft()+QPoint((renderArea.width()-size.width())/2.0,(renderArea.height()-size.height())/2.0),size);
            painter->drawPixmap( rect , pixmap );
        }
    }
}

/*!
 *
 */
QSize ImagePreviewDelegate::sizeHint(const QStyleOptionViewItem& /*option*/, const QModelIndex& index) const
{
    if(!index.isValid())
        return QSize();

    int row = index.row();
    ImagePreviewModel *model = (ImagePreviewModel*)index.model();

    QSize previewSize = model->previewSize(row);
    previewSize.rheight() += 2 * borderSize();
    previewSize.rwidth() += 2 * borderSize();

    return previewSize;
}

/*!
 *
 */
void ImagePreviewDelegate::setBorderSize(const int& size)
{
    m_borderSize = size;
}

/*!
 *
 */
int ImagePreviewDelegate::borderSize() const
{
    return m_borderSize;
}

/*!
 * 
 */
QBrush ImagePreviewDelegate::createBrush( const int & alpha ) const
{
    QPalette palette;
    QBrush b = palette.highlight();
    QColor col = b.color();
    col.setAlpha(alpha);
    b.setColor(col);
    return b;
}
