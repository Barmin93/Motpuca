#ifndef TRANSFORM_H
#define TRANSFORM_H

#include "vector.h"

class anyTransform
{
public:
    float matrix[16];
    /*
      OpenGL order:
      0  4  8 12
      1  5  9 13
      2  6 10 14
      3  7 11 15
    */

    anyTransform()
    {
        setToIdentity();
    }

    anyTransform(int)
    {
    }

    float &at(int x, int y)
    {
        return matrix[x + 4*y];
    }

    anyTransform(float t0, float t1, float t2, float t3, float t4, float t5, float t6, float t7, float t8, float t9, float t10, float t11, float t12, float t13, float t14, float t15)
    {
        matrix[0] = t0;
        matrix[1] = t1;
        matrix[2] = t2;
        matrix[3] = t3;
        matrix[4] = t4;
        matrix[5] = t5;
        matrix[6] = t6;
        matrix[7] = t7;
        matrix[8] = t8;
        matrix[9] = t9;
        matrix[10] = t10;
        matrix[11] = t11;
        matrix[12] = t12;
        matrix[13] = t13;
        matrix[14] = t14;
        matrix[15] = t15;
    }

    anyVector getX() const
    {
        return anyVector(matrix[0], matrix[4], matrix[8]);
    }

    anyVector getY() const
    {
        return anyVector(matrix[1], matrix[5], matrix[9]);
    }

    anyVector getZ() const
    {
        return anyVector(matrix[2], matrix[6], matrix[10]);
    }

    void setToIdentity()
    {
        for (int i = 0; i < 16; i++)
            matrix[i] = 0;
        matrix[0] = matrix[5] = matrix[10] = matrix[15] = 1;
    }

    void setToRotationX(float fi)
    {
        fi *= M_PI/180.0;
        float sfi = sin(fi);
        float cfi = cos(fi);
        setToIdentity();
        matrix[5] = cfi;
        matrix[9] = sfi;
        matrix[6] = -sfi;
        matrix[10] = cfi;
    }

    anyTransform &rotateX(float fi)
    {
        anyTransform m(0);
        m.setToRotationX(fi);
        *this = (*this)*m;
        return *this;
    }

    void setToRotationY(float fi)
    {
        fi *= M_PI/180.0;
        float sfi = sin(fi);
        float cfi = cos(fi);
        setToIdentity();
        matrix[0] = cfi;
        matrix[8] = sfi;
        matrix[2] = -sfi;
        matrix[10] = cfi;
    }

    anyTransform &rotateY(float fi)
    {
        anyTransform m(0);
        m.setToRotationY(fi);
        *this = (*this)*m;
        return *this;
    }

    void setToRotationZ(float fi)
    {
        fi *= M_PI/180.0;
        float sfi = sin(fi);
        float cfi = cos(fi);
        setToIdentity();
        matrix[0] = cfi;
        matrix[4] = -sfi;
        matrix[1] = sfi;
        matrix[5] = cfi;
    }

    anyTransform &rotateZ(float fi)
    {
        anyTransform m(0);
        m.setToRotationZ(fi);
        *this = (*this)*m;
        return *this;
    }

    void setToRotation(float fi, anyVector const &p, anyVector const &dir)
    {
        // http://inside.mines.edu/fs_home/gmurray/ArbitraryAxisRotation/
        // eq. (6.2)

        anyVector dirn = dir;
        dirn.normalize();

        fi *= M_PI/180.0;
        double sfi = sin(fi);
        double cfi = cos(fi);

        double u = dirn.x;
        double v = dirn.y;
        double w = dirn.z;

        double a = p.x;
        double b = p.y;
        double c = p.z;

        matrix[0] = u*u + (v*v + w*w)*cfi;
        matrix[4] = u*v*(1 - cfi) - w*sfi;
        matrix[8] = u*w*(1 - cfi) + v*sfi;
        matrix[12] = (a*(v*v + w*w) - u*(b*v + c*w))*(1 - cfi) + (b*w - c*v)*sfi;

        matrix[1] = u*v*(1 - cfi) + w*sfi;
        matrix[5] = v*v + (u*u + w*w)*cfi;
        matrix[9] = v*w*(1 - cfi) - u*sfi;
        matrix[13] = (b*(u*u + w*w) - v*(a*u + c*w))*(1 - cfi) + (c*u - a*w)*sfi;

        matrix[2] = u*w*(1 - cfi) - v*sfi;
        matrix[6] = v*w*(1 - cfi) + u*sfi;
        matrix[10] = w*w + (u*u + v*v)*cfi;
        matrix[14] = (c*(u*u + v*v) - w*(a*u + b*v))*(1 - cfi) + (a*v + b*u)*sfi;

        matrix[3] = 0;
        matrix[7] = 0;
        matrix[11] = 0;
        matrix[15] = 1;
    }

