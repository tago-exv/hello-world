#include "MeasureDataPlc.h"
#include "MeasureDataFormatter.h"
#include "MainUtility.h"

namespace exv {

	MeasureDataPlc::MeasureDataPlc(std::string iniFilePath, exv::Plc* plc, int cameraNo)
	{
		plc_ = plc;

		std::string sNumber = std::to_string(cameraNo);
		addressData_ = exv::GetConfigString(iniFilePath, "MeasureData", ("MeasureData" + sNumber).c_str());
		addressReq_ = exv::GetConfigString(iniFilePath, "MeasureData", ("MeasureRequest" + sNumber).c_str());
		addressDone_ = exv::GetConfigString(iniFilePath, "MeasureData", ("MeasureComplete" + sNumber).c_str());
	}

	MeasureDataPlc::Result MeasureDataPlc::checkPlc(std::string& resp)
	{
		if (plc_->checkPlc(resp) == Plc::Result::success) {
			return MeasureDataPlc::Result::success;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::writeData(std::string& s)
	{
		if (plc_->writeDataMemory(addressData_, s) == Plc::Result::success) {
			return MeasureDataPlc::Result::success;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::readData(std::string& s)
	{
		if (plc_->readDataMemory(addressData_, s) == Plc::Result::success) {
			return MeasureDataPlc::Result::success;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::setRequest()
	{
		if (plc_->writeRelay(addressReq_, true) == Plc::Result::success) {
			return MeasureDataPlc::Result::success;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::resetRequest()
	{
		if (plc_->writeRelay(addressReq_, false) == Plc::Result::success) {
			return MeasureDataPlc::Result::success;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::readRequest()
	{
		Plc::Result result = plc_->readRelay(addressReq_);
		if (result == Plc::Result:: on) {
			return MeasureDataPlc::Result::on;
		}
		else if (result == Plc::Result::off) {
			return MeasureDataPlc::Result::off;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::setDone()
	{
		if (plc_->writeRelay(addressDone_, true) == Plc::Result::success) {
			return MeasureDataPlc::Result::success;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::resetDone()
	{
		if (plc_->writeRelay(addressDone_, false) == Plc::Result::success) {
			return MeasureDataPlc::Result::success;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

	MeasureDataPlc::Result MeasureDataPlc::readDone()
	{
		Plc::Result result = plc_->readRelay(addressDone_);
		if (result == Plc::Result::on) {
			return MeasureDataPlc::Result::on;
		}
		else if (result == Plc::Result::off) {
			return MeasureDataPlc::Result::off;
		}
		else {
			return MeasureDataPlc::Result::fail;
		}
	}

}
