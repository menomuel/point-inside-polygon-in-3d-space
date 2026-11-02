#include "mini_test.h"
#include "test_utils.h"

TEST_CASE(squareInsideOutside)
{
  const auto square = unitSquare();
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.5)), Inside);
  CHECK_EQ(pointInPolygon(square, pointXY(0.001, 0.999)), Inside);
  CHECK_EQ(pointInPolygon(square, pointXY(1.5, 0.5)), Outside);
  CHECK_EQ(pointInPolygon(square, pointXY(-0.5, 0.5)), Outside);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, -0.001)), Outside);
  CHECK_EQ(pointInPolygon(square, pointXY(5., 5.)), Outside);
}

TEST_CASE(squareBoundary)
{
  const auto square = unitSquare();
  CHECK_EQ(pointInPolygon(square, pointXY(0., 0.)), Boundary);
  CHECK_EQ(pointInPolygon(square, pointXY(1., 1.)), Boundary);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.)), Boundary);
  CHECK_EQ(pointInPolygon(square, pointXY(1., 0.3)), Boundary);
  CHECK_EQ(pointInPolygon(square, pointXY(0., 0.7)), Boundary);
}

TEST_CASE(orientationDoesNotMatter)
{
  const auto clockwise = polygonXY({ { 0, 0 }, { 0, 1 }, { 1, 1 }, { 1, 0 } });
  CHECK_EQ(pointInPolygon(clockwise, pointXY(0.5, 0.5)), Inside);
  CHECK_EQ(pointInPolygon(clockwise, pointXY(1.5, 0.5)), Outside);
  CHECK_EQ(pointInPolygon(clockwise, pointXY(0.5, 1.)), Boundary);
}

TEST_CASE(rayThroughVertices)
{
  // The ray from the query point passes exactly through polygon vertices
  const auto diamond = polygonXY({ { 0, -1 }, { 1, 0 }, { 0, 1 }, { -1, 0 } });
  CHECK_EQ(pointInPolygon(diamond, pointXY(-0.5, 0.)), Inside);
  CHECK_EQ(pointInPolygon(diamond, pointXY(-2., 0.)), Outside);
  CHECK_EQ(pointInPolygon(diamond, pointXY(2., 0.)), Outside);
  CHECK_EQ(pointInPolygon(diamond, pointXY(0., 0.)), Inside);
  CHECK_EQ(pointInPolygon(diamond, pointXY(1., 0.)), Boundary);
}

TEST_CASE(concaveUShape)
{
  const auto u = uShape();
  CHECK_EQ(pointInPolygon(u, pointXY(0.5, 2.)), Inside);  // left part
  CHECK_EQ(pointInPolygon(u, pointXY(2.5, 2.)), Inside);  // right part
  CHECK_EQ(pointInPolygon(u, pointXY(1.5, 0.5)), Inside); // bottom
  CHECK_EQ(pointInPolygon(u, pointXY(1.5, 2.)), Outside); // inside the cut
  CHECK_EQ(pointInPolygon(u, pointXY(1.5, 1.)), Boundary);
  CHECK_EQ(pointInPolygon(u, pointXY(2., 2.)), Boundary);
  // Rays along the horizontal edges of the notch.
  CHECK_EQ(pointInPolygon(u, pointXY(0.5, 1.)), Inside);
  CHECK_EQ(pointInPolygon(u, pointXY(0.5, 3.)), Boundary);
  CHECK_EQ(pointInPolygon(u, pointXY(-1., 1.)), Outside);
  CHECK_EQ(pointInPolygon(u, pointXY(-1., 3.)), Outside);
}

TEST_CASE(redundantVertices)
{
  // Closing vertex repeated, a duplicated vertex and a collinear vertex in the middle of an edge
  const auto square = polygonXY({ { 0, 0 }, { 0.5, 0 }, { 1, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 } });
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.5)), Inside);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.)), Boundary);
  CHECK_EQ(pointInPolygon(square, pointXY(1.5, 0.)), Outside);
  CHECK_EQ(pointInPolygon(square, pointXY(-0.5, 0.)), Outside);
}

TEST_CASE(pointOffThePlane)
{
  const auto square = unitSquare();
  CHECK_EQ(pointInPolygon(square, { 0.5, 0.5, 1e-3 }), Outside);
  CHECK_EQ(pointInPolygon(square, { 0.5, 0.5, -1e-3 }), Outside);
  CHECK_EQ(pointInPolygon(square, { 0.5, 0., 1e-3 }), Outside);
  // Within the default tolerance (1e-9 of the size) the point is treated as lying in the plane.
  CHECK_EQ(pointInPolygon(square, { 0.5, 0.5, 1e-12 }), Inside);
  CHECK_EQ(pointInPolygon(square, { 0.5, 0., 1e-12 }), Boundary);
}

TEST_CASE(tiltedPlane)
{
  // Triangle in the plane x + y + z = 1.
  const std::vector<Point3d> triangle{ { 1, 0, 0 }, { 0, 1, 0 }, { 0, 0, 1 } };
  const double third = 1. / 3.;
  CHECK_EQ(pointInPolygon(triangle, { third, third, third }), Inside);
  CHECK_EQ(pointInPolygon(triangle, { 0.5, 0.5, 0. }), Boundary);
  CHECK_EQ(pointInPolygon(triangle, { 0., 0., 1. }), Boundary);
  CHECK_EQ(pointInPolygon(triangle, { 0.6, 0.6, -0.2 }), Outside); // in the plane, outside
  CHECK_EQ(pointInPolygon(triangle, { 0.3, 0.3, 0.3 }), Outside);  // inside in projection, off plane
}

TEST_CASE(explicitTolerance)
{
  const auto square = unitSquare();
  Options options { 0.1 };
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.05), options), Boundary);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, -0.05), options), Boundary);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.15), options), Inside);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, -0.15), options), Outside);
  CHECK_EQ(pointInPolygon(square, { 0.5, 0.5, 0.05 }, options), Inside);
  CHECK_EQ(pointInPolygon(square, { 0.5, 0.5, 0.15 }, options), Outside);

  // Same points with default tolerance
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.05)), Inside);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, -0.05)), Outside);
}

TEST_CASE(defaultToleranceScalesWithSize)
{
  const auto tiny = polygonXY({ { 0, 0 }, { 1e-6, 0 }, { 1e-6, 1e-6 }, { 0, 1e-6 } });
  CHECK_EQ(pointInPolygon(tiny, pointXY(5e-7, 1e-12)), Inside);
  CHECK_EQ(pointInPolygon(tiny, pointXY(5e-7, -1e-12)), Outside);
  CHECK_EQ(pointInPolygon(tiny, pointXY(5e-7, 1e-17)), Boundary);
}
