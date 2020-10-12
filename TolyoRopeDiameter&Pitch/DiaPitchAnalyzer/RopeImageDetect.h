#pragma once
#include <vector>
#include "ExImage.h"

// �摜�����ɂ�� Outline ���o�̊֐�
int32_t imageDetectVerticalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue);
int32_t imageDetectHorizontalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue);