    anyTransform &rotate(float fi, anyVector const &p, anyVector const &dir)
    {
        anyTransform m(0);
        m.setToRotation(fi, p, dir);
        *this = (*this)*m;
        return *this;
    }

    void setToTranslation(anyVector const &d)
    {
        setToIdentity();
        matrix[12] = d.x;
        matrix[13] = d.y;
        matrix[14] = d.z;
    }

    anyTransform &translate(anyVector const &d)
    {
        anyTransform m(0);
        m.setToTranslation(d);
        *this = (*this)*m;
        return *this;
    }

    void setToScale(anyVector const &d)
    {
        setToIdentity();
        matrix[0] = d.x;
        matrix[5] = d.y;
        matrix[10] = d.z;
    }

    anyTransform &scale(anyVector const &d)
    {
        anyTransform m(0);
        m.setToScale(d);
        *this = (*this)*m;
        return *this;
    }

    void setToFrustum(float near_plane, float far_plane, float left_plane, float right_plane, float top_plane, float bottom_plane)
    {
        // http://www.songho.ca/opengl/gl_projectionmatrix.html

        matrix[0] = 2*near_plane/(right_plane - left_plane);
        matrix[1] = 0;
        matrix[2] = 0;
        matrix[3] = 0;
        matrix[4] = 0;
        matrix[5] = 2*near_plane/(top_plane - bottom_plane);
        matrix[6] = 0;
        matrix[7] = 0;
        matrix[8] = (right_plane + left_plane)/(right_plane - left_plane);
        matrix[9] = (top_plane + bottom_plane)/(top_plane - bottom_plane);
        matrix[10] = (far_plane + near_plane)/(near_plane - far_plane);
        matrix[11] = -1;
        matrix[12] = 0;
        matrix[13] = 0;
        matrix[14] = 2*far_plane*near_plane/(near_plane - far_plane);
        matrix[15] = 0;
    }

    void setToPerspective(float verticalAngle, float aspectRatio, float nearPlane, float farPlane)
    {
        // https://unspecified.wordpress.com/2012/06/21/calculating-the-gluperspective-matrix-and-other-opengl-matrix-maths/

        float f = 1.0/tan(M_PI*verticalAngle/360.0); //  = /2.0 /180.0
        matrix[0] = f/aspectRatio;
        matrix[1] = 0;
        matrix[2] = 0;
        matrix[3] = 0;
        matrix[4] = 0;
        matrix[5] = f;
        matrix[6] = 0;
        matrix[7] = 0;
        matrix[8] = 0;
        matrix[9] = 0;
        matrix[10] = (farPlane + nearPlane)/(nearPlane - farPlane);
        matrix[11] = -1;
        matrix[12] = 0;
        matrix[13] = 0;
        matrix[14] = 2*(farPlane*nearPlane)/(nearPlane - farPlane);
        matrix[15] = 0;
    }

    anyVector operator*(anyVector const &v) const
    {
        return anyVector(matrix[0]*v.x + matrix[4]*v.y + matrix[8]*v.z  + matrix[12],
                matrix[1]*v.x + matrix[5]*v.y + matrix[9]*v.z  + matrix[13],
                matrix[2]*v.x + matrix[6]*v.y + matrix[10]*v.z + matrix[14]);
    }

