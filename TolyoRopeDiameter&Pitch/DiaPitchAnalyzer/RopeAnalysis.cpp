#include <iostream>
#include "RopeAnalysis.h"

#define DEBUG

namespace exv {
namespace rope{


    RopeAnalysis::RopeAnalysis(void) {}
    RopeAnalysis::~RopeAnalysis(void){}

/*!
* ロープの上側外形線と下側外形線を求める
*
* 8ビットグレイスケール画像の指定されたROI内を2値化し、各列Y方向「最後の」白→黒変化点のY座標の配列を生成する
* @param topOutline  出力上側外形線のＹ座標配列
* @param topOutline  出力下側外形線のＹ座標配列
* @param grayImage 入力グレイスケール画像
* @param roi ROI
* @param binarizeThreshold 2値画像生成時のしきい値
*/
	void RopeAnalysis::createOutline(Outlines& outlines, const ExImage& grayImage, ExRegion roi, uint8_t binarizeThreshold)
	{
        std::int32_t left = roi.x;
        std::int32_t top = roi.y;
        std::int32_t width = roi.width;
        std::int32_t height = roi.height;
        std::int32_t halfHeight = height / 2;

        std::int32_t stride = std::int32_t(grayImage.getStride());

        std::uint8_t* grayPtr = static_cast<std::uint8_t*>(grayImage.getImageDataPointer());
        // Get the top side outline 
        for (std::int32_t y = top + 1; y < halfHeight; ++y)
        {
            // Move to the top of the line
            std::uint8_t* p_thisLine = grayPtr + (stride * y) + left;
            std::uint8_t* p_prevLine = p_thisLine - stride;
            for (std::int32_t x = 0; x < width; ++x)
            {
                if (*p_thisLine < binarizeThreshold && *p_prevLine >= binarizeThreshold) {
                    outlines.topOutline[x] = y;
                }
                p_thisLine++;
                p_prevLine++;
            }
        }
        // Get the bottom side outline
        for (std::int32_t y = top + height - 2 ; y >= halfHeight; --y)
        {
            // Move to the top of the line
            std::uint8_t* p_thisLine = grayPtr + (stride * y) + left;
            std::uint8_t* p_prevLine = p_thisLine + stride;
            for (std::int32_t x = 0; x < width; ++x)
            {
                if (*p_thisLine < binarizeThreshold && *p_prevLine >= binarizeThreshold) {
                    outlines.bottomOutline[x] = y;
                }
                p_thisLine++;
                p_prevLine++;
            }
        }
        // Get center line
        for (std::int32_t x = 0; x < width; ++x) {
            if (outlines.topOutline[x] != INVALID_VALUE && outlines.topOutline[x] != INVALID_VALUE) {
                outlines.centerLine[x] = (outlines.topOutline[x] + outlines.bottomOutline[x]) / 2;
            }
            else {
                outlines.centerLine[x] = INVALID_VALUE;
            }
        }
    }



    void RopeAnalysis::createOutline(std::vector<int32_t>& output, ExImage& image, ExRegion& roi)
    {

        std::int32_t left = roi.x;
        std::int32_t top = roi.y;
        std::int32_t width = roi.width;
        std::int32_t height = roi.height;
        std::int32_t stride = image.getStride();

        std::uint8_t* grayPtr = static_cast<std::uint8_t*>(image.getImageDataPointer());
        grayPtr += stride * top;

        std::vector<int32_t> outline(width);
        std::vector<int32_t>::iterator itr;

        for (itr = outline.begin(); itr != outline.end(); itr++) {
            *itr = 0;
        }

        for (std::int32_t y = 0; y < height; y++) {
            std::uint8_t* iPtr = grayPtr + (stride * y) + left;

            for (std::int32_t x = 0; x < width; x++) {
//                outline[x] += *iPtr++;
                outline[x] += (256 - *iPtr++);
            }
        }
        output = outline;
    }

