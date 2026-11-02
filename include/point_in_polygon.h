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

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point);

#endif // POINT_IN_POLYGON_H