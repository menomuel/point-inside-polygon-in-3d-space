#include "mini_test.h"
#include "test_utils.h"

#include <cmath>

namespace
{

  Options nonZero()
  {
    Options options;
    options.fillRule = FillRule::NonZero;
    return options;
  }

} // namespace

TEST_CASE(pentagramEvenOdd)
{
  const auto star = pentagram();
  CHECK_EQ(pointInPolygon(star, pointXY(0., 0.)), Outside); // centre is crossed twice
  CHECK_EQ(pointInPolygon(star, pointXY(0., 0.7)), Inside); // top spike
  CHECK_EQ(pointInPolygon(star, pointXY(0., -0.6)), Outside);
  CHECK_EQ(pointInPolygon(star, pointXY(0.9, 0.9)), Outside);
}

TEST_CASE(pentagramNonZero)
{
  const auto star = pentagram();
  CHECK_EQ(pointInPolygon(star, pointXY(0., 0.), nonZero()), Inside); // winding number 2
  CHECK_EQ(pointInPolygon(star, pointXY(0., 0.7), nonZero()), Inside);
  CHECK_EQ(pointInPolygon(star, pointXY(0., -0.6), nonZero()), Outside);
}

TEST_CASE(pentagramBoundary)
{
  const auto star = pentagram();
  const double pi = std::acos(-1.);
  // Points where edges cross each other: vertices of the inner pentagon.
  const double innerRadius = std::cos(2. * pi / 5.) / std::cos(pi / 5.);
  for (int i = 0; i < 5; ++i)
  {
    const double angle = pi / 2. + pi / 5. + 2. * pi * i / 5.;
    const Point3d crossing = pointXY(innerRadius * std::cos(angle), innerRadius * std::sin(angle));
    CHECK_EQ(pointInPolygon(star, crossing), Boundary);
    CHECK_EQ(pointInPolygon(star, crossing, nonZero()), Boundary);
  }
  CHECK_EQ(pointInPolygon(star, star[0]), Boundary);
  CHECK_EQ(pointInPolygon(star, pointXY(0., std::sin(pi / 10.))), Boundary); // middle of an edge
}

TEST_CASE(bowTieLobes)
{
  // The two lobes have opposite orientation, so their signed areas cancel (Newell normal = 0).
  const auto tie = bowTie();
  CHECK_EQ(pointInPolygon(tie, pointXY(0.2, 1.)), Inside);
  CHECK_EQ(pointInPolygon(tie, pointXY(1.8, 1.)), Inside);
  CHECK_EQ(pointInPolygon(tie, pointXY(1., 0.2)), Outside);
  CHECK_EQ(pointInPolygon(tie, pointXY(1., 1.8)), Outside);
  CHECK_EQ(pointInPolygon(tie, pointXY(1., 1.)), Boundary); // self-intersection
  CHECK_EQ(pointInPolygon(tie, pointXY(1.8, 1.), nonZero()), Inside);
  CHECK_EQ(pointInPolygon(tie, { 0.2, 1., 0.1 }), Outside);
}

TEST_CASE(squareTracedTwice)
{
  // Every interior point has winding number 2: the rules disagree everywhere inside.
  const auto twice =
    polygonXY({ { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 }, { 0, 0 }, { 1, 0 }, { 1, 1 }, { 0, 1 } });
  CHECK_EQ(pointInPolygon(twice, pointXY(0.5, 0.5)), Outside);
  CHECK_EQ(pointInPolygon(twice, pointXY(0.5, 0.5), nonZero()), Inside);
  CHECK_EQ(pointInPolygon(twice, pointXY(0.5, 1.)), Boundary);
  CHECK_EQ(pointInPolygon(twice, pointXY(1.5, 0.5), nonZero()), Outside);
}

TEST_CASE(overlappingEdges)
{
  // Square with a zero-width spike: the edge (2,1)-(3,1) is traversed forth and back.
  const auto spiked = polygonXY({ { 0, 0 }, { 2, 0 }, { 2, 1 }, { 3, 1 }, { 2, 1 }, { 2, 2 }, { 0, 2 } });
  CHECK_EQ(pointInPolygon(spiked, pointXY(1., 1.)), Inside); // ray runs along the spike
  CHECK_EQ(pointInPolygon(spiked, pointXY(2.5, 1.)), Boundary);
  CHECK_EQ(pointInPolygon(spiked, pointXY(3., 1.)), Boundary);
  CHECK_EQ(pointInPolygon(spiked, pointXY(2.5, 1.5)), Outside);
  CHECK_EQ(pointInPolygon(spiked, pointXY(2.5, 0.5)), Outside);
}

TEST_CASE(polygonWithHoleViaBridge)
{
  // Outer square 0..4 and inner square 1..3 traversed in the same direction. Under even-odd the inner square is a hole.
  const auto withHole = polygonXY(
    { { 0, 0 }, { 4, 0 }, { 4, 4 }, { 0, 4 }, { 0, 0 }, { 1, 1 }, { 3, 1 }, { 3, 3 }, { 1, 3 }, { 1, 1 } });
  CHECK_EQ(pointInPolygon(withHole, pointXY(2., 2.)), Outside);
  CHECK_EQ(pointInPolygon(withHole, pointXY(2., 2.), nonZero()), Inside);
  CHECK_EQ(pointInPolygon(withHole, pointXY(0.5, 2.)), Inside);
  CHECK_EQ(pointInPolygon(withHole, pointXY(3.5, 3.5)), Inside);
  CHECK_EQ(pointInPolygon(withHole, pointXY(2., 3.)), Boundary);
}