    void RopeAnalysis::cleanOutline(std::vector<int32_t>& output, std::vector<int32_t>& input, double_t rejectRatio, int32_t division, int32_t invalidValue ) {
            
        Histogram hist;
        RopeAnalysis::histogram(hist, input, division, invalidValue);

        int32_t n_reject = input.size() * rejectRatio;
        int32_t validMax, validMin;

        int32_t cumul;
		int32_t *p_bins, *p_frqs;

		cumul = 0;
		p_bins = &hist.bins[0];
		p_frqs = &hist.frqs[0];
		for (int32_t i = 0; i < hist.frqs.size() ; i++){
			cumul += *p_frqs;
			if(cumul > n_reject ){
				validMin = *p_bins;
				break;
			}
			p_bins++;
			p_frqs++;
		}

		cumul = 0;
		p_bins = &hist.bins[hist.bins.size()-1];
		p_frqs = &hist.frqs[hist.frqs.size()-1];
		for (int32_t i = 0; i < hist.frqs.size(); i++) {
			cumul += *p_frqs;
			if (cumul > n_reject) {
				validMax = *p_bins;
				break;
			}
			--p_bins;
			--p_frqs;
		}

        std::vector<int32_t> tmp = input;
        for (auto itr = tmp.begin(); itr != tmp.end(); itr++) {
            if (*itr == invalidValue) {
                continue;
            }
            if (*itr < validMin || *itr > validMax) {
                *itr = invalidValue;
            }
        }
        output = tmp;
    }

    //  最小二乗法で直線の式を求める
    //  ｙ＝ ax ＋ b としたとき
    //  a = { n * Σ(xy) - Σ(x)Σ(y) } / { n* Σ(x**2) - (Σ(x)) **2}
    //  b = { Σ(x**2) * Σ(y) - Σ(xy)Σ(x) } / { n* Σ(x**2) - (Σ(x))**2}

    void RopeAnalysis::LeastSquare(LinearEquation& output, std::vector<int32_t>& input, int32_t invalidValue)
    {
        int32_t n = 0;
        double_t x = 0;
        double_t y = 0;
        double_t xx = 0;
        double_t xy = 0;

        int32_t* p_input = &input[0];
        for (int32_t i = 0; i < input.size(); i++, p_input++) {
            if (*p_input == invalidValue) {
                continue;
            }
            n++;
            x += i;
            y += *p_input;
            xx += i * i;
            xy += i * *p_input;
        }
        output.a = static_cast<double_t>(n * xy - x * y) / static_cast<double_t>(n * xx - x * x);
        output.b = static_cast<double_t>(xx * y - xy * x) / static_cast<double_t>(n * xx - x * x);
    }


