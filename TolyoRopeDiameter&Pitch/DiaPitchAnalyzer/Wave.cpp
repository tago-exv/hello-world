#include <iostream>
#include "RopeType.h"
#include "Wave.h"
#include "Outline.h"

using namespace exv;
using namespace rope;

Wave::Result Wave::analyzeCycle(Cycle& output, const int32_t specCycle, const double_t cleanRatio)
{
    Outline::Result outlineResult;

    // initialize output
    output.num = 0;
    cycle.num = 0;

    // 外形線から外れ値を除去する
    // Reject outlier values in the outline 
    outlineResult = outline.clean(cleanOutline, cleanRatio);
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }

    // 移動平均によるスムージングを行い高周波成分をカットする
    int32_t averagePeriod = specCycle / 4;
    outlineResult = cleanOutline.smooth(midFrqOutline, specCycle / 4);
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }

    // 移動平均によるスムージングを行い低周波成分をカットする
    outlineResult = cleanOutline.smooth(lowFrqOutline, specCycle);
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }

    // 両者を引き算して波形の周波数成分を取り出す
    outlineResult = midFrqOutline.subtract(smoothOutline, lowFrqOutline);
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }

    // Get the middle value
    Outline::Maxmin mxmn;
    outlineResult = smoothOutline.maxmin(mxmn);
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }

//    DataType midValue = (mxmn.max + mxmn.min) / 2;
    DataType midValue = 0;

    // Save the amplitude value to member this->waveAmplitude
    waveAmplitude = mxmn.max - mxmn.min;

    // initialize the member this->cycle
    cycle.num = 0;
    for (int32_t i = 0; i < exv::rope::MAX_NUM_CYCLE; i++) {
        cycle.cycle[i].start.x = cycle.cycle[i].start.y = exv::rope::INVALID_VALUE;
        cycle.cycle[i].end.x = cycle.cycle[i].end.y = exv::rope::INVALID_VALUE;
    }

    // search the left and right sholders by state machine
    enum { Initial, SearchLeft, SearchRight, Complete } state = Initial;

    int32_t idxCycle = 0;
    auto ptr = &smoothOutline.data[0];
    for (int i = 0; i < smoothOutline.size(); i++, ptr++) {
        if (*ptr == exv::rope::INVALID_VALUE) {
            continue;
        }
        switch (state) {
        case Initial:
            if (*ptr <= midValue) {
                state = SearchLeft;
            }
            break;
        case SearchLeft:
            if (*ptr > midValue) {
                cycle.num++;
                cycle.cycle[idxCycle].start.x = i - (*ptr - midValue) / (*ptr - *(ptr - 1));
                cycle.cycle[idxCycle].start.y = midValue;
                state = SearchRight;
            }
            break;
        case SearchRight:
            if (*ptr <= midValue) {
                cycle.cycle[idxCycle].end.x = i - (*ptr - midValue) / (*ptr - *(ptr - 1));
                cycle.cycle[idxCycle].end.y = midValue;
                state = Complete;
            }
            break;

        default:
            std::cout << "Wave::analyzeCycle() unexpected state" << state << std::endl;
            return Wave::Result::Ng;
        }
        if (state == Complete) {
            idxCycle++;
            if (idxCycle >= exv::rope::MAX_NUM_CYCLE) {
                break;
            }
            else {
                state = Initial;    // try to find a next wave
            }
        }
    }
    output = cycle;
    return Wave::Result::Ok;
}


Wave::Result Wave::analyzeRadius(Point& output, int32_t idxCycle, const double_t measurePressRatio)
{
    if (idxCycle > cycle.num
        || cycle.cycle[idxCycle].start.x == exv::rope::INVALID_VALUE
        || cycle.cycle[idxCycle].end.x == exv::rope::INVALID_VALUE) {
        return Wave::Result::Ng;
    }

    int32_t left = cycle.cycle[idxCycle].start.x;
    int32_t right = cycle.cycle[idxCycle].end.x;
    return analyzeRadius(output, left, right, measurePressRatio);
}

Wave::Result Wave::analyzeRadius(Point& output, int32_t left, int32_t right, const double_t measurePressRatio)
{
    int32_t idxMax = left;

    DataType max = DATA_MIN;
    DataType validRange = waveAmplitude * measurePressRatio;
    auto ptrData = &outline.data[left];
    auto ptrSmooth = &midFrqOutline.data[left];
    for (int32_t i = left; i < right; i++) {
        if (*ptrData > max && *ptrData <= (*ptrSmooth + validRange)) {
            max = *ptrData;
            idxMax = i;
        }
        ptrData++;
        ptrSmooth++;
    }
    // save the peak point to the member this->peak
    peak.y = max;
    peak.x = idxMax;
    output = peak;

    return Wave::Result::Ok;
}


