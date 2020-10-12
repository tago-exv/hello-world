#include "RopeType.h"

namespace exv {
	namespace rope {
        Line exv::rope::Line::move(Point p)
        {
            return Line(start + p, end + p);
        }

        Line exv::rope::Line::moveTo(Point p)
        {
            return Line(p, p + end - start);
        }
        Line exv::rope::Line::perpendicular(Point p)
		{
            double_t a;
            exv::rope::Point foot;

            if (end.y == start.y) {
                return Line(Point(p.x, start.y), p);
            }
            else if (end.x == start.x) {
                return Line(Point(start.x, p.y), p);

            }
            else {
                a = (end.y - start.y) / (end.x - start.x);
                foot.x = (a * start.x - start.y + p.x / a + p.y) / (a + 1 / a);
                foot.y = a * foot.x - a * start.x + start.y;
                return Line(foot, p);
            }
		}

	}
}