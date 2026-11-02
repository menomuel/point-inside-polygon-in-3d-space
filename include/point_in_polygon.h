#ifndef POINT_IN_POLYGON_H
#define POINT_IN_POLYGON_H

#include <vector>

struct Point3d
{
  double x{ 0. };
  double y{ 0. };
  double z{ 0. };
};

enum class PointLocation
{
  Outside,
  Inside,
  Boundary
};

// Rule that defines "inside" for self-intersecting polygons. Both rules agree for simple polygons.
enum class FillRule
{
  EvenOdd, // inside if a ray from the point crosses the boundary an odd number of times
  NonZero  // inside if the boundary winds around the point a non-zero number of times
};

struct Options
{
  // Distance at which a point is considered to lie on the boundary or in the polygon plane.
  // A non-positive (or non-finite) value selects a default relative to the polygon size.
  double tolerance{ 0. };
  FillRule fillRule{ FillRule::EvenOdd };
};

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point) noexcept;

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point, Options options) noexcept;

#endif // POINT_IN_POLYGON_H