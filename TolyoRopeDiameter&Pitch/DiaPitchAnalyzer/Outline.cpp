#include <iostream>
#include "Outline.h"

using namespace exv;
using namespace rope;

Outline::Result Outline::clean(Outline& output, const double_t rejectRatio, const int32_t division, const DataType invalidValue)
{
	Histogram hist;
	histogram(hist, division, invalidValue);

	int32_t n_reject = size() * rejectRatio;
	DataType validMax, validMin;

	DataType cumul = 0;

	auto p_bins = &hist.bins[0];
	auto p_frqs = &hist.frqs[0];
	for (int32_t i = 0; i < hist.frqs.size(); i++) {
		cumul += *p_frqs;
		if (cumul > n_reject) {
			validMin = *p_bins;
			break;
		}
		p_bins++;
		p_frqs++;
	}

	cumul = 0;
	p_bins = &hist.bins[hist.bins.size() - 1];
	p_frqs = &hist.frqs[hist.frqs.size() - 1];
	for (int32_t i = 0; i < hist.frqs.size(); i++) {
		cumul += *p_frqs;
		if (cumul > n_reject) {
			validMax = *p_bins;
			break;
		}
		--p_bins;
		--p_frqs;
	}

	Outline tmp(size());
	auto ptrDst = &tmp.data[0];
	auto ptrSrc = &data[0];

	for (int32_t i = 0; i < size(); i++){
		if (*ptrSrc == invalidValue) {
			*ptrDst = invalidValue;
		}
		else if (*ptrSrc < validMin || *ptrSrc > validMax) {
			*ptrDst = invalidValue;
		}
		else {
			*ptrDst = *ptrSrc;
		}
		*ptrSrc++;
		*ptrDst++;
	}
	output = tmp;
	return Outline::Result::Ok;
}

Outline::Result Outline::smooth(Outline& output, const int32_t period, const DataType invalidValue)
{
	Outline tmp(size(), invalidValue);

	if (period > size()) {
		return Outline::Result::Ng;
	}

	auto ptrDst = &tmp.data[period / 2];

	auto ptrHead = &data[0];
	DataType sum = 0;
	int32_t num = 0;
	int32_t i = 0;
	for (; i < period; i++, ptrHead++) {
		if (*ptrHead == invalidValue) {
			continue;
		}
		sum += *ptrHead;
		num++;
	}
	*ptrDst++ = num != 0 ? sum / num : invalidValue;

	auto ptrTail = &data[0];
	for (; i < size(); i++, ptrHead++, ptrTail++) {
		if (*ptrHead != invalidValue) {
			sum += *ptrHead;
			num++;
		}
		if (*ptrTail != invalidValue) {
			sum -= *ptrTail;
			--num;
		}
		*ptrDst++ = num != 0 ? sum / num : invalidValue;
	}
	output = tmp;
	return Outline::Result::Ok;
}

Outline::Result exv::rope::Outline::subtract(Outline& output, Outline& input, const DataType invalidValue)
{
	Outline workLine(size(), invalidValue); // working instance
	auto ptrDst = &workLine.data[0];
	auto ptrSrc = &data[0];
	auto ptrSub = &input.data[0];

	for (int32_t i = 0; i < size(); i++) {
		if (*ptrSrc != invalidValue && *ptrSub != invalidValue) {
			*ptrDst = *ptrSrc - *ptrSub;
		}
		ptrSrc++, ptrSub++, ptrDst++;
	}
	output = workLine;
	return Outline::Result::Ok;
}

Outline::Result Outline::cutOff(Outline& output, const int32_t start, const int32_t length)
{
	Outline workLine(length); // working instance
	auto ptrDst = &workLine.data[0];
	auto ptrSrc = &data[start];
	for (int32_t i = 0; i < length; i++) {
		*ptrDst++ = *ptrSrc++;
	}
	output = workLine;
	return Outline::Result::Ok;
}

Outline::Result Outline::autocorrelation(Autocorrelation& output, const int32_t refStart, const int32_t refLen, const int32_t searchStart, const int32_t searchLen, const DataType invalidValue)
{
	Outline::Result result;
	Autocorrelation correlation(searchLen);  // working instance
	auto ptrScore = &correlation.score[0];

	auto ptrRef = &data[refStart];
	auto ptrSearch = &data[searchStart];
	for (int32_t i = 0; i < searchLen; i++) {
		double_t score;
		result = evalDifference(score, ptrRef, ptrSearch++, refLen, invalidValue);

		if (result == Outline::Result::Ok) {
			*ptrScore++ = score;
		}
		else {
			*ptrScore++ = static_cast<double_t>(invalidValue);
		}
	}

	result = correlation.bestSore();
	output = correlation;
	return result;
}

