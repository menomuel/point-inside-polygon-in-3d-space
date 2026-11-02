#include "pip/point_in_polygon.h"

#include <algorithm>
#include <cmath>
#include <cstddef>
#include <cstdint>
#include <optional>

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

  std::optional<Plane> tryComputePlane(const std::vector<Point3d>& points, double tol)
  {
    if (points.empty())
      return std::nullopt;

    const std::size_t n = points.size();
    const Point3d& origin = points[0];

    // Calculate newell normal
    Vector3d newell;
    // Calculate point farthest from origin
    Vector3d farthest;
    double farthestDist2 = 0.;
    for (std::size_t i = 0; i < n; ++i)
    {
      const Vector3d a = points[i] - origin;
      const Vector3d b = points[(i + 1) % n] - origin;
      newell = newell + cross(a, b);

      const double dist2 = dot(a, a);
      if (dist2 > farthestDist2)
      {
        farthestDist2 = dist2;
        farthest = a;
      }
    }

    const double farthestDist = std::sqrt(farthestDist2);
    if (farthestDist <= tol)
      return std::nullopt;

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
      return std::nullopt;

    const double newellLength = length(newell);
    const Vector3d normal = newellLength > triangleLength ? normalize(newell) : normalize(triangle);

    Plane plane;
    const Vector3d inPlane = farthest - normal * dot(farthest, normal);
    plane.origin = origin;
    plane.normal = normal;
    plane.u = normalize(inPlane);
    plane.v = cross(normal, plane.u);
    return plane;
  }

  // Determine if the query point lies on the linear polyline
  bool isOnPolyline(const std::vector<Point3d>& points, const Point3d& point, double tol)
  {
    const std::size_t n = points.size();
    if (n == 0)
      return false;

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

  PointLocation classifyInPlane(const std::vector<Point3d>& points, const Plane& plane, const Vector2d& pointOnPlane, double tol, FillRule fillRule)
  {
    const double tol2 = tol * tol;
    std::int64_t winding = 0;

    Vector2d a = plane.project(points.back());
    for (const Point3d& vertex : points)
    {
      const Vector2d b = plane.project(vertex);

      if (calculateDistanceToSegmentSquared(pointOnPlane, a, b) <= tol2)
        return PointLocation::Boundary;

      Vector2d segmentVec = b - a;
      Vector2d targetVec = pointOnPlane - a;
      if (a.y <= pointOnPlane.y && pointOnPlane.y < b.y && cross(segmentVec, targetVec) > 0.) // upward edge, point on its left
      {
        ++winding;
      }
      else if (b.y <= pointOnPlane.y && pointOnPlane.y < a.y && cross(segmentVec, targetVec) < 0.) // downward edge, point on its right
      {
        --winding;
      }
      a = b;
    }

    const bool inside = fillRule == FillRule::EvenOdd ? (winding % 2 != 0) : (winding != 0);
    return inside ? PointLocation::Inside : PointLocation::Outside;
  }

} // namespace

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point) noexcept
{
  return pointInPolygon(polygonPoints, point, Options{});
}

PointLocation pointInPolygon(const std::vector<Point3d>& polygonPoints, const Point3d& point, Options options) noexcept
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

  const bool userTolerance = std::isfinite(options.tolerance) && options.tolerance > 0.;
  const double tol = userTolerance ? options.tolerance : RelativeTolerance * box.diagonal();

  if (!box.contains(point, tol))
    return PointLocation::Outside;

  const std::optional<Plane> plane = tryComputePlane(polygonPoints, tol);
  if (!plane)
  {
    // Degenerate polygon (a point or a line)
    return isOnPolyline(polygonPoints, point, tol) ? PointLocation::Boundary : PointLocation::Outside;
  }

  if (std::abs(plane->calculateSignedDistance(point)) > tol)
    return PointLocation::Outside;

  return classifyInPlane(polygonPoints, *plane, plane->project(point), tol, options.fillRule);
}