    anyTransform operator*(anyTransform const &t) const
    {
        return anyTransform(t.matrix[0]*matrix[0] + t.matrix[1]*matrix[4] + t.matrix[2]*matrix[8] + t.matrix[3]*matrix[12],
                t.matrix[0]*matrix[1] + t.matrix[1]*matrix[5] + t.matrix[2]*matrix[9] + t.matrix[3]*matrix[13],
                t.matrix[0]*matrix[2] + t.matrix[1]*matrix[6] + t.matrix[2]*matrix[10] + t.matrix[3]*matrix[14],
                t.matrix[0]*matrix[3] + t.matrix[1]*matrix[7] + t.matrix[2]*matrix[11] + t.matrix[3]*matrix[15],

                t.matrix[4]*matrix[0] + t.matrix[5]*matrix[4] + t.matrix[6]*matrix[8] + t.matrix[7]*matrix[12],
                t.matrix[4]*matrix[1] + t.matrix[5]*matrix[5] + t.matrix[6]*matrix[9] + t.matrix[7]*matrix[13],
                t.matrix[4]*matrix[2] + t.matrix[5]*matrix[6] + t.matrix[6]*matrix[10] + t.matrix[7]*matrix[14],
                t.matrix[4]*matrix[3] + t.matrix[5]*matrix[7] + t.matrix[6]*matrix[11] + t.matrix[7]*matrix[15],

                t.matrix[8]*matrix[0] + t.matrix[9]*matrix[4] + t.matrix[10]*matrix[8] + t.matrix[11]*matrix[12],
                t.matrix[8]*matrix[1] + t.matrix[9]*matrix[5] + t.matrix[10]*matrix[9] + t.matrix[11]*matrix[13],
                t.matrix[8]*matrix[2] + t.matrix[9]*matrix[6] + t.matrix[10]*matrix[10] + t.matrix[11]*matrix[14],
                t.matrix[8]*matrix[3] + t.matrix[9]*matrix[7] + t.matrix[10]*matrix[11] + t.matrix[11]*matrix[15],

                t.matrix[12]*matrix[0] + t.matrix[13]*matrix[4] + t.matrix[14]*matrix[8] + t.matrix[15]*matrix[12],
                t.matrix[12]*matrix[1] + t.matrix[13]*matrix[5] + t.matrix[14]*matrix[9] + t.matrix[15]*matrix[13],
                t.matrix[12]*matrix[2] + t.matrix[13]*matrix[6] + t.matrix[14]*matrix[10] + t.matrix[15]*matrix[14],
                t.matrix[12]*matrix[3] + t.matrix[13]*matrix[7] + t.matrix[14]*matrix[11] + t.matrix[15]*matrix[15]
                );
    }

    char *toString(char const *delimiter1 = "<", char const *delimiter2 = ">") const
    {
        static char h[10][500];
        static int h_indx = 0;
        h_indx = (h_indx + 1) % 10;
        snprintf(h[h_indx], 500, "%s%g, %g, %g, %g,  %g, %g, %g, %g,  %g, %g, %g, %g,   %g, %g, %g, %g%s",
                 delimiter1,
                 matrix[0], matrix[1], matrix[2], matrix[3],
                matrix[4], matrix[5], matrix[6], matrix[7],
                matrix[8], matrix[9], matrix[10], matrix[11],
                matrix[12], matrix[13], matrix[14], matrix[15],
                delimiter2
                );
        return h[h_indx];
    }

    char *toString2() const
    {
        static char h[10][500];
        static int h_indx = 0;
        h_indx = (h_indx + 1) % 10;
        snprintf(h[h_indx], 500, "\t%g, \t%g, \t%g, \t%g,\n\t%g, \t%g, \t%g, \t%g,\n\t%g, \t%g, \t%g, \t%g,\n\t%g, \t%g, \t%g, \t%g",
                 matrix[0], matrix[4], matrix[8], matrix[12],
                matrix[1], matrix[5], matrix[9], matrix[13],
                matrix[2], matrix[6], matrix[10], matrix[14],
                matrix[3], matrix[7], matrix[11], matrix[15]
                );
        return h[h_indx];
    }

