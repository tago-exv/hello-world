#include "RopeImageDetect.h"

// internal function
double_t centerGravity(std::vector<int32_t>& data, int32_t idx);

// 垂直方向のロープ境界点を検出する

int32_t imageDetectVerticalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue)
{
    std::int32_t left = roi.x;
    std::int32_t top = roi.y;
    std::int32_t width = roi.width;
    std::int32_t height = roi.height;

    topOutline = std::vector<double_t>(height, invalidValue);
    bottomOutline = std::vector<double_t>(height, invalidValue);

    std::int32_t stride = std::int32_t(image.getStride());
    std::uint8_t* grayPtr = static_cast<std::uint8_t*>(image.getImageDataPointer());

    // 
    int32_t max, min, idxMax, idxMin;
    std::vector<int32_t> diffs(width - diffPixes);
    int32_t* p_diff;

    std::uint8_t* p_thisPix;
    std::uint8_t* p_thisLine = grayPtr + (stride * top) + left;

    for (int32_t i = 0; i < height; i++) {
        p_thisPix = p_thisLine;
        p_diff = &diffs[0];

        max = min = 0;
        idxMax = idxMin = 0;
        for (int32_t j = 0; j < width - diffPixes; j++) {
            int32_t diff = *(p_thisPix + diffPixes) - *p_thisPix;
            if (diff > max) {
                max = diff;
                idxMax = j;
            }
            if (diff < min) {
                min = diff;
                idxMin = j;
            }
            *p_diff = diff;
            p_thisPix++;
            p_diff++;
        }
        topOutline[i] = centerGravity(diffs, idxMin) + diffPixes / 2;
        bottomOutline[i] = centerGravity(diffs, idxMax) + diffPixes / 2;

        p_thisLine += stride;
    }
    return int32_t();
}

// 水平方向のロープ境界点を検出する（重心を求める処理はまだ入っていない）

int32_t imageDetectHorizontalOutline(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, exv::ExImage& image, exv::ExRegion roi, int32_t diffPixes, double_t invalidValue)
{
    std::int32_t left = roi.x;
    std::int32_t top = roi.y;
    std::int32_t width = roi.width;
    std::int32_t height = roi.height;

    topOutline = std::vector<double_t>(width, invalidValue);
    bottomOutline = std::vector<double_t>(width, invalidValue);

    // 画像pixにアクセスするポインタ
    std::int32_t stride = std::int32_t(image.getStride());
    std::uint8_t* grayPtr = static_cast<std::uint8_t*>(image.getImageDataPointer());
    std::uint8_t* p_thisLine = grayPtr + (stride * top) + left;
    std::uint8_t* p_thisPix;
    int32_t pixOffset = stride * diffPixes;  // diffPixes 下の画素までのポインタオフセット

    // 各列　画素微分の最大値と最小値
    std::vector<int32_t> max(width, 0), min(width, 0);

    for (int32_t i = diffPixes / 2; i < (diffPixes / 2) + (height - diffPixes); i++) {
        p_thisPix = p_thisLine;

        for (int32_t j = 0; j < width; j++) {
            int32_t diff = *(p_thisPix + pixOffset) - *p_thisPix;
            if (diff > max[j]) {
                max[j] = diff;
                bottomOutline[j] = i;
            }
            if (diff < min[j]) {
                min[j] = diff;
                topOutline[j] = i;
            }
            p_thisPix++;
        }
        p_thisLine += stride;
    }
    return int32_t();
}

// data[idx]の前後、値が0に到達するまでの範囲でdataの重心を求める

double_t centerGravity(std::vector<int32_t>& data, int32_t idx)
{
    int32_t moment0 = 0;
    int32_t moment1 = 0;
    int32_t signal = data[idx] < 0 ? -1 : 1; // signal of initial data

    int32_t* ptr = &data[idx];
    for (int32_t i = idx; i >= 0; --i, --ptr) {
        if (*ptr * signal <= 0) {   // if 0 or signal changes 
            break;
        }
        moment0 += *ptr;
        moment1 += *ptr * i;
    }
    ptr = &data[idx + 1];
    for (int32_t i = idx + 1; i < data.size(); i++, ptr++) {
        if (*ptr * signal <= 0) {   // if 0 or signal changes 
            break;
        }
        moment0 += *ptr;
        moment1 += *ptr * i;
    }
    return static_cast<double_t>(moment1) / static_cast<double_t>(moment0);
}