#ifndef SAVER_H
#define SAVER_H

#include <QObject>
#include <QRunnable>

#include <QFileInfo>

#include <QImage>
#include <QImageReader>

#include <QDir>

#include "qexifimageheader/qexifimageheader.h"
#include "positionselector.h"

class Saver : public QObject, public QRunnable
{
    Q_OBJECT
public:
    explicit Saver(QObject *parent = 0);
    void run();

    void setFileInfo(QFileInfo info);
    void setOutputSubfolder(QString subfolder);
    void setNeedRotation(bool rotation);
    void setNoResize(bool noResize);
    void setUseRatio(bool useRatio);
    void setAddLogo(bool addLogo);

    void setSizeMax(int sizeMax);
    void setRatio(double ratio);

    void setLogo(QImage logo);
    void setLogoPosition(PositionSelector::POSITION position, int xShift, int yShift);

private:
    QFileInfo info_;
    QString subfolder_;
    bool needRotation_;
    bool noResize_;
    bool useRatio_;
    bool addLogo_;

    int sizeMax_;
    double ratio_;

    QImage logo_;
    int xShift_,yShift_;
    PositionSelector::POSITION position_;
    
signals:
    void resizeFinished(QString absoluteFilePath);

public slots:
    
};

#endif // SAVER_H