    real det() const
    {
        return
                matrix[12]*matrix[9]*matrix[6]*matrix[3]  - matrix[8]*matrix[13]*matrix[6]*matrix[3]  - matrix[12]*matrix[5]*matrix[10]*matrix[3] + matrix[4]*matrix[13]*matrix[10]*matrix[3] +
                matrix[8]*matrix[5]*matrix[14]*matrix[3]  - matrix[4]*matrix[9]*matrix[14]*matrix[3]  - matrix[12]*matrix[9]*matrix[2]*matrix[7]  + matrix[8]*matrix[13]*matrix[2]*matrix[7]  +
                matrix[12]*matrix[1]*matrix[10]*matrix[7] - matrix[0]*matrix[13]*matrix[10]*matrix[7] - matrix[8]*matrix[1]*matrix[14]*matrix[7]  + matrix[0]*matrix[9]*matrix[14]*matrix[7]  +
                matrix[12]*matrix[5]*matrix[2]*matrix[11] - matrix[4]*matrix[13]*matrix[2]*matrix[11] - matrix[12]*matrix[1]*matrix[6]*matrix[11] + matrix[0]*matrix[13]*matrix[6]*matrix[11] +
                matrix[4]*matrix[1]*matrix[14]*matrix[11] - matrix[0]*matrix[5]*matrix[14]*matrix[11] - matrix[8]*matrix[5]*matrix[2]*matrix[15]  + matrix[4]*matrix[9]*matrix[2]*matrix[15]  +
                matrix[8]*matrix[1]*matrix[6]*matrix[15]  - matrix[0]*matrix[9]*matrix[6]*matrix[15]  - matrix[4]*matrix[1]*matrix[10]*matrix[15] + matrix[0]*matrix[5]*matrix[10]*matrix[15];
    }

    anyTransform inverted() const
    {
        anyTransform v;
        real x = det();

        if (x != 0)
        {
            v.matrix[0]  = matrix[9]*matrix[14]*matrix[7]  - matrix[13]*matrix[10]*matrix[7] + matrix[13]*matrix[6]*matrix[11] - matrix[5]*matrix[14]*matrix[11] - matrix[9]*matrix[6]*matrix[15] + matrix[5]*matrix[10]*matrix[15];
            v.matrix[4]  = matrix[12]*matrix[10]*matrix[7] - matrix[8]*matrix[14]*matrix[7]  - matrix[12]*matrix[6]*matrix[11] + matrix[4]*matrix[14]*matrix[11] + matrix[8]*matrix[6]*matrix[15] - matrix[4]*matrix[10]*matrix[15];
            v.matrix[8]  = matrix[8]*matrix[13]*matrix[7]  - matrix[12]*matrix[9]*matrix[7]  + matrix[12]*matrix[5]*matrix[11] - matrix[4]*matrix[13]*matrix[11] - matrix[8]*matrix[5]*matrix[15] + matrix[4]*matrix[9]*matrix[15];
            v.matrix[12] = matrix[12]*matrix[9]*matrix[6]  - matrix[8]*matrix[13]*matrix[6]  - matrix[12]*matrix[5]*matrix[10] + matrix[4]*matrix[13]*matrix[10] + matrix[8]*matrix[5]*matrix[14] - matrix[4]*matrix[9]*matrix[14];
            v.matrix[1]  = matrix[13]*matrix[10]*matrix[3] - matrix[9]*matrix[14]*matrix[3]  - matrix[13]*matrix[2]*matrix[11] + matrix[1]*matrix[14]*matrix[11] + matrix[9]*matrix[2]*matrix[15] - matrix[1]*matrix[10]*matrix[15];
            v.matrix[5]  = matrix[8]*matrix[14]*matrix[3]  - matrix[12]*matrix[10]*matrix[3] + matrix[12]*matrix[2]*matrix[11] - matrix[0]*matrix[14]*matrix[11] - matrix[8]*matrix[2]*matrix[15] + matrix[0]*matrix[10]*matrix[15];
            v.matrix[9]  = matrix[12]*matrix[9]*matrix[3]  - matrix[8]*matrix[13]*matrix[3]  - matrix[12]*matrix[1]*matrix[11] + matrix[0]*matrix[13]*matrix[11] + matrix[8]*matrix[1]*matrix[15] - matrix[0]*matrix[9]*matrix[15];
            v.matrix[13] = matrix[8]*matrix[13]*matrix[2]  - matrix[12]*matrix[9]*matrix[2]  + matrix[12]*matrix[1]*matrix[10] - matrix[0]*matrix[13]*matrix[10] - matrix[8]*matrix[1]*matrix[14] + matrix[0]*matrix[9]*matrix[14];
            v.matrix[2]  = matrix[5]*matrix[14]*matrix[3]  - matrix[13]*matrix[6]*matrix[3]  + matrix[13]*matrix[2]*matrix[7]  - matrix[1]*matrix[14]*matrix[7]  - matrix[5]*matrix[2]*matrix[15] + matrix[1]*matrix[6]*matrix[15];
            v.matrix[6]  = matrix[12]*matrix[6]*matrix[3]  - matrix[4]*matrix[14]*matrix[3]  - matrix[12]*matrix[2]*matrix[7]  + matrix[0]*matrix[14]*matrix[7]  + matrix[4]*matrix[2]*matrix[15] - matrix[0]*matrix[6]*matrix[15];
            v.matrix[10] = matrix[4]*matrix[13]*matrix[3]  - matrix[12]*matrix[5]*matrix[3]  + matrix[12]*matrix[1]*matrix[7]  - matrix[0]*matrix[13]*matrix[7]  - matrix[4]*matrix[1]*matrix[15] + matrix[0]*matrix[5]*matrix[15];
            v.matrix[14] = matrix[12]*matrix[5]*matrix[2]  - matrix[4]*matrix[13]*matrix[2]  - matrix[12]*matrix[1]*matrix[6]  + matrix[0]*matrix[13]*matrix[6]  + matrix[4]*matrix[1]*matrix[14] - matrix[0]*matrix[5]*matrix[14];
            v.matrix[3]  = matrix[9]*matrix[6]*matrix[3]   - matrix[5]*matrix[10]*matrix[3]  - matrix[9]*matrix[2]*matrix[7]   + matrix[1]*matrix[10]*matrix[7]  + matrix[5]*matrix[2]*matrix[11] - matrix[1]*matrix[6]*matrix[11];
            v.matrix[7]  = matrix[4]*matrix[10]*matrix[3]  - matrix[8]*matrix[6]*matrix[3]   + matrix[8]*matrix[2]*matrix[7]   - matrix[0]*matrix[10]*matrix[7]  - matrix[4]*matrix[2]*matrix[11] + matrix[0]*matrix[6]*matrix[11];
            v.matrix[11] = matrix[8]*matrix[5]*matrix[3]   - matrix[4]*matrix[9]*matrix[3]   - matrix[8]*matrix[1]*matrix[7]   + matrix[0]*matrix[9]*matrix[7]   + matrix[4]*matrix[1]*matrix[11] - matrix[0]*matrix[5]*matrix[11];
            v.matrix[15] = matrix[4]*matrix[9]*matrix[2]   - matrix[8]*matrix[5]*matrix[2]   + matrix[8]*matrix[1]*matrix[6]   - matrix[0]*matrix[9]*matrix[6]   - matrix[4]*matrix[1]*matrix[10] + matrix[0]*matrix[5]*matrix[10];

            x = 1/x;
            for (int i = 0; i < 16; i++)
                v.matrix[i] *= x;
        }
        return v;
    }


