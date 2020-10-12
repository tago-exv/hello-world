#pragma once
#include <vector>
#include "ExImage.h"


namespace exv
{
	namespace rope {

#define INVALID_VALUE -1024

		struct Vector {
			ExPoint start;
			ExPoint end;
		};

		struct Histogram {
			int32_t totalFrqs;
			std::vector<int32_t> bins;
			std::vector<int32_t> frqs;
		};

		struct Maxmin {
			int32_t max;
			int32_t min;
			int32_t idxMax;
			int32_t idxMin;
		};

		struct d_Maxmin {
			double_t max;
			double_t min;
			int32_t idxMax;
			int32_t idxMin;
		};


		struct Cycle {
			int32_t cycle;
			std::vector<int32_t> pos;
			std::vector<double_t> score;
		};

		struct LineSegment {
			int32_t start;
			int32_t len;
		};

		struct Outlines {
		public:
			std::vector<int32_t> topOutline;
			std::vector<int32_t> bottomOutline;
			std::vector<int32_t> centerLine;

			Outlines(int32_t length, int32_t value) {
				topOutline = std::vector<int32_t>(length, value);
				bottomOutline = std::vector<int32_t>(length, value);
				centerLine = std::vector<int32_t>(length, value);
			}
		};

		struct LinearEquation {
			double_t a, b;
		};

		class RopeAnalysis {
		public:
			RopeAnalysis(void);
			~RopeAnalysis(void);
			void createOutline(Outlines& outlines, const ExImage& grayImage, ExRegion roi, uint8_t binarizeThreshold);
			void createOutline(std::vector<int32_t>& output, ExImage& image, ExRegion& roi);
			void cleanOutline(std::vector<int32_t>& output, std::vector<int32_t>& input, double_t rejectRatio, int32_t division = 20, int32_t invalidValue = INVALID_VALUE);
			void LeastSquare(LinearEquation& output, std::vector<int32_t>& input, int32_t invalidValue = INVALID_VALUE);
			void getCoarseCycle(LineSegment& coarseCycle, std::vector<int32_t>& Outline, int32_t specCycle, bool expand = false, int32_t invalidValue = INVALID_VALUE);
			void expandCoarseCycle(LineSegment& output, std::vector<int32_t>& Outline, LineSegment input, int32_t invalidValue = INVALID_VALUE);
			void measureOutline(int32_t& output, std::vector<int32_t>& input, LineSegment coarseCycle, double_t measurePressRatio, int32_t invalidValue = INVALID_VALUE);
			void measureCycle(Cycle& output, std::vector<int32_t>& input, int32_t ref_start, int32_t ref_len, int32_t eval_start, int32_t eval_len);
			void analyPitch(double_t& pitch, std::vector<int32_t>& outScore, std::vector<int32_t>& input, double_t patternLength, double_t stdPitch, double_t searchRange, int32_t invalidValue = INVALID_VALUE);
			void printLineData(const char* title, std::vector<int32_t>& lineData);
			void printLineData(const char* title, std::vector<double_t>& lineData);

		private:
			void histogram(Histogram& output, std::vector<int32_t>& input, int32_t bins = 10, int32_t invalidValue = INVALID_VALUE);
			void histogram(Histogram& output, std::vector<int32_t>& input, std::vector<int32_t>& bins, int32_t invalidValue = INVALID_VALUE);
			void maxmin(Maxmin& maxmin, std::vector<int32_t>& input, int32_t invalidValue = INVALID_VALUE);
			void maxmin(d_Maxmin& maxmin, std::vector<double_t>& input, double_t invalidValue = INVALID_VALUE);
			int32_t getAverage(std::vector<int32_t>& outline, int32_t invalidValue = INVALID_VALUE);
			double_t evalDifference(int32_t* p_ptn, int32_t* p_obj, int32_t len, int32_t invalidValue = INVALID_VALUE);
			void movingAverage(std::vector<int32_t>& output, std::vector<int32_t>& input, int32_t period, int32_t invalidValue = INVALID_VALUE);
			void getDifferential(std::vector<int32_t>& output, std::vector<int32_t>& input, int32_t period, int32_t invalidValue = INVALID_VALUE);
		};
	}
}