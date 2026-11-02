#include "mini_test.h"
#include "test_utils.h"

#include <limits>

TEST_CASE(emptyPolygon)
{
  CHECK_EQ(pointInPolygon({}, pointXY(0., 0.)), Outside);
}

TEST_CASE(singleVertex)
{
  const std::vector<Point3d> single{ { 1., 2., 3. } };
  CHECK_EQ(pointInPolygon(single, { 1., 2., 3. }), Boundary);
  CHECK_EQ(pointInPolygon(single, { 1., 2., 3.1 }), Outside);
}

TEST_CASE(allVerticesEqual)
{
  const std::vector<Point3d> same{ { 1., 1., 1. }, { 1., 1., 1. }, { 1., 1., 1. } };
  CHECK_EQ(pointInPolygon(same, { 1., 1., 1. }), Boundary);
  CHECK_EQ(pointInPolygon(same, { 0., 0., 0. }), Outside);
}

TEST_CASE(twoVertices)
{
  const std::vector<Point3d> segment{ { 0., 0., 0. }, { 2., 2., 2. } };
  CHECK_EQ(pointInPolygon(segment, { 1., 1., 1. }), Boundary);
  CHECK_EQ(pointInPolygon(segment, { 2., 2., 2. }), Boundary);
  CHECK_EQ(pointInPolygon(segment, { 1., 1., 1.1 }), Outside);
  CHECK_EQ(pointInPolygon(segment, { 3., 3., 3. }), Outside);
}

TEST_CASE(collinearVertices)
{
  // Zero-area polygon going back and forth along a line.
  const auto line = polygonXY({ { 0., 0. }, { 1., 1. }, { 3., 3. }, { 2., 2. } });
  CHECK_EQ(pointInPolygon(line, pointXY(0.5, 0.5)), Boundary);
  CHECK_EQ(pointInPolygon(line, pointXY(3., 3.)), Boundary);
  CHECK_EQ(pointInPolygon(line, pointXY(1., 1.1)), Outside);
  CHECK_EQ(pointInPolygon(line, pointXY(4., 4.)), Outside);
}

TEST_CASE(nearlyCollinearButValid)
{
  // A thin sliver, 1e-6 high: still a proper polygon at the default tolerance.
  const auto sliver = polygonXY({ { 0., 0. }, { 1., 0. }, { 0.5, 1e-6 } });
  CHECK_EQ(pointInPolygon(sliver, pointXY(0.5, 5e-7)), Inside);
  CHECK_EQ(pointInPolygon(sliver, pointXY(0.5, 2e-6)), Outside);
  CHECK_EQ(pointInPolygon(sliver, pointXY(0.5, 0.)), Boundary);
}

TEST_CASE(nonFiniteInput)
{
  constexpr const double nan = std::numeric_limits<double>::quiet_NaN();
  constexpr const double inf = std::numeric_limits<double>::infinity();
  const auto square = unitSquare();
  CHECK_EQ(pointInPolygon(square, { nan, 0.5, 0. }), Outside);
  CHECK_EQ(pointInPolygon(square, { 0.5, inf, 0. }), Outside);

  auto broken = unitSquare();
  broken[2].z = nan;
  CHECK_EQ(pointInPolygon(broken, pointXY(0.5, 0.5)), Outside);
  broken[2].z = -inf;
  CHECK_EQ(pointInPolygon(broken, pointXY(0.5, 0.5)), Outside);
}

TEST_CASE(invalidToleranceFallsBackToDefault)
{
  const auto square = unitSquare();
  double tolerance = std::numeric_limits<double>::quiet_NaN();
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.5), tolerance), Inside);
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.), tolerance), Boundary);
  tolerance = -1.;
  CHECK_EQ(pointInPolygon(square, pointXY(0.5, 0.01), tolerance), Inside);
}
