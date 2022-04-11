#pragma once
#include <string>
#include "plc.h"
#include "MeasureDataFormatter.h"

namespace exv {
	class MeasureDataPlc
	{
		std::string addressData_;
		std::string addressReq_;
		std::string addressDone_;

	public:
		enum class Result {
		fail = -1,
		notData = 0,
		success = 1,
		on = 2,
		off = 3
		};

		MeasureDataPlc() : plc_(NULL) {};
		MeasureDataPlc(std::string iniFilePath, exv::Plc* plc, int cameraNo);

		Result checkPlc(std::string& resp);
		Result writeData(std::string& s);
		Result readData(std::string& s);
		Result setRequest();
		Result resetRequest();
		Result readRequest();
		Result setDone();
		Result resetDone();
		Result readDone();

		Plc* plc_;
	};
}