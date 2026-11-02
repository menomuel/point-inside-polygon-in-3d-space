#include "point_in_polygon.h"

#include <algorithm>
#include <cmath>
#include <cstddef>

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

  struct Plane
  {
    Vector2d project(const Point3d& p) const
    {
      const Vector3d d = p - origin;
      return { dot(d, u), dot(d, v) };
    }

    double calculateSignedDistance(const Point3d& p) const
    {
      return dot(p - origin, normal);
    }

    Point3d origin;
    Vector3d u;
    Vector3d v;
    Vector3d normal;
  };

  bool tryComputePlane(const std::vector<Point3d>& points, double tol, Plane& plane)
  {
    const Point3d& origin = points[0];

    // Calculate point farthest from origin
    Vector3d farthest;
    double farthestDist2 = 0.;
    for (const Point3d& p : points)
    {
      const Vector3d vec = p - origin;
      const double dist2 = dot(vec, vec);
      if (dist2 > farthestDist2)
      {
        farthestDist2 = dist2;
        farthest = vec;
      }
    }

    const double farthestDist = std::sqrt(farthestDist2);
    if (farthestDist <= tol)
      return false;

    // Find largest triangle spanned by origin, the vertex farthest from it and a third one
    Vector3d triangle;
    double triangleLength = 0.;
    for (const Point3d& p : points)
    {
      const Vector3d c = cross(farthest, p - origin);
      const double len = length(c);
      if (len > triangleLength)
      {
        triangleLength = len;
        triangle = c;
      }
    }

    // Calculate distance of a third triangle vertex to the line (origin, farthest).
    if (triangleLength <= tol * farthestDist)
      return false;

    const Vector3d normal = triangle * (1. / triangleLength);
    const Vector3d inPlane = farthest - normal * dot(farthest, normal);
    plane.origin = origin;
    plane.normal = normal;
    plane.u = normalize(inPlane);
    plane.v = cross(normal, plane.u);
    return true;
  }

  // Determine if the query point lies on the linear polyline
  bool isOnPolyline(const std::vector<Point3d>& points, const Point3d& point, double tol)
  {
    const std::size_t n = points.size();
    const Vector3d p = point - points[0];
    for (std::size_t i = 0; i < n; ++i)
    {
      const Vector3d a = points[i] - points[0];
      const Vector3d b = points[(i + 1) % n] - points[0];
      if (calculateDistanceToSegmentSquared(p, a, b) <= tol * tol)
        return true;
    }
    return false;
  }

  PointLocation classifyInPlane(const std::vector<Point3d>& points, const Plane& plane, const Vector2d& pointOnPlane, double tolerance)
  {
    const double tol2 = tolerance * tolerance;
    long long winding = 0;

    Vector2d a = plane.project(points.back());
    for (const Point3d& vertex : points)
    {
      const Vector2d b = plane.project(vertex);

      if (calculateDistanceToSegmentSquared(pointOnPlane, a, b) <= tol2)
        return PointLocation::Boundary;

      Vector2d segmentVec = b - a;
      Vector2d targetVec = pointOnPlane - a;
      if (a.y <= pointOnPlane.y && pointOnPlane.y < b.y)
      {
        if (cross(segmentVec, targetVec) > 0.) // upward edge, point on its left
          ++winding;
      }
      else if (b.y <= pointOnPlane.y && pointOnPlane.y < a.y)
      {
        if (cross(segmentVec, targetVec) < 0.) // downward edge, point on its right
          --winding;
      }
      a = b;
    }

    return winding != 0 ? PointLocation::Inside : PointLocation::Outside;
  }

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

  Plane plane;
  if (!tryComputePlane(polygonPoints, tol, plane))
  {
    // Degenerate polygon (a point or a line)
    return isOnPolyline(polygonPoints, point, tol) ? PointLocation::Boundary : PointLocation::Outside;
  }

  return classifyInPlane(polygonPoints, plane, plane.project(point), tol);
}
