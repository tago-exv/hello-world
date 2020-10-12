#pragma once
#include <math.h>
#include "RopeType.h"
#include "Wave.h"
#include "Outline.h"

namespace exv {
	namespace rope {



		class Centerline
		{
		private:
			class LinearEquation {
			public:
				double_t a, b;

				DataType substitute(int32_t x) {
//					return static_cast<DataType>(round(a * x + b));
					return static_cast<DataType>(a * x + b);
				};
				double_t substitute(double_t x) {
					return (a * x + b);
				};
			};

			LinearEquation equation;
			int32_t size;

		public:
			// result of member functions
			enum class Result{
				Ok = 0,
				Ng = 1
			};

			// Constructor
			// Centerline() : a(0), b(0) {};
			// Destructor
			~Centerline() {};

			// Methods
			Result create(Outline& topOutline, Outline& bottomOutline, const double_t cleanRatio);
			Result get(Line& output);
			Result subtract(Outline& output, Outline& input);
			Result subtractMinus(Outline& output, Outline& input);
			Result add(Point& output, Point& input);
			Result addMinus(Point& output, Point& input);
			Result add(Line& output, Line& input);
			Result addMinus(Line& output, Line& input);
			Result distance(Line& output, Point& input);
			Result angleTransfer(Point& output, Point& input);
		private:
			Result leastSquare(LinearEquation& output, Outline& input, DataType invalidValue = INVALID_VALUE);
		};
	}
}

