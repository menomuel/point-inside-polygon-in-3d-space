#include "point_in_polygon.h"

#include <algorithm>
#include <cmath>

#include "vector_math.h"

using namespace pip;

namespace
{

  // Default boundary tolerance as a fraction of the polygon's bounding box diagonal.
  constexpr double RelativeTolerance = 1e-9;

  bool isFinite(const Point3d& p)
  {
    return std::isfinite(p.x) && std::isfinite(p.y) && std::isfinite(p.z);
  }

  struct BoundingBox
  {
    explicit BoundingBox(const Point3d& p) : min(p), max(p) {}

    void add(const Point3d& p)
    {
      min = { std::min(min.x, p.x), std::min(min.y, p.y), std::min(min.z, p.z) };
      max = { std::max(max.x, p.x), std::max(max.y, p.y), std::max(max.z, p.z) };
    }

    double diagonal() const
    {
      return length(max - min);
    }

    bool contains(const Point3d& p, double tol) const
    {
      return
        p.x >= min.x - tol && p.x <= max.x + tol &&
        p.y >= min.y - tol && p.y <= max.y + tol &&
        p.z >= min.z - tol && p.z <= max.z + tol;
    }

    Point3d min;
    Point3d max;
  };

} // namespace

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point)
{
  return pointInPolygon(polygonPoints, point, 0.);
}

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point, double tolerance)
{
  if (polygonPoints.empty() || !isFinite(point))
    return PointLocation::Outside;

  BoundingBox box(polygonPoints[0]);
  for (const Point3d& p : polygonPoints)
  {
    if (!isFinite(p))
      return PointLocation::Outside;
    box.add(p);
  }

  const bool userTolerance = std::isfinite(tolerance) && tolerance > 0.;
  const double tol = userTolerance ? tolerance : RelativeTolerance * box.diagonal();

  if (!box.contains(point, tol))
    return PointLocation::Outside;

  return PointLocation::Outside; // TODO: process plane, linear case and general
}
