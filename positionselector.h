#ifndef POSITIONSELECTOR_H
#define POSITIONSELECTOR_H

#include <QWidget>
#include <QMouseEvent>
#include <QPainter>

class PositionSelector : public QWidget
{
    Q_OBJECT

public:
    enum POSITION {TOP_LEFT=0,TOP_RIGHT,BOTTOM_LEFT,BOTTOM_RIGHT,CENTER};

    explicit PositionSelector(QWidget *parent = 0);

    inline POSITION position() {return position_;}
    inline void setPosition(POSITION pos){ if(pos<TOP_LEFT || pos>CENTER) position_ = BOTTOM_RIGHT; else position_ = pos;}

protected:
    void mousePressEvent(QMouseEvent *);
    void mouseMoveEvent(QMouseEvent *);
    void mouseReleaseEvent(QMouseEvent *);
    void enterEvent(QEvent *);
    void leaveEvent(QEvent *);

    void paintEvent(QPaintEvent *);

    QSize sizeHint();
    QSize minimumSizeHint();

private:
    POSITION position_;
    bool focus_;
    POSITION mousePosition_;
    
signals:
    
public slots:
    
};

#endif // POSITIONSELECTOR_H