Wave::Result Wave::measureRadius(Point& output, const double_t measurePressRatio)
{
    Outline::Result outlineResult;

    //　left、rightとも、落ち続けて反発上昇するところまで広げる
    //　smoothOutlineは、両端にinvalidValueが溜まっているので注意！
    
    int32_t left = cycle.cycle[0].start.x;
    int32_t right = cycle.cycle[0].end.x;

    int32_t n_exp;
    DataType lastValue;

    auto ptr = &smoothOutline.data[left];
    n_exp = 0;
    lastValue = *ptr;
    for (int32_t i = 0; i < left; i++) {
        --ptr;
        if (*ptr != INVALID_VALUE) {
            n_exp = i;
            if (*ptr > lastValue) {
                break;
            }
            else {
                lastValue = *ptr;
            }
        }
    }
    left -= n_exp;

    ptr = &smoothOutline.data[right];
    n_exp = 0;
    lastValue = *ptr;
    for (int32_t i = 0; i < smoothOutline.size() - right; i++) {
        ptr++;
        if (*ptr != INVALID_VALUE) {
            n_exp = i;
            if (*ptr > lastValue) {
                break;
            }
            else {
                lastValue = *ptr;
            }
        }
    }
    right += n_exp;
   
    // Extract 1 wave data "this->oneWave"
    outlineResult = cleanOutline.cutOff(oneWave, left, (right - left));
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }
    // Create bins for a histogram
    Outline::Maxmin mxmn;
    oneWave.maxmin(mxmn);

   int32_t binsMax = static_cast<int32_t>(mxmn.max);
   int32_t binsMin = static_cast<int32_t>(mxmn.min);

    std::vector<DataType> bins(binsMax - binsMin + 1);
    auto itr = bins.begin();
    for (int32_t i = binsMin; i <= binsMax; i++) {
        *itr++ = i;
    }

    // create Histogram
    Outline::Histogram hist;
    outlineResult = oneWave.histogram(hist, bins);
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }

    // Touch point means where cumulative frequency reaches touch size
    int32_t touchSize = hist.totalFrqs * measurePressRatio;
    int32_t touchPoint = INVALID_VALUE;

    int32_t cumulat = 0;
    auto itr_bins = hist.bins.end() - 1;
    auto itr_frqs = hist.frqs.end() - 1;
    for (int32_t i = 0; i < hist.frqs.size(); i++) {
        cumulat += *itr_frqs;
        if (cumulat > touchSize) {
            touchPoint = *itr_bins;
            break;
        }
        --itr_bins;
        --itr_frqs;
    }
    // save the peak point to the member this->peak
    peak.y = touchPoint;
    peak.x = (cycle.cycle[0].start.x + cycle.cycle[0].end.x) / 2;
    output = peak;

    return Wave::Result::Ok;
}

Wave::Result Wave::measureWavelength(Line& output, const int32_t specCycle, const int32_t searchRange)
{
    Outline::Result outlineResult;

    int32_t refCenter = (cycle.cycle[0].start.x + cycle.cycle[0].end.x) / 2;
    int32_t refLeft = refCenter - specCycle / 2;
    int32_t refLength = specCycle;

    int32_t searchStart = refLeft + specCycle - searchRange;
    int32_t searchEnd = refLeft + specCycle + searchRange;
    int32_t searchLength = searchEnd - searchStart;

    // 参照部を検索範囲の最後まで移動させたとき、全体からはみ出す場合は参照部を短縮する
    int32_t outlineLength = this->smoothOutline.size();
    if (refLength > outlineLength - searchEnd) {
        refLength = outlineLength - searchEnd;
    }
    // 参照部の長さが、specCycle の半分に満たない場合は、計測不能とする
    if (refLength < specCycle / 2) {
        return Wave::Result::Ng;
    }

    // member "smoothOutline" 上で自己相関処理を行い最小点を求める。結果は member "correlation" に受け取る
    outlineResult = this->smoothOutline.autocorrelation(this->correlation, refLeft, refLength, searchStart, searchLength);
    if (outlineResult != Outline::Result::Ok) {
        return Wave::Result::Ng;
    }

    //　outputは member "peak" からの線分とする
    output.start = peak;
    output.end.x = peak.x + (specCycle - searchRange + this->correlation.idxMin);
    output.end.y = peak.y;

    return Wave::Result::Ok;
}

void Wave::print(std::ostream& out) {
    outline.print("Outline", out);
    cleanOutline.print("Clean outline", out);
    midFrqOutline.print("MidFrq outline", out);
    lowFrqOutline.print("LowFrq outline", out);
    smoothOutline.print("Smooth outline", out);
//    cycle.print("Wave cycles", out);
    out << "Wave cycles";
    for (int32_t i = 0; i < cycle.num; i++) {
        out << ", " << cycle.cycle[i].start.x;
        out << ", " << cycle.cycle[i].end.x;
    }
    out << std::endl;
//    correlation.print("Autocorrelation data", out);
}
