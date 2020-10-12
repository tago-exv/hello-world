#pragma once
#include <iostream>
#include <vector>

namespace exv
{
	namespace rope {

		typedef double_t DataType;	// elementary data type
		const DataType DATA_MAX = DBL_MAX;
		const DataType DATA_MIN = DBL_MIN;
		const DataType INVALID_VALUE = -1024;

		class Outline
		{
		public:
			// result of the member function
			enum class Result {
				Ok = 0,
				Ng = 1
			};


			struct Histogram {
				int32_t totalFrqs;
				std::vector<DataType> bins;
				std::vector<int32_t> frqs;
			};

			struct Maxmin {
				DataType max;
				DataType min;
				int32_t idxMax;
				int32_t idxMin;
			};

			class Autocorrelation {
			public:
				int32_t idxMin;
				double_t min;
				std::vector<double_t> score;

			public:
				Autocorrelation() {};
				Autocorrelation(int32_t size) : score(size), idxMin(0), min(0){};
				Result bestSore(double_t invalidValue = static_cast<double_t>(INVALID_VALUE));

				void print(const char* title, std::ostream& out);
			};



		public:
			std::vector<DataType> data;	// body of outline data
		
		// Constructor
			Outline() : data() {};
			Outline(int32_t size) : data(size) {};
			Outline(int32_t size, DataType initialValue) : data(size, initialValue) {};
			Outline(std::vector<double_t> data) : data(data) {};
		// Destructor
			~Outline() {};
		// Methods
			inline size_t size() {return data.size();} // return the number of date
			Result clean(Outline& output, const double_t rejectRatio, const int32_t division = 20, const DataType invalidValue = INVALID_VALUE);
			Result smooth(Outline& output, const int32_t period, const DataType invalidValue = INVALID_VALUE);
			Result subtract(Outline& output, Outline& input, const DataType invalidValue = INVALID_VALUE);
			Result cutOff(Outline& output, const int32_t start, const int32_t length);
			Result autocorrelation(Autocorrelation& output, const int32_t refStart, const int32_t refLen, const int32_t searchStart, const int32_t searchLen, const DataType invalidValue = INVALID_VALUE);
			Result histogram(Histogram& output, const int32_t bins = 10, const DataType invalidValue = INVALID_VALUE);
			Result histogram(Histogram& output, std::vector<DataType>& bins, const DataType invalidValue = INVALID_VALUE);
			Result maxmin(Maxmin& output, const DataType invalidValue = INVALID_VALUE);
			Result average(DataType& output, const DataType invalidValue = INVALID_VALUE);

			void print(const char* title, std::ostream &out);

		private:
			Result evalDifference(double_t& output, DataType* p_ptn, DataType* p_obj, const int32_t len, const DataType invalidValue = INVALID_VALUE);

		};
	}
}


