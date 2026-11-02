#include "point_in_polygon.h"

#include "vector_math.h"

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point)
{
  return PointLocation::Outside;
}
