#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>
#include <QPainter>
#include <QPaintEvent>
#include <QFileInfo>
#include <QToolTip>

#include <QDebug>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    MyLabel(QWidget *parent = 0);
    MyLabel(QString absoluteFilePath, QWidget *parent = 0);

    void setAbsoluteFilePath(QString absoluteFilePath);

    QString getAbsoluteFilePath();

    void setChecked(bool checked = true);
    void setUnChecked(bool checked = true);
    bool isChecked();

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);
    void paintEvent(QPaintEvent *e);

    //bool event(QEvent *e);

private:
    QString absoluteFilePath_;
    QString filename_;
    bool displayIcons_;
    bool checked_;
    QRect autoRect_,resetRect_,leftRect_,rightRect_,removeRect_,deleteRect_,checkRect_;

signals:
    void autoPressed(QString);
    void resetPressed(QString);
    void leftPressed(QString);
    void rightPressed(QString);
    void removePressed(QString);
    void deletePressed(QString);
};

#endif // MYLABEL_H
