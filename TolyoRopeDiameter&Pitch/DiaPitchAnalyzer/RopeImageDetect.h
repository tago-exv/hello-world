#pragma once
#include <vector>
#include "ExImage.h"

// 画像処理による Outline 検出の関数
int32_t imageDetectVerticalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue);
int32_t imageDetectHorizontalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue);
