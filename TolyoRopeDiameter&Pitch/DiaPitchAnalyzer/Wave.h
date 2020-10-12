#pragma once
#include <iostream>
#include "RopeType.h"
#include "Outline.h"

namespace exv{
	namespace rope {

		class Wave
		{
		public:
			// result of the member function
			enum class Result {
				Ok = 0,
				Ng = 1
			};

		private:
			Outline outline;
			Outline cleanOutline;
			Outline midFrqOutline;
			Outline lowFrqOutline;
			Outline smoothOutline;
			Outline oneWave;
			Cycle cycle;
			Outline::Autocorrelation correlation;
			Point peak;
			DataType waveAmplitude;
		public:
		// Constructor
			Wave() : outline(), cleanOutline(), lowFrqOutline(), midFrqOutline(), smoothOutline(), oneWave(),cycle(), correlation(), peak(), waveAmplitude() {};
			Wave(Outline& input) : outline(input), cleanOutline(), lowFrqOutline(), midFrqOutline(), smoothOutline(), oneWave(), cycle(),  correlation(), peak(), waveAmplitude() {};
		// Destructor
			~Wave() {};
		// Methods
			Result analyzeCycle(Cycle& ouput, const int32_t specCycle, const double_t cleanRatio);
			Result analyzeRadius(Point& output, int32_t idxCycle, const double_t measurePressRatio);
			Result analyzeRadius(Point& output, int32_t left, int32_t right, const double_t measurePressRatio);

			Result measureRadius(Point& output, const double_t measurePressRatio);
			Result measureWavelength(Line& output, const int32_t specCycle, const int32_t searchRange);

			void print(std::ostream& out);
		};

	}
}


