#ifndef VECTOR_H
#define VECTOR_H

#include "types.h"

#include <stdio.h>
#include <float.h>
#include <math.h>
#include <stdlib.h>

class anyVector
{
public:
    real x, y, z;

    anyVector();
    anyVector(real _x, real _y, real _z);
    anyVector(real _x);
    real length2() const;
    real length() const;
    real dot (anyVector const &v);
    void normalize();
    anyVector operator+(anyVector const &v) const;
    void operator+=(anyVector const &v);
    void operator-=(anyVector const &v);
    void set(real _x = 0, real _y = 0, real _z = 0);
    anyVector operator-(anyVector const &v) const;
    anyVector operator*(real n) const;
    anyVector operator/(real n) const;
    anyVector operator*(anyVector const &v) const;
    void operator*=(real n);
    bool operator==(anyVector const &v) const;
    bool operator!=(anyVector const &v) const;
    bool is_zero() const;
    void set_random(int dim, real length);
    char *toString() const;
    anyVector operator*(float *m) const;
    real operator |(anyVector const &v) const;
};

const anyVector vectorZero(0, 0, 0);

#endif // VECTOR_H
