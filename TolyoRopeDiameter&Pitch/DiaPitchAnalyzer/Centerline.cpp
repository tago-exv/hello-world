#include <math.h>
#include "Centerline.h"
#include "Outline.h"

using namespace exv;
using namespace rope;

Centerline::Result Centerline::create(Outline& topOutline, Outline& bottomOutline, const double_t cleanRatio)
{
	size = topOutline.size();  // save the size in the member "size"
	Outline::Result outlineResult;

	// working outline
	Outline centerline(size);

	// Get center line

	auto ptrTop = &topOutline.data[0];
	auto ptrBottom = &bottomOutline.data[0];
	auto ptrCenter = &centerline.data[0];
	for (std::int32_t x = 0; x < size; ++x) {
		if (*ptrTop != INVALID_VALUE && *ptrBottom != INVALID_VALUE) {
			*ptrCenter = (*ptrTop + *ptrBottom) / 2;
		}
		else {
			*ptrCenter = INVALID_VALUE;
		}
		ptrTop++;
		ptrBottom++;
		ptrCenter++;
	}

	// 
	// 中心線から外れ値を除去する
	// Reject outlier values in the centerline

	// working outline
	Outline cleanCenterline;
	outlineResult = centerline.clean(cleanCenterline, cleanRatio);
	if (outlineResult != Outline::Result::Ok) {
		return Centerline::Result::Ng;
	}

	// 中心線の１次式を、最小二乗法で求める。結果は member "equation" に保存
	// Get the linear equation of cleanCenterline and keep in the memmber "equation"
	Centerline::Result centerlinResult;
	centerlinResult = leastSquare(equation, cleanCenterline);
	if (centerlinResult != Centerline::Result::Ok) {
		return Centerline::Result::Ng;
	}

	return Centerline::Result::Ok;
}

Centerline::Result exv::rope::Centerline::get(Line& output)
{
	output.start.x = 0;
	output.start.y = equation.substitute(0);
	output.end.x = size;
	output.end.y = equation.substitute(size);

	return Result();
}

Centerline::Result Centerline::subtract(Outline& output, Outline& input)
{
	int32_t size = input.data.size();
	auto ptrInput = &input.data[0];

	Outline workLine(size);
	auto ptrWork = &workLine.data[0];


	for (int32_t i = 0; i < size; i++) {
		if (*ptrInput == INVALID_VALUE) {
			*ptrWork = INVALID_VALUE;
		}
		else {
			*ptrWork = *ptrInput - equation.substitute(i);
		}
		ptrInput++, ptrWork++;
	}
	output = workLine;
	return Centerline::Result::Ok;
}

Centerline::Result Centerline::subtractMinus(Outline& output, Outline& input)
{
	int32_t size = input.data.size();
	auto ptrInput = &input.data[0];

	Outline workLine(size);
	auto ptrWork = &workLine.data[0];


	for (int32_t i = 0; i < size; i++) {
		if (*ptrInput == INVALID_VALUE) {
			*ptrWork = INVALID_VALUE;
		}
		else {
			*ptrWork = equation.substitute(i) - *ptrInput;
		}
		ptrInput++, ptrWork++;
	}
	output = workLine;
	return Centerline::Result::Ok;
}

Centerline::Result Centerline::add(Point& output, Point& input)
{
	output.x = input.x;
	output.y = input.y + equation.substitute(input.x);
	return Centerline::Result::Ok;
}

Centerline::Result Centerline::addMinus(Point& output, Point& input)
{
	output.x = input.x;
	output.y = - input.y + equation.substitute(input.x);
	return Centerline::Result::Ok;
}

Centerline::Result Centerline::add(Line& output, Line& input)
{
	add(output.start, input.start);
	add(output.end, input.end);
	return Centerline::Result::Ok;
}

Centerline::Result Centerline::addMinus(Line& output, Line& input)
{
	addMinus(output.start, input.start);
	addMinus(output.end, input.end);
	return Centerline::Result::Ok;
}

Centerline::Result Centerline::distance(Line& output, Point& input)
{
	double_t x0 = (input.x + equation.a * (input.y - equation.b)) / (equation.a * equation.a + 1);
	double_t y0 = equation.a * x0 + equation.b;
	output.start.x = x0;
	output.start.y = y0;
	output.end = input;

	return Centerline::Result::Ok;
}

Centerline::Result Centerline::angleTransfer(Point& output, Point& input)
{
	output.x = input.x;
	output.y = input.y + equation.a * input.x;
	return Centerline::Result::Ok;
}


//  最小二乗法で直線の式を求める
//  ｙ＝ ax ＋ b としたとき
//  a = { n * Σ(xy) - Σ(x)Σ(y) } / { n* Σ(x**2) - (Σ(x)) **2}
//  b = { Σ(x**2) * Σ(y) - Σ(xy)Σ(x) } / { n* Σ(x**2) - (Σ(x))**2}

Centerline::Result Centerline::leastSquare(LinearEquation& output, Outline& input, DataType invalidValue)
{
	int32_t n = 0;
	double_t x = 0;
	double_t y = 0;
	double_t xx = 0;
	double_t xy = 0;

	auto ptr = &input.data[0];
	for (int32_t i = 0; i < input.size(); i++, ptr++) {
		if (*ptr == invalidValue) {
			continue;
		}
		n++;
		x += i;
		y += *ptr;
		xx += i * i;
		xy += i * *ptr;
	}
	if ((n * xx - x * x) == 0) {
		return Centerline::Result::Ng;
	}

	output.a = static_cast<double_t>(n * xy - x * y) / static_cast<double_t>(n * xx - x * x);
	output.b = static_cast<double_t>(xx * y - xy * x) / static_cast<double_t>(n * xx - x * x);
	return Centerline::Result::Ok;
}