    anyTransform &inverse()
    {
        *this = inverted();
        return *this;
    }


    anyTransform normMatrix()
    {
        anyTransform v;

        float det =    +at(0,0)*(at(1,1)*at(2,2)-at(2,1)*at(1,2))
                       -at(0,1)*(at(1,0)*at(2,2)-at(1,2)*at(2,0))
                       +at(0,2)*(at(1,0)*at(2,1)-at(1,1)*at(2,0));
        float invdet = 1/det;

        v.at(0,0) =  (at(1,1)*at(2,2)-at(2,1)*at(1,2))*invdet;
        v.at(1,0) = -(at(0,1)*at(2,2)-at(0,2)*at(2,1))*invdet;
        v.at(2,0) =  (at(0,1)*at(1,2)-at(0,2)*at(1,1))*invdet;
        v.at(0,1) = -(at(1,0)*at(2,2)-at(1,2)*at(2,0))*invdet;
        v.at(1,1) =  (at(0,0)*at(2,2)-at(0,2)*at(2,0))*invdet;
        v.at(2,1) = -(at(0,0)*at(1,2)-at(1,0)*at(0,2))*invdet;
        v.at(0,2) =  (at(1,0)*at(2,1)-at(2,0)*at(1,1))*invdet;
        v.at(1,2) = -(at(0,0)*at(2,1)-at(2,0)*at(0,1))*invdet;
        v.at(2,2) =  (at(0,0)*at(1,1)-at(1,0)*at(0,1))*invdet;

        return v;
    }

    float *matrix3x3() const
    {
        static float v[9];

        v[0] = matrix[0];
        v[1] = matrix[1];
        v[2] = matrix[2];

        v[3] = matrix[4];
        v[4] = matrix[5];
        v[5] = matrix[6];

        v[6] = matrix[8];
        v[7] = matrix[9];
        v[8] = matrix[10];

        return v;
    }

};

#endif // TRANSFORM_H
