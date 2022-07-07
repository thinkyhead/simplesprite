/*
 *  OpenGL SimpleSprite Class Suite
 *  (c) 2004 Scott Lahteine.
 *
 *  SS_Math.h
 *
 *  $Id: SS_Math.h,v 1.1 2007/03/02 08:05:45 slurslee Exp $
 *
 */

#ifndef __SS_MATH_H__
#define __SS_MATH_H__

class Vector
{
    public:
        float x, y, z;

        Vector(float x, float y, float z=0.0);
        Vector(const Vector &v);
        Vector() { };

        Vector &operator=(const Vector &v);
        Vector &operator=(float f);

        Vector &set(float x, float y, float z=0.0);

        // inversion
        Vector &operator-() const;
        // sum of 3d Vectors
        Vector &operator+(const Vector &v) const;
        Vector &operator+=(const Vector &v);
        // subtraction of 3d Vectors
        Vector &operator-(const Vector &v) const;
        Vector &operator-=(const Vector &v);
        // product of Vector and scalar
        Vector &operator*(float f) const;
        Vector &operator*=(float f);
        friend Vector &operator*(float f, const Vector v);
        // division of Vector and scalar
        Vector &operator/(float f) const;
        Vector &operator/=(float f);

        // comparison
        bool operator==(const Vector &v);
        bool operator==(float f);

        // dot and cross products
        float dot(const Vector &v) const;
        Vector cross(const Vector &v) const;

        // normal or modulo
        float normal() const;
        Vector &unit() const;

        // transform in place
        void normalize();
        void invert();
};

#endif