Outline::Result Outline::Autocorrelation::bestSore(double_t invalidValue) {
	double_t t_min = DBL_MAX;
	int32_t ix_min = 0;

	auto ptr = &score[0];
	for (int32_t i = 0; i < score.size(); i++, ptr++) {
		if (*ptr == invalidValue) {
			continue;
		}
		if (*ptr < t_min) {
			t_min = *ptr;
			ix_min = i;
		}
	}
	// save on the members this->min, this->idxMin
	min = t_min;
	idxMin = ix_min;
	return Outline::Result::Ok;
}

void exv::rope::Outline::Autocorrelation::print(const char* title, std::ostream& out)
{
	out << title;

	auto ptr = &score[0];
	for (int32_t i = 0; i < score.size(); i++, ptr++) {
		out << ", " << *ptr;
	}
	out << std::endl;

}


Outline::Result Outline::histogram(Histogram& output, const int32_t bins, const DataType invalidValue)
{
	Maxmin mxmn;
	maxmin(mxmn, invalidValue);

	std::vector<DataType> tmp_bins(bins + 1);
	auto itr = tmp_bins.begin();
	for (int32_t i = 0; i <= bins; i++) {
		*itr++ = mxmn.min + (mxmn.max - mxmn.min) * i / bins;
	}
	return histogram(output, tmp_bins, invalidValue);

}

Outline::Result Outline::histogram(Histogram& output, std::vector<DataType>& bins, const DataType invalidValue)
{
	int32_t n_bins = bins.size() - 1;
	std::vector<int32_t> tmp_frqs(n_bins, 0);
	int32_t total_frqs = 0;

	auto ptr = &data[0];
	for (int32_t j = 0; j < size(); j++,  ptr++) {
		if (*ptr == invalidValue) {
			continue;
		}
		for (int32_t i = 0; i < n_bins; i++) {
			if (*ptr <= bins[i + 1]) {
				tmp_frqs[i]++;
				break;
			}
		}
		total_frqs++;
	}
	output.bins = bins;
	output.frqs = tmp_frqs;
	output.totalFrqs = total_frqs;

	return Outline::Result::Ok;
}

Outline::Result Outline::maxmin(Maxmin& output, const DataType invalidValue)
{
	DataType t_max = DATA_MIN;
	DataType t_min = DATA_MAX;
	int32_t ix_max = 0;
	int32_t ix_min = 0;

	auto ptr = &data[0];
	for (int32_t i = 0; i < size(); i++, ptr++) {
		if (*ptr == invalidValue) {
			continue;
		}
		if (*ptr > t_max) {
			t_max = *ptr;
			ix_max = i;
		}
		else if (*ptr < t_min) {
			t_min = *ptr;
			ix_min = i;
		}
	}
	output.max = t_max;
	output.min = t_min;
	output.idxMax = ix_max;
	output.idxMin = ix_min;

	return Outline::Result::Ok;
}

Outline::Result Outline::average(DataType& output, const DataType invalidValue)
{
	DataType sum = 0;
	int32_t num = 0;

	auto ptr = &data[0];
	for (int32_t i = 0; i < size(); i++, ptr++) {
		if (*ptr == invalidValue) {
			continue;
		}
		sum += *ptr;
		num++;
	}
	if (num != 0) {
		output = sum / num;
		return Outline::Result::Ok;
	}
	else {
		output = invalidValue;
		return Outline::Result::Ng;
	}
}

void exv::rope::Outline::print(const char* title, std::ostream &out)
{
	out << title;

	auto ptr = &data[0];
	for (int32_t i = 0; i < size(); i++, ptr++) {
		out << ", " << *ptr;
	}
	out << std::endl;
}

Outline::Result Outline::evalDifference(double_t& output, DataType* p_ptn, DataType* p_obj, const int32_t len, const DataType invalidValue)
{
	DataType score = 0;
	int32_t n_valid = 0;
	for (int32_t i = 0; i < len; i++) {
		if (*p_obj != invalidValue && *p_ptn != invalidValue) {
			n_valid++;
			DataType dif = *p_obj - *p_ptn;
			if (dif >= 0) {
				score += dif;
			}
			else {
				score -= dif;
			}
		}
		p_ptn++;
		p_obj++;
	}
	if (n_valid == 0) {
		output = static_cast<double_t>(invalidValue);
		return Outline::Result::Ng;
	}
	else {
		output = static_cast<double_t>(score) / static_cast<double_t>(n_valid);
		return Outline::Result::Ok;
	}
}


