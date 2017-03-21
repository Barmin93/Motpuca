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

    anyVector(real _x = 0, real _y = 0, real _z = 0): x(_x), y(_y), z(_z) {}
    real length2() const
    {
        return x*x + y*y + z*z;
    }
    real length() const
    {
        return sqrt(x*x + y*y + z*z);
    }
    void normalize()
    {
        real l = length();
        if (l > 0)
        {
            l = 1/l;
            x *= l;
            y *= l;
            z *= l;
        }
        else
        {
            set_random(3, 1);
//            x = 1;
//            y = z = 0;
        }
    }
    anyVector operator+(anyVector const &v) const
    {
        return anyVector(x + v.x, y + v.y, z + v.z);
    }
    void operator+=(anyVector const &v)
    {
        x += v.x;
        y += v.y;
        z += v.z;
    }
    void operator-=(anyVector const &v)
    {
        x -= v.x;
        y -= v.y;
        z -= v.z;
    }
    void set(real _x = 0, real _y = 0, real _z = 0)
    {
        x = _x;
        y = _y;
        z = _z;
    }
    anyVector operator-(anyVector const &v) const
    {
        return anyVector(x - v.x, y - v.y, z - v.z);
    }
    anyVector operator*(real n) const
    {
        return anyVector(n*x, n*y, n*z);
    }
    anyVector operator*(anyVector const &v) const
    {
        return anyVector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
    }
    void operator*=(real n)
    {
        x *= n;
        y *= n;
        z *= n;
    }
    bool operator==(anyVector const &v) const
    {
        return x == v.x && y == v.y && z == v.z;
    }
    bool operator!=(anyVector const &v) const
    {
        return x != v.x || y != v.y || z != v.z;
    }

    bool is_zero() const
    {
        return !x && !y && !z;
    }

    void set_random(int dim, real length)
    {
        do
        {
            x = 2*(rand()/real(RAND_MAX)) - 1;
            y = 2*(rand()/real(RAND_MAX)) - 1;
            if (dim == 2)
                z = 0;
            else
                z = 2*(rand()/real(RAND_MAX)) - 1;
        }
        while (!x && !y && !z);
        normalize();
        x *= length;
        y *= length;
        z *= length;
    }
    char *toString() const
    {
        static char h[10][100];
        static int h_indx = 0;
        h_indx = (h_indx + 1) % 10;
        snprintf(h[h_indx], 100, "<%g, %g, %g>", x, y, z);
        return h[h_indx];
    }
    anyVector operator*(float *m) const
    {
        real v = m[3]*x + m[7]*y + m[11]*z + m[15];
        return anyVector(m[0]*x + m[4]*y + m[8]*z + m[12],
                         m[1]*x + m[5]*y + m[9]*z + m[13],
                         m[2]*x + m[6]*y + m[10]*z + m[14])*(1/v);
    }
    real operator |(anyVector const &v) const
    {
        return x*v.x + y*v.y + z*v.z;
    }

};

const anyVector vectorZero(0, 0, 0);

#endif // VECTOR_H
