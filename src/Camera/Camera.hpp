/*
** EPITECH PROJECT, 2026
** G-OOP-400-MAR-4-1-raytracer-5 [WSL : Ubuntu]
** File description:
** Camera
*/

#ifndef CAMERA_HPP_
#define CAMERA_HPP_

#include "../Math/Rectangle.hpp"
#include "Ray.hpp"

class Camera {
    public:
        Math::Point3D origin;
        Rectangle3D screen;

        Camera() = default;
        Camera(Math::Point3D origin, Rectangle3D screen) : origin(origin), screen(screen) {}

        Ray ray(double u, double v) const
        {
            Math::Point3D screen_point = screen.pointAt(u, v);
            Math::Vector3D direction(
                screen_point.x - origin.x,
                screen_point.y - origin.y,
                screen_point.z - origin.z
            );
            return Ray(origin, direction);
        }

        void translate(double dx, double dy, double dz)
        {
            origin.x += dx;
            origin.y += dy;
            origin.z += dz;
            screen.origin.x += dx;
            screen.origin.y += dy;
            screen.origin.z += dz;
        }

        void rotate(double yaw, double pitch)
        {
            double yawRad   = yaw   * M_PI / 180.0;
            double pitchRad = pitch * M_PI / 180.0;

            Math::Vector3D center(
                screen.origin.x + screen.bottom_side.x * 0.5 + screen.left_side.x * 0.5 - origin.x,
                screen.origin.y + screen.bottom_side.y * 0.5 + screen.left_side.y * 0.5 - origin.y,
                screen.origin.z + screen.bottom_side.z * 0.5 + screen.left_side.z * 0.5 - origin.z
            );

            auto rotateYaw = [&](Math::Vector3D v) -> Math::Vector3D {
                return Math::Vector3D(
                    v.x * std::cos(yawRad) - v.y * std::sin(yawRad),
                    v.x * std::sin(yawRad) + v.y * std::cos(yawRad),
                    v.z
                );
            };

            auto rotatePitch = [&](Math::Vector3D v, Math::Vector3D axis) -> Math::Vector3D {
                axis = axis.normalize();
                double c = std::cos(pitchRad);
                double s = std::sin(pitchRad);
                return v * c + axis.cross(v) * s + axis * axis.dot(v) * (1 - c);
            };

            Math::Vector3D right = screen.bottom_side.normalize();

            center           = rotateYaw(center);
            center           = rotatePitch(center, right);
            screen.bottom_side = rotateYaw(screen.bottom_side);
            screen.bottom_side = rotatePitch(screen.bottom_side, right);
            screen.left_side   = rotateYaw(screen.left_side);
            screen.left_side   = rotatePitch(screen.left_side, right);

            Math::Vector3D newCenter = center;
            screen.origin.x = origin.x + newCenter.x - screen.bottom_side.x * 0.5 - screen.left_side.x * 0.5;
            screen.origin.y = origin.y + newCenter.y - screen.bottom_side.y * 0.5 - screen.left_side.y * 0.5;
            screen.origin.z = origin.z + newCenter.z - screen.bottom_side.z * 0.5 - screen.left_side.z * 0.5;
        }

        void moveRelative(double dx, double dy, double dz)
        {
            Math::Vector3D right = screen.bottom_side.normalize();
            Math::Vector3D up = screen.left_side.normalize();
            Math::Vector3D forward = up.cross(right);

            Math::Vector3D movement = right * dx + forward * dy + up * dz;
            
            origin.x += movement.x;
            origin.y += movement.y;
            origin.z += movement.z;
            screen.origin.x += movement.x;
            screen.origin.y += movement.y;
            screen.origin.z += movement.z;
        }
};

#endif /* !CAMERA_HPP_ */
