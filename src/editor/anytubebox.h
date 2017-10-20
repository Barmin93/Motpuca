#ifndef ANYTUBEBOX_H
#define ANYTUBEBOX_H

#include "anytube.h"

class anyTubeBox
{
public:
    int no_tubes;
    anyTube **tubes;

    anyTubeBox(): no_tubes(0), tubes(0) {}
};

#endif // ANYTUBEBOX_H
