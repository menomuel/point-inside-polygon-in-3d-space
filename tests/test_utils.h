#ifndef TEST_UTILS_H
#define TEST_UTILS_H

#include "point_in_polygon.h"

#include <cmath>
#include <initializer_list>
#include <utility>
#include <vector>

constexpr PointLocation Inside = PointLocation::Inside;
constexpr PointLocation Outside = PointLocation::Outside;
constexpr PointLocation Boundary = PointLocation::Boundary;

inline const char* toString(PointLocation location)
{
  switch (location)
  {
  case PointLocation::Outside:
    return "Outside";
  case PointLocation::Inside:
    return "Inside";
  case PointLocation::Boundary:
    return "Boundary";
  }
  return "?";
}

inline Point3d pointXY(double x, double y)
{
  return { x, y, 0. };
}

// Polygon in the XY plane, z = 0.
inline std::vector<Point3d> polygonXY(std::initializer_list<std::pair<double, double>> xy)
{
  std::vector<Point3d> points;
  for (const auto& p : xy)
  {
    points.push_back({ p.first, p.second, 0. });
  }
  return points;
}

inline std::vector<Point3d> unitSquare()
{
  return polygonXY({ { 0., 0. }, { 1., 0. }, { 1., 1. }, { 0., 1. } });
}

// U shape: 3x3 square with a 1x2 notch cut from the top middle.
inline std::vector<Point3d> uShape()
{
  return polygonXY({ { 0., 0. }, { 3., 0. }, { 3., 3. }, { 2., 3. }, { 2., 1. }, { 1., 1. }, { 1., 3. }, { 0., 3. } });
}

// Self-intersecting "bow tie": two triangles touching at (1, 1) with opposite orientation.
inline std::vector<Point3d> bowTie()
{
  return polygonXY({ { 0., 0. }, { 2., 2. }, { 2., 0. }, { 0., 2. } });
}

// Five-pointed star drawn with a single closed line (vertices of a regular pentagon taken
// in the order 0, 2, 4, 1, 3), centred at the origin with circumradius 1.
inline std::vector<Point3d> pentagram()
{
  const double pi = std::acos(-1.);
  std::vector<Point3d> points;
  for (int i = 0; i < 5; ++i)
  {
    const double angle = pi / 2. + 2. * pi * ((2 * i) % 5) / 5.;
    points.push_back({ std::cos(angle), std::sin(angle), 0. });
  }
  return points;
}

#endif // TEST_UTILS_H