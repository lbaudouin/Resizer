#ifndef ROTATIONSTATE_H
#define ROTATIONSTATE_H

#include <QFileInfo>

enum RotationState {NO_ROTATION=0, CLOCKWISE, REVERSE, COUNTERCLOCKWISE, INVALID_ROTATION};
enum RotationDirection {ROTATION_LEFT=0, ROTATION_RIGHT};

Q_DECLARE_METATYPE( RotationState )

struct ImageInfo{
    QFileInfo fileinfo;
    RotationState rotation;
};


#endif // ROTATIONSTATE_H
