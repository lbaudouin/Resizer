#ifndef IMAGELABEL_H
#define IMAGELABEL_H

#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QFileInfo>
#include <QToolTip>

#include "qexifimageheader/qexifimageheader.h"
#include "rotationstate.h"

#include <QDebug>

class ImageLabel : public QLabel
{
    Q_OBJECT
public:
    ImageLabel(QWidget *parent = 0);
    ImageLabel(QString absoluteFilePath, QWidget *parent = 0);

    void setAbsoluteFilePath(QString absoluteFilePath);

    QString getAbsoluteFilePath();

    bool isChecked() const;
    bool selected() const;

    RotationState rotation();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *e);

    //bool event(QEvent *e);

private:
    QString m_absoluteFilePath;
    QString m_filename;
    bool displayIcons_;
    bool checked_;
    QRect autoRect_,resetRect_,leftRect_,rightRect_,removeRect_,deleteRect_,checkRect_;

    RotationState m_rotation;

public slots:
    void toggleCheck();
    void setChecked();
    void setUnChecked();
    void setRotation(RotationState rotation);
    void setPixmapAndRotation(const QPixmap &pixmap, const RotationState &rotation);
    void rotateLeft();
    void rotateRight();
    void resetRotation();
    void detectRotation();

signals:
    void removePressed(QString);
    void deletePressed(QString);

    void selectionChanged(bool);
};

#endif // MYLABEL_H
