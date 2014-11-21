#include "imagelabel.h"

ImageLabel::ImageLabel(QWidget *parent) : QLabel(parent), m_absoluteFilePath(""), m_filename(""), displayIcons_(false), checked_(false)
{
    this->setMouseTracking(true);
}

ImageLabel::ImageLabel(QString absoluteFilePath, QWidget *parent) : QLabel(parent), displayIcons_(false), checked_(false)
{
    this->setMouseTracking(true);
    setAbsoluteFilePath(absoluteFilePath);
}

void ImageLabel::setPixmapAndRotation(const QPixmap &pixmap, const RotationState &rotation)
{
    setPixmap(pixmap);
    setRotation(rotation);
}

void ImageLabel::setAbsoluteFilePath(QString absoluteFilePath)
{
    m_absoluteFilePath = absoluteFilePath;
    m_filename = QFileInfo(m_absoluteFilePath).fileName();

    this->setToolTip(m_filename);
}

QString ImageLabel::getAbsoluteFilePath()
{
    return m_absoluteFilePath;
}

void ImageLabel::toggleCheck()
{
    checked_ = !checked_;
    update();
    emit selectionChanged(checked_);
}

void ImageLabel::setChecked()
{
    checked_ = true;
    update();
    emit selectionChanged(checked_);
}

void ImageLabel::setUnChecked()
{
    checked_ = false;
    update();
    emit selectionChanged(checked_);
}

bool ImageLabel::isChecked() const
{
    return checked_;
}

bool ImageLabel::selected() const
{
    return checked_;
}

void ImageLabel::mousePressEvent(QMouseEvent *e)
{
    if(autoRect_.contains(e->pos()) || resetRect_.contains(e->pos()) || leftRect_.contains(e->pos()) || rightRect_.contains(e->pos()) ||
       removeRect_.contains(e->pos()) || deleteRect_.contains(e->pos()) )
        return;

    if(e->button()==Qt::LeftButton){
        toggleCheck();
        update();
    }
    if(e->button()==Qt::RightButton){
        setChecked();
        update();
    }
    if(e->button()==Qt::MiddleButton){
        setUnChecked();
        update();
    }
}

void ImageLabel::mouseReleaseEvent(QMouseEvent *e)
{
    if(autoRect_.contains(e->pos())){
        detectRotation();
    }
    if(resetRect_.contains(e->pos())){
        resetRotation();
    }
    if(leftRect_.contains(e->pos())){
        rotateLeft();
    }
    if(rightRect_.contains(e->pos())){
        rotateRight();
    }

    if(removeRect_.contains(e->pos())){
        emit this->removePressed(m_absoluteFilePath);
    }
    if(deleteRect_.contains(e->pos())){
        emit this->deletePressed(m_absoluteFilePath);
    }
}

void ImageLabel::mouseMoveEvent(QMouseEvent *e)
{
    if(autoRect_.contains(e->pos())){
        this->setCursor(Qt::PointingHandCursor);
        //this->setStatusTip(tr("Detect rotation"));
        this->setToolTip(tr("Detect rotation"));
    }else if(resetRect_.contains(e->pos())){
        this->setCursor(Qt::PointingHandCursor);
        //this->setStatusTip(tr("Reset rotation"));
        this->setToolTip(tr("Reset rotation"));
    }else if(leftRect_.contains(e->pos())){
        this->setCursor(Qt::PointingHandCursor);
        //this->setStatusTip(tr("Rotate to left"));
        this->setToolTip(tr("Rotate to left"));
    }else if(rightRect_.contains(e->pos())){
        this->setCursor(Qt::PointingHandCursor);
        //this->setStatusTip(tr("Rotate to right"));
        this->setToolTip(tr("Rotate to right"));
    }else if(removeRect_.contains(e->pos())){
        this->setCursor(Qt::PointingHandCursor);
        //this->setStatusTip(tr("Remove from grid"));
        this->setToolTip(tr("Remove from grid"));
    }else if(deleteRect_.contains(e->pos())){
        this->setCursor(Qt::PointingHandCursor);
        //this->setStatusTip(tr("Delete file"));
        this->setToolTip(tr("Delete file"));
    }else{
        this->setCursor(Qt::ArrowCursor);
        //this->setStatusTip("");
        this->setToolTip(m_filename);
    }
    //QToolTip::showText(e->globalPos(), this->toolTip());
    //QStatusTipEvent(this->statusTip());
}

void ImageLabel::enterEvent(QEvent *)
{
    displayIcons_ = true;
    update();
}

void ImageLabel::leaveEvent(QEvent *)
{
    displayIcons_ = false;
    update();
    this->setCursor(Qt::ArrowCursor);
}

void ImageLabel::paintEvent(QPaintEvent *e){
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
        painter.drawPixmap(resetRect_.topLeft(),QPixmap(":images/reset").scaled(32,32));
        painter.drawPixmap(leftRect_.topLeft(),QPixmap(":images/left").scaled(32,32));
        painter.drawPixmap(rightRect_.topLeft(),QPixmap(":images/right").scaled(32,32));
        painter.drawPixmap(removeRect_.topLeft(),QPixmap(":images/remove").scaled(32,32));
        painter.drawPixmap(deleteRect_.topLeft(),QPixmap(":images/delete").scaled(32,32));
    }

    if(checked_){
        painter.drawPixmap(checkRect_.topLeft(),QPixmap(":images/check").scaled(maxCheckSize,maxCheckSize));
    }
}

void ImageLabel::setRotation(RotationState rotation)
{
    if(m_rotation==rotation)
        return;

    //Reset transformation
    QTransform transform;
    switch(m_rotation){
    case CLOCKWISE: transform.rotate(-90); break;
    case REVERSE: transform.rotate(-180); break;
    case COUNTERCLOCKWISE: transform.rotate(90); break;
    default: break;
    }
    switch(rotation){
    case CLOCKWISE: transform.rotate(90); break;
    case REVERSE: transform.rotate(180); break;
    case COUNTERCLOCKWISE: transform.rotate(-90); break;
    default: break;
    }

    m_rotation = rotation;
    setPixmap( pixmap()->transformed(transform) );
}

void ImageLabel::rotateLeft()
{
    switch(m_rotation){
    case NO_ROTATION: setRotation(COUNTERCLOCKWISE); break;
    case CLOCKWISE: setRotation(NO_ROTATION); break;
    case REVERSE: setRotation(CLOCKWISE); break;
    case COUNTERCLOCKWISE: setRotation(REVERSE); break;
    default: break;
    }
}

void ImageLabel::rotateRight()
{
    switch(m_rotation){
    case NO_ROTATION: setRotation(CLOCKWISE); break;
    case CLOCKWISE: setRotation(REVERSE); break;
    case REVERSE: setRotation(COUNTERCLOCKWISE); break;
    case COUNTERCLOCKWISE: setRotation(NO_ROTATION); break;
    default: break;
    }
}

RotationState ImageLabel::rotation()
{
    return m_rotation;
}

void ImageLabel::resetRotation()
{
    setRotation(NO_ROTATION);
}

void ImageLabel::detectRotation()
{
    int orientation = QExifImageHeader(m_absoluteFilePath).value(QExifImageHeader::Orientation).toShort();
    RotationState rotation;
    switch(orientation){
    case 6: rotation = CLOCKWISE; break;
    case 3: rotation = REVERSE; break;
    case 8: rotation = COUNTERCLOCKWISE; break;
    default: rotation = NO_ROTATION;
    }

    setRotation(rotation);
}
