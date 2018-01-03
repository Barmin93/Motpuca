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
    float x, y, z;

    anyVector();
    anyVector(float _x, float _y, float _z);
    anyVector(float _x);
    float length2() const;
    float length() const;
    float dot (anyVector const &v);
    void normalize();
    anyVector operator+(anyVector const &v) const;
    void operator+=(anyVector const &v);
    void operator-=(anyVector const &v);
    void set(float _x = 0, float _y = 0, float _z = 0);
    anyVector operator-(anyVector const &v) const;
    anyVector operator*(float n) const;
    anyVector operator/(float n) const;
    anyVector operator*(anyVector const &v) const;
    void operator*=(float n);
    bool operator==(anyVector const &v) const;
    bool operator!=(anyVector const &v) const;
    bool is_zero() const;
    void set_random(int dim, float length);
    char *toString() const;
    anyVector operator*(float *m) const;
    float operator |(anyVector const &v) const;
};

const anyVector vectorZero(0, 0, 0);

#endif // VECTOR_H
