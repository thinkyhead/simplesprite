/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Math.cpp
 *
 *  $Id: SS_Math.cpp,v 1.1 2007/03/02 08:05:54 slurslee Exp $
 *
 */

#include "SS_Math.h"

#include <math.h>

Vector::Vector(float ix, float iy, float iz)
{
    x = ix;
    y = iy;
    z = iz;
}

Vector::Vector(const Vector &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
}

Vector &Vector::set(float ix, float iy, float iz)
{
    x = ix;
    y = iy;
    z = iz;
    return *this;
}

Vector &Vector::operator=(const Vector &v)
{
    x = v.x;
    y = v.y;
    z = v.z;
    return *this;
}

Vector &Vector::operator=(float f)
{
    x = y = z = f;
    return *this;
}

Vector &Vector::operator+(const Vector &v) const
{
    return *(new Vector(x + v.x, y + v.y, z + v.z));
}

Vector &Vector::operator+=(const Vector &v)
{
    x += v.x;
    y += v.y;
    z += v.z;
    return *this;
}

Vector &Vector::operator-(const Vector &v) const
{
    return *(new Vector(x - v.x, y - v.y, z - v.z));
}

Vector &Vector::operator-=(const Vector &v)
{
    x -= v.x;
    y -= v.y;
    z -= v.z;
    return *this;
}

Vector &Vector::operator-() const
{
    return *(new Vector(-x, -y, -z));
}

Vector &Vector::operator*(float f) const
{
    return *(new Vector(x * f, y * f, z * f));
}

Vector &Vector::operator/(float f) const
{
    if (f == 0)
        f = 1;
    return *(new Vector(x / f, y / f, z / f));
}

Vector &Vector::operator/=(float f)
{
    if (f == 0)
        f = 1;
    x /= f;
    y /= f;
    z /= f;
    return *this;
}

Vector &Vector::operator*=(float f)
{
    x *= f;
    y *= f;
    z *= f;
    return *this;
}

float Vector::dot(const Vector &v) const
{
    return x * v.x + y * v.y + z * v.z;
}

Vector Vector::cross(const Vector &v) const
{
    return *(new Vector(
        y * v.z - v.y * z,
        z * v.x - v.z * x,
        x * v.y - v.x * y));
}

float Vector::normal() const
{
    return sqrt( x * x + y * y + z * z );
}

Vector &Vector::unit() const
{
    float n = normal();
    return *this / n;
}

void Vector::normalize()
{
    float n = normal();
    x /= n;
    y /= n;
    z /= n;
}

void Vector::invert()
{
    x = -x;
    y = -y;
    z = -z;
}

Vector &operator*(float f, const Vector v)
{
    return v * f;
}

bool Vector::operator==(const Vector &v)
{
    return (x == v.x && y == v.y && z == v.z);
}

bool Vector::operator==(float f)
{
    return (x == f && y == f && z == f);
}