    //  アウトラインデータ列から、おおよそ１山周期の範囲を求める
    //　アウトラインデータ列を移動平均でスムージングして、データ全体の中間値とのゼロクロスポイントを求める
    //　引数 expand がtrueの場合、さらに谷底まで範囲を広げる
    void RopeAnalysis::getCoarseCycle(LineSegment& output, std::vector<int32_t>& outline, int32_t specCycle, bool expand, int32_t invalidValue)
    {

        int32_t avePeriod = specCycle / 4;
        int32_t midValue;
        std::vector<int32_t> smoothOutline(outline.size());

        movingAverage(smoothOutline, outline, avePeriod, invalidValue);
#ifdef DEBUG
        printLineData("Smoothing outline", smoothOutline);
#endif

        // Get the middle value of cutting line
 //       midValue = getAverage(smoothOutline, invalidValue);  // 全体の対称性がないと平均値は中間値に適さない可能性がある
        rope::Maxmin mxmn;
        RopeAnalysis::maxmin(mxmn, smoothOutline, invalidValue);
        midValue = (mxmn.max + mxmn.min) / 2;

        // search the left and right sholders by state machine
        int32_t left = invalidValue;
        int32_t right = invalidValue;
        enum {Initial, SearchLeft, SearchRight, Complete} state = Initial;
 
        auto itr = smoothOutline.begin();
        for (int i = 0; i < smoothOutline.size(); i++, itr++) {
            if (*itr == invalidValue) {
                continue;
            }
            switch (state) {
            case Initial:
                if (*itr <= midValue) {
                    state = SearchLeft;
                }
                break;
            case SearchLeft:
                if (*itr > midValue) {
                    left = i;
                    state = SearchRight;
                }
                break;
            case SearchRight:
                if (*itr <= midValue) {
                    right = i;
                    state = Complete;
                    i = smoothOutline.size(); // to break for loop
                }
                break;
            default:
                std::cout << "RopeAnalysis::getCoarseCycle() unexpected state" << state << std::endl;
            }
        }
        //　両端まで見つからなかった場合、終了する
        if(state != Complete){
            output.start = invalidValue;
            output.len = invalidValue;
            return;
        }
#ifdef DEBUG
        printf("midValue = %d  left = %d  right = %d\n", midValue, left, right);
#endif

        //　left、rightとも、落ち続けて反発上昇するところまで広げる
        //　smoothOutlineは、両端にinvalidValueが溜まっているので注意！
        if (expand == true) { 
            int32_t n_exp;
            int32_t lastValue;

            itr = smoothOutline.begin() + left;
            n_exp = 0;
            lastValue = *itr;
            for (int32_t i = 0 ; i < left ; i++){
                --itr;
                if (*itr != invalidValue) {
                    n_exp = i;
                    if (*itr > lastValue) {
                        break;
                    }
                    else {
                        lastValue = *itr;
                    }
                }
            }
            left -= n_exp;

            itr = smoothOutline.begin() + right;
            n_exp = 0;
            lastValue = *itr;
            for (int32_t i = 0; i < smoothOutline.size() - right ; i++) {
                itr++;
                if (*itr != invalidValue) {
                    n_exp = i;
                    if (*itr > lastValue) {
                        break;
                    }
                    else {
                        lastValue = *itr;
                    }
                }
            }
            right += n_exp;
        }
        output.start = left;
        output.len = right - left;


#if 1
        std::vector<int32_t> diffOutline(outline.size());
        std::vector<int32_t> diff2Outline(outline.size());

        getDifferential(diffOutline, smoothOutline, avePeriod, invalidValue);
        printLineData("Differential of Outline", diffOutline);

        getDifferential(diff2Outline, diffOutline, specCycle / 2, invalidValue);
        printLineData("2nd differential of Outline", diff2Outline);
#endif

    }

    void RopeAnalysis::expandCoarseCycle(LineSegment& output, std::vector<int32_t>& Outline, LineSegment input, int32_t invalidValue)
    {
    }

    void RopeAnalysis::measureOutline(int32_t& output, std::vector<int32_t>& input, LineSegment coarseCycle, double_t measurePressRatio, int32_t invalidValue)
    {
        // Cut and copy to working line
        std::vector<int32_t> wrkline(coarseCycle.len);
        auto itr_src = input.begin() + coarseCycle.start;
        for (auto itr_dst = wrkline.begin(); itr_dst != wrkline.end(); itr_dst++, itr_src++) {
            *itr_dst = *itr_src;
        }
        
        // Create bins for a histogram
        rope::Maxmin mxmn;
        RopeAnalysis::maxmin(mxmn, wrkline, invalidValue);

        std::vector<int32_t> bins(mxmn.max - mxmn.min + 1);
        auto itr = bins.begin();
        for (int32_t i = mxmn.min ; i <= mxmn.max ; i++) {
            *itr++ = i;
        }

        // create Histogram
        rope::Histogram hist;
        RopeAnalysis::histogram(hist, wrkline, bins, invalidValue);

        // Touch point means where cumulative frequency reaches touch size
        int32_t touchSize = hist.totalFrqs * measurePressRatio;
        int32_t touchPoint = invalidValue;

        int32_t cumulat = 0;
        auto itr_bins = hist.bins.end()-1;
        auto itr_frqs = hist.frqs.end()-1;
        for (int32_t i = 0; i < hist.frqs.size(); i++) {
            cumulat += *itr_frqs;
            if (cumulat > touchSize) {
                touchPoint = *itr_bins;
                break;
            }
            --itr_bins;
            --itr_frqs;
        }
        output = touchPoint;
    }





	void RopeAnalysis::measureCycle(Cycle& output, std::vector<int32_t>& input, int32_t ref_start, int32_t ref_len, int32_t eval_start, int32_t eval_len)
	{
        if (eval_start + eval_len + ref_len > input.size()) {
            return;
        }

        std::vector<int32_t> pos(eval_len);
        std::vector<double_t> score(eval_len);
        auto itrPos = pos.begin();
        auto itrScore = score.begin();

        int32_t* p_ref = &input[ref_start];
        int32_t* p_eval = &input[eval_start];

        for (int32_t i = 0; i < eval_len; i++) {
            *itrPos++ = eval_start - ref_start + i;
            *itrScore++ = evalDifference(p_ref, p_eval, ref_len);
            p_eval++;
        }
        d_Maxmin mxmn;
        maxmin(mxmn, score);
        output.cycle = eval_start - ref_start + mxmn.idxMin;
        output.score = score;
        output.pos = pos;
	}


