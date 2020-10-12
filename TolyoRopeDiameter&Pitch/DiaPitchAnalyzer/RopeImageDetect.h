#pragma once
#include <vector>
#include "ExImage.h"

// ‰æ‘œˆ—‚É‚æ‚é Outline ŒŸo‚ÌŠÖ”
int32_t imageDetectVerticalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue);
int32_t imageDetectHorizontalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue);
