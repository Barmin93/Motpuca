#include "vector.h"

anyVector::anyVector(): x(0), y(0), z(0){}
anyVector::anyVector(float _x, float _y, float _z): x(_x), y(_y), z(_z) {}
anyVector::anyVector(float _x): x(_x), y(_x), z(_x) {}

float anyVector::length2() const{
    return x*x + y*y + z*z;
}

float anyVector::length() const{
    return sqrt(x*x + y*y + z*z);
}
float anyVector::dot (anyVector const &v){
    return v.x*x + v.y*y + v.z*z;
}

void anyVector::normalize(){
    float l = length();
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
    }
}

anyVector anyVector::operator+(anyVector const &v) const{
    return anyVector(x + v.x, y + v.y, z + v.z);
}

void anyVector::operator+=(anyVector const &v){
    x += v.x;
    y += v.y;
    z += v.z;
}

void anyVector::operator-=(anyVector const &v){
    x -= v.x;
    y -= v.y;
    z -= v.z;
}

void anyVector::set(float _x, float _y, float _z){
    x = _x;
    y = _y;
    z = _z;
}

anyVector anyVector::operator-(anyVector const &v) const{
    return anyVector(x - v.x, y - v.y, z - v.z);
}

anyVector anyVector::operator*(float n) const{
    return anyVector(n*x, n*y, n*z);
}

anyVector anyVector::operator/(float n) const{
    return anyVector(x/n, y/n, z/n);
}

void anyVector::operator/=(float n){
    x /= n;
    y /= n;
    z /= n;
}
void anyVector::operator*=(anyVector const &v){
    x *= v.x;
    y *= v.y;
    z *= v.z;
}

anyVector anyVector::operator*(anyVector const &v) const{
    return anyVector(y*v.z - z*v.y, z*v.x - x*v.z, x*v.y - y*v.x);
}

void anyVector::operator*=(float n){
    x *= n;
    y *= n;
    z *= n;
}

bool anyVector::operator==(anyVector const &v) const{
    return x == v.x && y == v.y && z == v.z;
}

bool anyVector::operator!=(anyVector const &v) const{
    return x != v.x || y != v.y || z != v.z;
}

bool anyVector::is_zero() const{
    return !x && !y && !z;
}

void anyVector::set_random(int dim, float length){
    do
    {
        x = 2*(rand()/float(RAND_MAX)) - 1;
        y = 2*(rand()/float(RAND_MAX)) - 1;
        if (dim == 2)
            z = 0;
        else
            z = 2*(rand()/float(RAND_MAX)) - 1;
    }
    while (!x && !y && !z);
    normalize();
    x *= length;
    y *= length;
    z *= length;
}

char* anyVector::toString() const{
    static char h[10][100];
    static int h_indx = 0;
    h_indx = (h_indx + 1) % 10;
    snprintf(h[h_indx], 100, "<%g, %g, %g>", x, y, z);
    return h[h_indx];
}

anyVector anyVector::operator*(float *m) const{
    float v = m[3]*x + m[7]*y + m[11]*z + m[15];
    return anyVector(m[0]*x + m[4]*y + m[8]*z + m[12],
                     m[1]*x + m[5]*y + m[9]*z + m[13],
                     m[2]*x + m[6]*y + m[10]*z + m[14])*(1/v);
}

float anyVector::operator |(anyVector const &v) const{
    return x*v.x + y*v.y + z*v.z;
}
