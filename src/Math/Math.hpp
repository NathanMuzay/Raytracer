/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Math
*/

#ifndef MATH_HPP_
#define MATH_HPP_

#include <cmath>

namespace Math {
    class Point3D;

    class Vector3D {
        public:
            double x;
            double y;
            double z;

            Vector3D() = default;
            Vector3D(double x, double y, double z) : x(x), y(y), z(z) {}

            double length() const { return std::sqrt(x * x + y * y + z * z); }
            Vector3D normalize() const
            {
                double len = length();
                if (len == 0.0)
                    return *this;
                return Vector3D(x / len, y / len, z / len);
            }
            double dot(const Vector3D &other) const { return x * other.x + y * other.y + z * other.z; }
            Vector3D cross(const Vector3D &other) const
            {
                return Vector3D(
                    y * other.z - z * other.y,
                    z * other.x - x * other.z,
                    x * other.y - y * other.x
                );
            }

            Vector3D operator+(const Vector3D &other) const { return Vector3D(x + other.x, y + other.y, z + other.z); }
            Vector3D &operator+=(const Vector3D &other) { x += other.x; y += other.y; z += other.z; return *this; }
            Vector3D operator-(const Vector3D &other) const { return Vector3D(x - other.x, y - other.y, z - other.z); }
            Vector3D &operator-=(const Vector3D &other) { x -= other.x; y -= other.y; z -= other.z; return *this; }
            Vector3D operator*(const Vector3D &other) const { return Vector3D(x * other.x, y * other.y, z * other.z); }
            Vector3D &operator*=(const Vector3D &other) { x *= other.x; y *= other.y; z *= other.z; return *this; }
            Vector3D operator/(const Vector3D &other) const { return Vector3D(x / other.x, y / other.y, z / other.z); }
            Vector3D &operator/=(const Vector3D &other) { x /= other.x; y /= other.y; z /= other.z; return *this; }

            Vector3D operator*(double scalar) const { return Vector3D(x * scalar, y * scalar, z * scalar); }
            Vector3D &operator*=(double scalar) { x *= scalar; y *= scalar; z *= scalar; return *this; }
            Vector3D operator/(double scalar) const { return Vector3D(x / scalar, y / scalar, z / scalar); }
            Vector3D &operator/=(double scalar) { x /= scalar; y /= scalar; z /= scalar; return *this; }
            inline Point3D toPoint() const;
    };

    class Point3D {
        public:
            double x;
            double y;
            double z;

            Point3D() = default;
            Point3D(double x, double y, double z) : x(x), y(y), z(z) {}

            Point3D operator+(const Vector3D &vector) const { return Point3D(x + vector.x, y + vector.y, z + vector.z); }
            Point3D &operator+=(const Vector3D &vector) { x += vector.x; y += vector.y; z += vector.z; return *this; }
            Point3D operator-(const Vector3D &vector) const { return Point3D(x - vector.x, y - vector.y, z - vector.z); }
            Point3D &operator-=(const Vector3D &vector) { x -= vector.x; y -= vector.y; z -= vector.z; return *this; }
            Vector3D operator-(const Point3D &other) const { return Vector3D(x - other.x, y - other.y, z - other.z); }
            Vector3D toVector() const { return Vector3D(x, y, z); }
    };

    inline Point3D Vector3D::toPoint() const { return Point3D(x, y, z); }
}

#endif /* !MATH_HPP_ */
