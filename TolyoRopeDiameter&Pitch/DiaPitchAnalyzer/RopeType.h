#pragma once
#include <cstdint>
#include <cmath>

namespace exv {
	namespace rope {


		struct Point {
			double_t x, y;

			Point() :x(), y() {}
			Point(double_t x, double_t y) :x(x), y(y) {}
			void swapXY() { double_t xx = x; x = y; y = xx; }
			Point operator+(const Point& obj) { return Point(this->x + obj.x, this->y + obj.y); }
			Point operator-(const Point& obj) { return Point(this->x - obj.x, this->y - obj.y); }
			double_t length() {	return sqrt(x * x + y * y);}
		};

		struct Line {
			Point start, end;
			Line() : start(), end() {}
			Line(Point start, Point end) : start(start), end(end) {}

			double_t length() {
				double_t x = end.x - start.x;
				double_t y = end.y - start.y;
				return sqrt(x * x + y * y);
			}
			void swapXY(){start.swapXY(); end.swapXY();}

			double_t angle() {
				double_t x = end.x - start.x;
				double_t y = end.y - start.y;
				return y / x;
			}
			Line move(Point p);
			Line moveTo(Point p);
			Line perpendicular(Point p);
		};

		const size_t MAX_NUM_CYCLE = 3;

		struct Cycle {
			size_t num;
			Line cycle[MAX_NUM_CYCLE];
		};
	}
}
