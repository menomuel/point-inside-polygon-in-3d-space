#ifndef POINT_IN_POLYGON_VECTOR_MATH_H
#define POINT_IN_POLYGON_VECTOR_MATH_H

#include "point_in_polygon.h"

#include <cmath>

namespace pip
{

  struct Vector2d
  {
    double x{ 0. };
    double y{ 0. };
  };

  inline Vector2d operator-(const Vector2d& a, const Vector2d& b)
  {
    return { a.x - b.x, a.y - b.y };
  }

  inline Vector2d operator*(const Vector2d& a, double s)
  {
    return { a.x * s, a.y * s };
  }

  inline double dot(const Vector2d& a, const Vector2d& b)
  {
    return a.x * b.x + a.y * b.y;
  }

  // z-component of the 3D cross product; > 0 if b is counter-clockwise from a
  inline double cross(const Vector2d& a, const Vector2d& b)
  {
    return a.x * b.y - a.y * b.x;
  }

  struct Vector3d
  {
    double x{ 0. };
    double y{ 0. };
    double z{ 0. };
  };

  inline Vector3d operator-(const Point3d& a, const Point3d& b)
  {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
  }

  inline Vector3d operator+(const Vector3d& a, const Vector3d& b)
  {
    return { a.x + b.x, a.y + b.y, a.z + b.z };
  }

  inline Vector3d operator-(const Vector3d& a, const Vector3d& b)
  {
    return { a.x - b.x, a.y - b.y, a.z - b.z };
  }

  inline Vector3d operator*(const Vector3d& a, double s)
  {
    return { a.x * s, a.y * s, a.z * s };
  }

  inline double dot(const Vector3d& a, const Vector3d& b)
  {
    return a.x * b.x + a.y * b.y + a.z * b.z;
  }

  inline Vector3d cross(const Vector3d& a, const Vector3d& b)
  {
    return { a.y * b.z - a.z * b.y, a.z * b.x - a.x * b.z, a.x * b.y - a.y * b.x };
  }

  inline double length(const Vector3d& a)
  {
    return std::sqrt(dot(a, a));
  }

  inline Vector3d normalize(const Vector3d& a)
  {
    return a * (1. / length(a));
  }

  // Squared distance from p to the segment [a, b]; works for a zero-length segment too.
  template <class Vec>
  double calculateDistanceToSegmentSquared(const Vec& p, const Vec& a, const Vec& b)
  {
    const Vec ab = b - a;
    const Vec ap = p - a;
    const double len2 = dot(ab, ab);
    double t = len2 > 0. ? dot(ap, ab) / len2 : 0.;
    t = t < 0. ? 0. : (t > 1. ? 1. : t);
    const Vec d = ap - ab * t;
    return dot(d, d);
  }

} // namespace pip

#endif // POINT_IN_POLYGON_VECTOR_MATH_H