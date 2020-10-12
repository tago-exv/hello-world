#pragma once
#include <iostream>
#include "RopeType.h"
#include "Wave.h"
#include "Centerline.h"
#include "Outline.h"

namespace exv {
	namespace rope {

		struct measure {
			Line centerline;
			Line waveLength;
			Line diameter;
			Line topRadius;
			Line bottomRadius;
// compatible for old version
			Line topWavelength;
			Line topRadius0;
			Line bottomWavelength;
			Line bottomRadius0;
			void swapXY() { centerline.swapXY(); waveLength.swapXY(); diameter.swapXY(); topRadius.swapXY(); bottomRadius.swapXY();
			topWavelength.swapXY(); topRadius0.swapXY(); bottomWavelength.swapXY(); bottomRadius0.swapXY();
			}
		};

		struct RopeCycle {
			Cycle top;
			Cycle bottom;
			Cycle whole;
		};

		class Rope
		{
		public:
			enum class Result {
				Ok = 0,
				Ng = 1
			};

		private:
			Outline topOutline;
			Outline bottomOutline;
			Centerline centerline;
			Wave topWave;
			Wave bottomWave;
			Wave wholeWave;
			RopeCycle cycle;
			double_t cleanRatio;
			int32_t specCycle;

		public:
			// Methods
			Result create(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, double_t cleanRatio);
			Result analyze(measure& output, const int32_t specCycle, double_t cleanRatio, const double_t measurePressRatio);

			void print(std::ostream& out);
		};


	}
}