    void RopeAnalysis::analyPitch(double_t& pitch, std::vector<int32_t>& outScore ,std::vector<int32_t>& input, double_t patternLength, double_t stdPitch, double_t searchRange, int32_t invalidValue) {
        int32_t n_patternLength = input.size() * patternLength;
        int32_t n_pitch = input.size() * stdPitch;
        int32_t n_searchRange = n_pitch * searchRange;
        int32_t start = n_pitch - n_searchRange;
        int32_t end = n_pitch + n_searchRange;
        if (end > input.size() - n_patternLength) {
            end = input.size() - n_patternLength;
        }
        
        std::vector<int32_t> score(end - start);
        auto itrScore = score.begin();
        for (int32_t i = start; i < end; i++) {
            *itrScore++ = evalDifference(&input[0], &input[i], n_patternLength);
        }
        outScore = score;
        
        Maxmin mxmn;
        maxmin(mxmn, score);
        pitch = start + mxmn.idxMin;

#ifdef DEBUG
		int32_t* p_input;
		p_input = &input[0];
		for (int32_t i =0; i < n_patternLength; i++) {
			std::cout << *p_input++ << ", ";
		}
		std::cout << std::endl;

		for (int32_t i = 0; i < start + mxmn.idxMin; i++) {
			std::cout << ", ";
		}
		p_input = &input[0];
		for (int32_t i = 0; i < n_patternLength; i++) {
			std::cout << *p_input++ << ", ";
		}
		std::cout << std::endl;


		for (int32_t i = start; i < end; i++) {
			std::cout << i << ", ";
		}
		std::cout << std::endl;
#endif
    }


