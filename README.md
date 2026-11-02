# Point in Polygon in 3D

A small library that classifies a query point relative to a polygon in 3D space as **Inside**, **Outside**, or **Boundary**.

The polygon is defined by an ordered sequence of 3D vertices. Consecutive vertices define polygon edges.

The library has no dependencies beyond the standard library and comes with its own unit tests.

## Usage

```cpp
#include <pip/point_in_polygon.h>

const std::vector<Point3d> square{ { 0., 0., 0. }, { 1., 0., 0. }, { 1., 1., 0. }, { 0., 1., 0. } };

pointInPolygon(square, { 0.5, 0.5, 0. });  // PointLocation::Inside
pointInPolygon(square, { 1.0, 0.3, 0. });  // PointLocation::Boundary
pointInPolygon(square, { 0.5, 0.5, 1. });  // PointLocation::Outside (not in the polygon's plane)
```

The second overload takes an `Options` struct for the less common cases:

```cpp
Options options;
options.tolerance = 1e-3;                 // absolute distance; the default scales with the polygon size
options.fillRule = FillRule::NonZero;     // only matters for self-intersecting polygons

pointInPolygon(polygon, point, options);
```

## Requirements

The polygon:
- is closed;
- is planar;
- may be concave;
- may contain self-intersections.

## Behaviour

- **Closing edge is implicit.** The last vertex connects back to the first. Repeating the first vertex at the end is allowed, and so are duplicate or collinear vertices.
- **Orientation doesn't matter.** Clockwise and counter-clockwise polygons give the same answers.
- **Tolerance.** A point counts as being in the polygon's plane, or on its boundary, if it is within `tolerance` of it. If no tolerance is given (or it is zero, negative or not finite), the default is `1e-9` times the diagonal of the polygon's bounding box.
- **Self-intersecting polygons.** The choice of fill rule decides what "inside" means for them:
  - `EvenOdd` (the default): a point is inside if a ray from it crosses the boundary an odd number of times. The centre of a pentagram is outside.
  - `NonZero`: a point is inside if the boundary winds around it at least once. The centre of a pentagram is inside.

  For simple polygons, the two rules give the same result.
- **Degenerate input.** If all the vertices coincide, or all lie on one line, the polygon is then treated as a polyline: points on it are `Boundary`, and everything else is `Outside`.
- **Invalid input.** An empty polygon, or any `NaN` or infinite coordinate in the polygon or the point, gives `Outside`.
- **No exceptions, no allocations.** Both functions are `noexcept` and run in O(n) time for n vertices.

## How it works

1. **Validate and quick reject.** Check the input for non-finite values and build the bounding box. A point outside the box (expanded by the tolerance) is rejected.
2. **Fit the plane.** Find the vertex farthest from the first vertex, and then the third vertex that makes the largest triangle with those two. If even that triangle is too thin to define a plane within the tolerance, the polygon is degenerate and the polyline check applies.

   The normal is the longer of two candidates: Newell’s normal, computed from all the edges, and the normal of the largest triangle. Newell's normal averages over the whole polygon, but is proportional to the signed area which cancels out in shapes like a bow tie. In that case the triangle normal is used instead.
3. **Check the distance to the plane.** If the point is farther than the tolerance from the plane, it's `Outside`.
4. **Project to 2D.** Both the polygon and the point are expressed in an orthonormal basis lying in the plane. The problem transforms into 2D.
5. **Classify.** One pass over the edges does two things:
   - **Boundary:** if the point is within the tolerance of any edge, it's on the boundary.
   - **Winding number:** otherwise, a winding number is accumulated using the standard crossing test along a horizontal ray. Edges are treated as half-open intervals in y. That way a ray passing exactly through a vertex, or running along a horizontal edge, is counted exactly once. The fill rule then turns the winding number into `Inside` or `Outside`.

## Project structure

```
include/pip/point_in_polygon.h   public API
src/point_in_polygon.cpp         implementation
src/vector_math.h                small internal 2D/3D vector helpers
tests/                           unit tests and the mini test framework
```

## Building and running the tests

CMake 3.16 or newer and a C++17 compiler.

```sh
cmake -S . -B build
cmake --build build --config Release
cd build
ctest -C Release --output-on-failure
```

Visual Studio can also open the folder directly as a CMake project (*File → Open → Folder*).

| CMake option | Default | Meaning |
|---|---|---|
| `PIP_BUILD_TESTS` | `ON` | Build the unit tests |
| `PIP_WARNINGS_AS_ERRORS` | `OFF` | Treat compiler warnings as errors (`/WX` or `-Werror`) |

## Tests

The tests use a tiny framework in `tests/mini_test.h` to avoid external dependencies.

The 25 test cases are split into three files:

- **`test_basic.cpp`:** simple cases.
  - Convex and concave polygons, both orientations, points on edges and at vertices.
  - Rays passing exactly through vertices and along horizontal edges.
  - Redundant vertices.
  - A tilted plane, points just on and just off the plane.
  - Explicit tolerances, and the default tolerance scaling with polygon size.
- **`test_degenerate.cpp`:** input without a proper plane.
  - An empty polygon, a single vertex, all vertices equal, and collinear vertices.
  - A very thin but still valid sliver.
  - `NaN` and infinite coordinates, and invalid tolerance values.
- **`test_selfIntersecting.cpp`:** the cases where the fill rules matter.
  - A pentagram under both rules, including points exactly where its edges cross.
  - A bow tie, a square traced twice, an edge traversed forward and back.
  - A polygon with a hole made by a bridge edge.
