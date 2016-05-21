#include "positionselector.h"

PositionSelector::PositionSelector(QWidget *parent) :
    QWidget(parent), position_(BOTTOM_RIGHT), focus_(false)
{
    this->setMinimumSize(150,75);
    this->setMouseTracking(true);
}

QSize PositionSelector::sizeHint()
{
    return QSize(200,100);
}

QSize PositionSelector::minimumSizeHint()
{
    return QSize(200,100);
}

void PositionSelector::mousePressEvent(QMouseEvent *)
{
}

void PositionSelector::mouseReleaseEvent(QMouseEvent *)
{
    position_ = mousePosition_;
    repaint();
}

void PositionSelector::mouseMoveEvent(QMouseEvent *e)
{
    QList< QPair<double,POSITION> > dist;

    dist << QPair<double,POSITION>(QLineF(e->pos(),this->rect().topLeft()).length(),TOP_LEFT);
    dist << QPair<double,POSITION>(QLineF(e->pos(),this->rect().topRight()).length(),TOP_RIGHT);
    dist << QPair<double,POSITION>(QLineF(e->pos(),this->rect().bottomLeft()).length(),BOTTOM_LEFT);
    dist << QPair<double,POSITION>(QLineF(e->pos(),this->rect().bottomRight()).length(),BOTTOM_RIGHT);
    dist << QPair<double,POSITION>(QLineF(e->pos(),this->rect().center()).length(),CENTER);

    qSort(dist.begin(),dist.end());

    mousePosition_ = dist.at(0).second;

    repaint();
}

void PositionSelector::enterEvent(QEvent *)
{
    focus_ = true;
}

void PositionSelector::leaveEvent(QEvent *)
{
    focus_ = false;
    repaint();
}

void PositionSelector::paintEvent(QPaintEvent *)
{
    QPainter painter(this);
    painter.setRenderHint(QPainter::Antialiasing);

    painter.drawRect(this->rect());

    painter.drawRect(QRect(this->rect().topLeft(),QSize(10,10)));
    painter.drawRect(QRect(this->rect().topRight(),QSize(-10,10)));
    painter.drawRect(QRect(this->rect().bottomLeft(),QSize(10,-10)));
    painter.drawRect(QRect(this->rect().bottomRight(),QSize(-10,-10)));
    painter.drawRect(QRect(this->rect().center()-QPoint(5,5),QSize(10,10)));

    painter.setBrush(Qt::black);

    switch(position_){
    case TOP_LEFT: painter.drawRect(QRect(this->rect().topLeft(),QSize(10,10))); break;
    case TOP_RIGHT: painter.drawRect(QRect(this->rect().topRight(),QSize(-10,10))); break;
    case BOTTOM_LEFT: painter.drawRect(QRect(this->rect().bottomLeft(),QSize(10,-10))); break;
    case BOTTOM_RIGHT: painter.drawRect(QRect(this->rect().bottomRight(),QSize(-10,-10))); break;
    case CENTER: painter.drawRect(QRect(this->rect().center()-QPoint(5,5),QSize(10,10))); break;
    }

    if(focus_){
        painter.setBrush(Qt::red);
        switch(mousePosition_){
        case TOP_LEFT: painter.drawRect(QRect(this->rect().topLeft(),QSize(10,10))); break;
        case TOP_RIGHT: painter.drawRect(QRect(this->rect().topRight(),QSize(-10,10))); break;
        case BOTTOM_LEFT: painter.drawRect(QRect(this->rect().bottomLeft(),QSize(10,-10))); break;
        case BOTTOM_RIGHT: painter.drawRect(QRect(this->rect().bottomRight(),QSize(-10,-10))); break;
        case CENTER: painter.drawRect(QRect(this->rect().center()-QPoint(5,5),QSize(10,10))); break;
        }
    }
}