    double_t RopeAnalysis::evalDifference(int32_t* p_ptn, int32_t* p_obj, int32_t len, int32_t invalidValue) {
        int32_t score = 0;
        int32_t n_valid = 0;
        for (int32_t i = 0 ; i < len ; i++) {
            if (*p_obj != invalidValue && *p_ptn != invalidValue) {
                n_valid++;
                int32_t dif = *p_obj - *p_ptn;
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
//        if (n_valid == 0) {
//            return -1;
//        }
        return static_cast<double_t>(score) / static_cast<double_t>(n_valid);
    }


    void RopeAnalysis::histogram(Histogram& output, std::vector<int32_t>& input, int32_t n_bins, int32_t invalidValue)
    {

		rope::Maxmin mxmn;
		RopeAnalysis::maxmin(mxmn, input, invalidValue);

		std::vector<int32_t> tmp_bins(n_bins + 1);
		auto itr = tmp_bins.begin();
        for (int32_t i = 0; i <= n_bins; i++) {
            *itr++ = mxmn.min + (mxmn.max - mxmn.min) * i / n_bins;
        }
        RopeAnalysis::histogram(output, input, tmp_bins);
    }

    void RopeAnalysis::histogram(Histogram& output, std::vector<int32_t>& input, std::vector<int32_t>& bins, int32_t invalidValue) {
        int32_t n_bins = bins.size() - 1;
        std::vector<int32_t> tmp_frqs(n_bins, 0);
        int32_t total_frqs = 0;

        for (auto itr = input.begin(); itr != input.end(); itr++) {
            if (*itr == invalidValue) {
                continue;
            }
            for (int32_t i = 0 ; i < n_bins ; i++) {
                if (*itr <= bins[i+1]) {
                    tmp_frqs[i]++;
                    break;
                }
            }
            total_frqs++;
        }
        output.bins = bins;
        output.frqs = tmp_frqs;
        output.totalFrqs = total_frqs;
    }

    void RopeAnalysis::maxmin(Maxmin& maxmin, std::vector<int32_t>& input, int32_t invalidValue) {
        int32_t t_max = INT32_MIN;
        int32_t t_min = INT32_MAX;
        int32_t ix_max = 0;
        int32_t ix_min = 0;
        
        auto itr_input = input.begin();
        for ( int32_t i = 0 ; i < input.size() ; i++, itr_input++) {
            if (*itr_input == invalidValue) {
                continue;
            }
            if (*itr_input > t_max) {
                t_max = *itr_input;
                ix_max = i;
            }
            else if (*itr_input < t_min) {
                t_min = *itr_input;
                ix_min = i;
            }
        }
        maxmin.max = t_max;
        maxmin.min = t_min;
        maxmin.idxMax = ix_max;
        maxmin.idxMin = ix_min;
    }

    void RopeAnalysis::maxmin(d_Maxmin& maxmin, std::vector<double_t>& input, double_t invalidValue) {
        auto itr_input = input.begin();
        auto t_max = *itr_input;
        auto t_min = *itr_input;
        int32_t ix_max = 0;
        int32_t ix_min = 0;

        for (int32_t i = 0; i < input.size(); i++, itr_input++) {
            if (*itr_input == invalidValue) {
                continue;
            }
            if (*itr_input > t_max) {
                t_max = *itr_input;
                ix_max = i;
            }
            else if (*itr_input < t_min) {
                t_min = *itr_input;
                ix_min = i;
            }
        }
        maxmin.max = t_max;
        maxmin.min = t_min;
        maxmin.idxMax = ix_max;
        maxmin.idxMin = ix_min;
    }


    int32_t RopeAnalysis::getAverage(std::vector<int32_t>& input, int32_t invalidValue) {
        int32_t sum = 0;
        int32_t num = 0;

        for (auto itr = input.begin(); itr != input.end(); itr++) {
            if (*itr == invalidValue) {
                continue;
            }
            sum += *itr;
            num++;
        }
        return( num != 0 ? sum / num : invalidValue);
    }


    void RopeAnalysis::movingAverage(std::vector<int32_t>& output, std::vector<int32_t>& input, int32_t period, int32_t invalidValue)
    {
        for (auto itr = output.begin(); itr != output.end(); itr++) {
            *itr = invalidValue;
        }
        if (period > input.size()) {
            return;
        }
        auto itr_out = output.begin() + period/2;

        auto itr_head = input.begin();
        auto itr_tail = itr_head;
        int32_t sum = 0;
        int32_t num = 0;
        for (int32_t i = 0; i < period; i++, itr_head++) {
            if (*itr_head == invalidValue) {
                continue;
            }
            sum += *itr_head;
            num++;
        }
        *itr_out++ = num != 0 ? sum / num : invalidValue;

        for (; itr_head != input.end(); itr_head++, itr_tail++) {
            if (*itr_head != invalidValue) {
                sum += *itr_head;
                num++;
            }
            if (*itr_tail != invalidValue) {
                sum -= *itr_tail;
                --num;
            }
            *itr_out++ = num != 0 ? sum / num : invalidValue;
        }
    }

    void RopeAnalysis::getDifferential(std::vector<int32_t>& output, std::vector<int32_t>& input, int32_t period, int32_t invalidValue)
    {
        for (auto itr = output.begin(); itr != output.end(); itr++) {
            *itr = invalidValue;
        }
        if (period > input.size()) {
            return;
        }
        auto itr_out = output.begin() + period / 2;
        auto itr_tail = input.begin();
        auto itr_head = itr_tail + period;
        for (; itr_head != input.end(); itr_head++, itr_tail++, itr_out++) {
            if (*itr_head != invalidValue && *itr_tail != invalidValue) {
                *itr_out = *itr_head - *itr_tail;
            }
        }
    }

    void RopeAnalysis::printLineData(const char* title, std::vector<int32_t>& lineData)
    {
        std::cout << title ;
        for (auto itr = lineData.begin(); itr != lineData.end(); itr++) {
            std::cout << ", " << *itr ;
        }
        std::cout << std::endl;
    }

    void RopeAnalysis::printLineData(const char* title, std::vector<double_t>& lineData)
    {
        std::cout << title;
        for (auto itr = lineData.begin(); itr != lineData.end(); itr++) {
            std::cout << ", " << *itr;
        }
        std::cout << std::endl;
    }
}
}