#include <mylabel.h>

MyLabel::MyLabel(QWidget *parent) : QLabel(parent), absoluteFilePath_(""), displayIcons_(false), checked_(false)
{
    this->setMouseTracking(true);
}

MyLabel::MyLabel(QString absoluteFilePath, QWidget *parent) : QLabel(parent), absoluteFilePath_(absoluteFilePath), displayIcons_(false), checked_(false)
{
    this->setMouseTracking(true);
}

void MyLabel::setAbsoluteFilePath(QString absoluteFilePath)
{
    absoluteFilePath_ = absoluteFilePath;
}

QString MyLabel::getAbsoluteFilePath()
{
    return absoluteFilePath_ ;
}

void MyLabel::setChecked(bool checked)
{
    checked_ = checked;
    update();
}

void MyLabel::setUnChecked(bool checked)
{
    checked_ = !checked;
    update();
}

bool MyLabel::isChecked()
{
    return checked_;
}

void MyLabel::mousePressEvent(QMouseEvent *e)
{
    if(autoRect_.contains(e->pos()) || resetRect_.contains(e->pos()) || leftRect_.contains(e->pos()) || rightRect_.contains(e->pos()) ||
       removeRect_.contains(e->pos()) || deleteRect_.contains(e->pos()) )
        return;

    if(e->button()==Qt::LeftButton){
        checked_ = !checked_;
        update();
    }
    if(e->button()==Qt::RightButton){
        checked_ = true;
        update();
    }
    if(e->button()==Qt::MiddleButton){
        checked_ = false;
        update();
    }
}

void MyLabel::mouseReleaseEvent(QMouseEvent *e)
{

    if(autoRect_.contains(e->pos())){
        emit this->autoPressed(absoluteFilePath_);
    }
    if(resetRect_.contains(e->pos())){
        emit this->resetPressed(absoluteFilePath_);
    }
    if(leftRect_.contains(e->pos())){
        emit this->leftPressed(absoluteFilePath_);
    }
    if(rightRect_.contains(e->pos())){
        emit this->rightPressed(absoluteFilePath_);
    }

    if(removeRect_.contains(e->pos())){
        emit this->removePressed(absoluteFilePath_);
    }
    if(deleteRect_.contains(e->pos())){
        emit this->deletePressed(absoluteFilePath_);
    }
}

void MyLabel::mouseMoveEvent(QMouseEvent *e)
{
    if(autoRect_.contains(e->pos()) || resetRect_.contains(e->pos()) || leftRect_.contains(e->pos()) || rightRect_.contains(e->pos()) ||
       removeRect_.contains(e->pos()) || deleteRect_.contains(e->pos()) ){
        this->setCursor(Qt::PointingHandCursor);
    }else{
        this->setCursor(Qt::ArrowCursor);
    }
}

void MyLabel::enterEvent(QEvent *)
{
    displayIcons_ = true;
    update();
}

void MyLabel::leaveEvent(QEvent *)
{
    displayIcons_ = false;
    update();
    this->setCursor(Qt::ArrowCursor);
}

void MyLabel::paintEvent(QPaintEvent *e){
    QLabel::paintEvent(e);
    QPainter painter(this);

    if(this->size().height()>202){
        autoRect_ = QRect(0,0,32,32);
        resetRect_ = QRect(0,32,32,32);
        leftRect_ = QRect(0,64,32,32);
        rightRect_ = QRect(0,96,32,32);
        removeRect_ = QRect(0,138,32,32);
        deleteRect_ = QRect(0,170,32,32);
    }else{
        autoRect_ = QRect(0,0,32,32);
        resetRect_ = QRect(32,0,32,32);
        leftRect_ = QRect(64,0,32,32);
        rightRect_ = QRect(96,0,32,32);
        removeRect_ = QRect(138,0,32,32);
        deleteRect_ = QRect(170,0,32,32);
    }

    int maxCheckSize = std::min(64,std::min(this->width(),this->height()));
    checkRect_ = QRect(this->width()-maxCheckSize,this->height()-maxCheckSize,maxCheckSize,maxCheckSize);


    if(displayIcons_){
            painter.drawPixmap(autoRect_.topLeft(),QPixmap(":images/auto").scaled(32,32));
            painter.drawPixmap(resetRect_.topLeft(),QPixmap(":images/cancel").scaled(32,32));
            painter.drawPixmap(leftRect_.topLeft(),QPixmap(":images/left").scaled(32,32));
            painter.drawPixmap(rightRect_.topLeft(),QPixmap(":images/right").scaled(32,32));
            painter.drawPixmap(removeRect_.topLeft(),QPixmap(":images/remove").scaled(32,32));
            painter.drawPixmap(deleteRect_.topLeft(),QPixmap(":images/delete").scaled(32,32));
    }

    if(checked_){
        painter.drawPixmap(checkRect_.topLeft(),QPixmap(":images/check").scaled(maxCheckSize,maxCheckSize));
    }
}

