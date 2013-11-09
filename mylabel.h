#ifndef MYLABEL_H
#define MYLABEL_H

#include <QLabel>

class MyLabel : public QLabel
{
    Q_OBJECT
public:
    MyLabel(QWidget *parent = 0) : QLabel(parent) {}
    MyLabel(QString absoluteFilePath, QWidget *parent = 0) : QLabel(parent), absoluteFilePath_(absoluteFilePath) {}

    void setAbsoluteFilePath(QString absoluteFilePath) { absoluteFilePath_ = absoluteFilePath;}

    QString getAbsoluteFilePath() { return absoluteFilePath_ ; }

private:
    QString absoluteFilePath_;
};

#endif // MYLABEL_H
