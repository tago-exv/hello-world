#include "pch.h"
#include <windows.h>
#include <string>
#include <iostream>
#include <fstream>
#include <filesystem>
#include <vector>
#include <opencv2\opencv.hpp>

#include "MainUtility.h"
#include "RopeImageDetect.h"
#include "ExImage.h"
#include "ExBitmap.h"
#include "RopeType.h"
#include "Rope.h"

struct IniData {
    double_t specificCycle;
    double_t searchRange;
    double_t cleanRatio;
    double_t measurePressRatio;

    double_t roiLeft;
    double_t roiRight;
    double_t roiTop;
    double_t roiBottom;

    int32_t diffPixes;

    enum { Horizontal, Vertical } ropeDirection;
};


int32_t singleImageAnalysis(const char* path, IniData& ini);
int32_t multiImageAnalysis(const char* path, IniData& ini);
int32_t ropeImageAnalysis(const char* path, IniData& ini, exv::rope::Rope& rope, exv::rope::measure& measure, std::vector<double_t>& time);
void drawResultImageLeftRight(cv::Mat& img, exv::rope::measure& measure, IniData& ini);
void drawResultImageWhole(cv::Mat& img, exv::rope::measure& measure, IniData& ini);


int main(int argc, char* argv[])
{
    // iniファイルから定数を読み出す
    std::string filePath = ".\\RopeAnalyzer.ini";

    IniData ini;

    ini.specificCycle = std::stod(GetConfigString(filePath, "RopeAnalyzer", "SpecificCycle"));
    ini.searchRange = std::stod(GetConfigString(filePath, "RopeAnalyzer", "SearchRange"));
    ini.cleanRatio = std::stod(GetConfigString(filePath, "RopeAnalyzer", "CleanRatio"));
    ini.measurePressRatio = std::stod(GetConfigString(filePath, "RopeAnalyzer", "MeasurePressRatio"));

    ini.roiLeft = std::stod(GetConfigString(filePath, "RopeAnalyzer", "RoiLeft"));
    ini.roiRight = std::stod(GetConfigString(filePath, "RopeAnalyzer", "RoiRight"));
    ini.roiTop = std::stod(GetConfigString(filePath, "RopeAnalyzer", "RoiTop"));
    ini.roiBottom = std::stod(GetConfigString(filePath, "RopeAnalyzer", "RoiBottom"));

    ini.diffPixes = std::stoi(GetConfigString(filePath, "RopeAnalyzer", "DiffPixes"));

    if (GetConfigString(filePath, "RopeAnalyzer", "RopeDirection")[0] == 'H') {
        ini.ropeDirection = IniData::Horizontal;
    }
    else {
        ini.ropeDirection = IniData::Vertical;
    }

    // 画像ファイルの読み出し

    if (argc > 1) {
        filePath = argv[1];
    }
    else {
        std::cout << "Please input a image file path : ";
        std::getline(std::cin, filePath);

        if (filePath.front() == '\"') {
            filePath.erase(filePath.begin());
        }
        if (filePath.back() == '\"') {
            filePath.pop_back();
        }
    }

    std::filesystem::path path(filePath);
    if (path.extension() == ".bmp") {
        singleImageAnalysis(filePath.c_str(), ini);
    }
    else if (path.extension() == ".txt") {
        multiImageAnalysis(filePath.c_str(), ini);
    }
    else {
        std::cout << "Unknown input file\n";
    }
}




int32_t singleImageAnalysis(const char* path, IniData& ini)
{
    exv::rope::Rope rope;
    exv::rope::measure measure;
    std::vector<double_t> time(6);

    ropeImageAnalysis(path, ini, rope, measure, time);

    // 垂直の場合 XY を入れ替える
    if (ini.ropeDirection == IniData::Vertical) {
        measure.swapXY();
    }

    // 結果表示
    // Title
    printf("WaveX, WaveY, WaveLength, Diameter, TopRadius, BottomRadius, Top&BottomRadius, Angle\n");

    double_t topRadius, bottomRadius;

    printf("%.2f, %.2f", measure.waveLength.start.x, measure.waveLength.start.y); // Wave position

    printf(", %.2f", measure.waveLength.length());

    topRadius = measure.centerline.perpendicular(measure.diameter.start).length();
    bottomRadius = measure.centerline.perpendicular(measure.diameter.end).length();
    printf(", %.2f", topRadius + bottomRadius);

    topRadius = measure.centerline.perpendicular(measure.topRadius.end).length();
    printf(", %.2f", topRadius);

    bottomRadius = measure.centerline.perpendicular(measure.bottomRadius.end).length();
    printf(", %.2f", bottomRadius);

    printf(", %.2f", topRadius + bottomRadius);

    printf(", %.2f", measure.centerline.angle());
    printf("\n");

    //　時間表示
    printf("time <Image detect>     %lf[ms]\n", time[0]);
    printf("time <rope.create>      %lf[ms]\n", time[1]);
    printf("time <analyze>          %lf[ms]\n", time[2]);
//    printf("time <measureDiameter>  %lf[ms]\n", time[3]);
//    printf("time <measureCycle>     %lf[ms]\n", time[4]);
    printf("time << TOTAL>>         %lf[ms]\n", time[5]);


    // 結果の画像表示
    cv::Mat img, dst;
    img = cv::imread(path);

    drawResultImageLeftRight(img, measure, ini);

    cv::imshow("Analysis result", img);

    // whole の結果画像表示
    dst = cv::imread(path);

    drawResultImageWhole(dst, measure, ini);
    cv::imshow("Analysis Whole result", dst);

    cv::waitKey(0);

    // 結果の保存
    char writePath[256];
    std::cout << "If you want to save the image, enter file path : ";
    fgets(writePath, sizeof(writePath) - 1, stdin);
    writePath[strlen(writePath) - 1] = '\0';

    if (strlen(writePath) != 0) {
        imwrite(writePath, img);

        std::filesystem::path wrPath(writePath);
        std::filesystem::path extension = wrPath.extension();
        std::filesystem::path filename = wrPath.stem();
        filename += std::filesystem::path("Whole");
        filename += extension;
 //       std::cout << filename << std::endl;
        imwrite(filename.string(), dst);
    }

    // Debug 出力の保存
    std::cout << "If you want to save the debug data, enter file path : ";
    fgets(writePath, sizeof(writePath) - 1, stdin);
    writePath[strlen(writePath) - 1] = '\0';

    if (strlen(writePath) != 0) {
        std::ofstream debugFile(writePath);
        if (!debugFile) {
            std::cout << "File open error !\n";
        }
        else {
            rope.print(debugFile);
        }
    }


	return 0;
}

int32_t multiImageAnalysis(const char* path, IniData& ini)
{
    std::ifstream listFile(path);
    std::string line;
    std::vector<double_t> time(6);

    if (!listFile) {
        printf("List file %s open error\n", path);
        return 0;
    }

    // Title
    printf("Name,WaveX,WaveY,WaveLength,Diameter,TopRadius,BottomRadius,Top&BottomRadius,Angle\n");

    while (listFile >> line) {


        exv::rope::Rope rope;
        exv::rope::measure measure;

        ropeImageAnalysis(line.c_str(), ini, rope, measure, time);

        // 垂直の場合 XY を入れ替える
        if (ini.ropeDirection == IniData::Vertical) {
            measure.swapXY();
        }
        // 結果表示
        std::cout << line;

        double_t topRadius, bottomRadius;

        printf(", %.2f, %.2f", measure.waveLength.start.x, measure.waveLength.start.y); // Wave position

        printf(", %.2f", measure.waveLength.length());

        topRadius = measure.centerline.perpendicular(measure.diameter.start).length();
        bottomRadius = measure.centerline.perpendicular(measure.diameter.end).length();
        printf(", %.2f", topRadius + bottomRadius);

        topRadius = measure.centerline.perpendicular(measure.topRadius.end).length();
        printf(", %.2f", topRadius);

        bottomRadius = measure.centerline.perpendicular(measure.bottomRadius.end).length();
        printf(", %.2f", bottomRadius);

        printf(", %.2f", topRadius + bottomRadius);

        printf(", %.2f", measure.centerline.angle());
        printf("\n");

        if (0) {
            cv::Mat img;
            img = cv::imread(line);

            drawResultImageLeftRight(img, measure, ini);

            std::filesystem::path wrPath(line);
            wrPath.replace_extension(".jpg");
            wrPath = wrPath.filename();

//            std::cout << wrPath << std::endl;
            imwrite(wrPath.string(), img);
        }
    }
    return 1;
};


int32_t ropeImageAnalysis(const char* path, IniData& ini, exv::rope::Rope& rope, exv::rope::measure& measure, std::vector<double_t>& time)
{

    // 時間計測 QueryPerformanceCounter関数の1秒当たりのカウント数を取得する
    LARGE_INTEGER freq, time0, time1, time2, time3, time4, time5;
    QueryPerformanceFrequency(&freq);

    // Read bmp file and create ExImage
    exv::ExImage image;
    exv::bmp::ExBitmapResult exBitmapResult;

    exBitmapResult = exv::bmp::loadBitmapFileToGray8(image, path);
    switch (exBitmapResult) {
    case exv::bmp::ExBitmapResult::Ok:
        break;
    case exv::bmp::ExBitmapResult::ExImageSizeChanged:
        break;
    default:
        std::cout << "loadBitmapFileToGray8() returns " << int(exBitmapResult) << std::endl;
        return 0;
    }

    // 時間計測開始
    QueryPerformanceCounter(&time0);
    //

    // create ROI
    int32_t x0 = image.getWidth() * ini.roiLeft;
    int32_t y0 = image.getHeight() * ini.roiTop;
    int32_t width = image.getWidth() * (ini.roiRight - ini.roiLeft);
    int32_t height = image.getHeight() * (ini.roiBottom - ini.roiTop);
    exv::ExRegion roi(x0, y0, width, height);

    std::vector<double_t> topOutline, bottomOutline;

    int32_t specCycle;  // Specific cycle by pixel

    if (ini.ropeDirection == IniData::Horizontal) {
        imageDetectHorizontalOutline(topOutline, bottomOutline, image, roi, ini.diffPixes, exv::rope::INVALID_VALUE);
        specCycle = width * ini.specificCycle;
    }
    else {
        imageDetectVerticalOutline(topOutline, bottomOutline, image, roi, ini.diffPixes, exv::rope::INVALID_VALUE);
        specCycle = height * ini.specificCycle;
    }

    //　時間計測1
    QueryPerformanceCounter(&time1);

    // Create Rope
    exv::rope::Rope::Result ropeResult;
    ropeResult = rope.create(topOutline, bottomOutline, ini.cleanRatio);

    //　時間計測2
    QueryPerformanceCounter(&time2);
 
    // Analyze Rope
    exv::rope::RopeCycle ropeCycle;

    ropeResult = rope.analyze(measure, specCycle, ini.cleanRatio, ini.measurePressRatio);
    if (ropeResult != exv::rope::Rope::Result::Ok) {
        std::cout << "analyze() returns " << int(ropeResult) << std::endl;
        return 0;
    }
    //　時間計測3
    QueryPerformanceCounter(&time3);

    //  残り
    time5 = time4 = time3;


    //　時間data
    time[0] = static_cast<double_t>(time1.QuadPart - time0.QuadPart) * 1000.0 / freq.QuadPart;
    time[1] = static_cast<double_t>(time2.QuadPart - time1.QuadPart) * 1000.0 / freq.QuadPart;
    time[2] = static_cast<double_t>(time3.QuadPart - time2.QuadPart) * 1000.0 / freq.QuadPart;
    time[3] = static_cast<double_t>(time4.QuadPart - time3.QuadPart) * 1000.0 / freq.QuadPart;
    time[4] = static_cast<double_t>(time5.QuadPart - time4.QuadPart) * 1000.0 / freq.QuadPart;
    time[5] = static_cast<double_t>(time5.QuadPart - time0.QuadPart) * 1000.0 / freq.QuadPart;

    return 1;
}



void drawResultImageLeftRight(cv::Mat& img, exv::rope::measure& measure, IniData& ini)
{
    cv::Point start, end;
    exv::rope::Point tmp;
    exv::rope::Line topRadius, bottomRadius, topWavelength, bottomWavelength;

    int32_t width = img.size().width * (ini.roiRight - ini.roiLeft);
    int32_t height = img.size().height * (ini.roiBottom - ini.roiTop);

    // Line
    start = cv::Point(measure.centerline.start.x, measure.centerline.start.y);
    end = cv::Point(measure.centerline.end.x, measure.centerline.end.y);
    cv::line(img, start, end, cv::Scalar(0, 255, 0), 3);

    start = cv::Point(measure.topRadius.start.x, measure.topRadius.start.y);
    end = cv::Point(measure.topRadius.end.x, measure.topRadius.end.y);
    cv::line(img, start, end, cv::Scalar(255, 0, 0), 3);

    topRadius = measure.centerline.perpendicular(measure.topRadius.end);
    start = cv::Point(topRadius.start.x, topRadius.start.y);
    end = cv::Point(topRadius.end.x, topRadius.end.y);
    cv::line(img, start, end, cv::Scalar(0, 0, 255), 1);

    start = cv::Point(measure.bottomRadius.start.x, measure.bottomRadius.start.y);
    end = cv::Point(measure.bottomRadius.end.x, measure.bottomRadius.end.y);
    cv::line(img, start, end, cv::Scalar(255, 0, 0), 3);

    bottomRadius = measure.centerline.perpendicular(measure.bottomRadius.end);
    start = cv::Point(bottomRadius.start.x, bottomRadius.start.y);
    end = cv::Point(bottomRadius.end.x, bottomRadius.end.y);
    cv::line(img, start, end, cv::Scalar(0, 0, 255), 1);

    topWavelength = measure.topWavelength.moveTo(topRadius.end);
    start = cv::Point(topWavelength.start.x, topWavelength.start.y);
    end = cv::Point(topWavelength.end.x, topWavelength.end.y);
    cv::line(img, start, end, cv::Scalar(0, 0, 255), 3);

    bottomWavelength = measure.bottomWavelength.moveTo(bottomRadius.end);
    start = cv::Point(bottomWavelength.start.x, bottomWavelength.start.y);
    end = cv::Point(bottomWavelength.end.x, bottomWavelength.end.y);
    cv::line(img, start, end, cv::Scalar(0, 0, 255), 3);

    // Text
    char text[256];

    snprintf(text, sizeof(text), "Top outline");
    cv::putText(img, text, cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

    snprintf(text, sizeof(text), "Radius=%.2f", topRadius.length());
    cv::putText(img, text, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

    snprintf(text, sizeof(text), "Wave length=%.2f", topWavelength.length());
    cv::putText(img, text, cv::Point(20, 90), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

    snprintf(text, sizeof(text), "Bottom outline");
    cv::putText(img, text, cv::Point(width - 400, height - 90), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

    snprintf(text, sizeof(text), "Radius=%.2f", bottomRadius.length());
    cv::putText(img, text, cv::Point(width - 400, height - 60), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

    snprintf(text, sizeof(text), "Wave length=%.2f", bottomWavelength.length());
    cv::putText(img, text, cv::Point(width - 400, height - 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);

}

void drawResultImageWhole(cv::Mat& img, exv::rope::measure& measure, IniData& ini)
{
    cv::Point start, end;
    exv::rope::Point tmp;
    exv::rope::Line topRadius, bottomRadius;
    int32_t width = img.size().width * (ini.roiRight - ini.roiLeft);
    int32_t height = img.size().height * (ini.roiBottom - ini.roiTop);

    // Line
    start = cv::Point(measure.centerline.start.x, measure.centerline.start.y);
    end = cv::Point(measure.centerline.end.x, measure.centerline.end.y);
    cv::line(img, start, end, cv::Scalar(0, 255, 0), 3);

    start = cv::Point(measure.diameter.start.x, measure.diameter.start.y);
    end = cv::Point(measure.diameter.end.x, measure.diameter.end.y);
    cv::line(img, start, end, cv::Scalar(255, 0, 0), 3);

//    topRadius = measure.centerline.perpendicular(measure.diameter.start);
    topRadius = measure.topRadius;
    start = cv::Point(topRadius.start.x, topRadius.start.y);
    end = cv::Point(topRadius.end.x, topRadius.end.y);
    cv::line(img, start, end, cv::Scalar(0, 0, 255), 1);

//    bottomRadius = measure.centerline.perpendicular(measure.diameter.end);
    bottomRadius = measure.bottomRadius;
    start = cv::Point(bottomRadius.start.x, bottomRadius.start.y);
    end = cv::Point(bottomRadius.end.x, bottomRadius.end.y);
    cv::line(img, start, end, cv::Scalar(0, 0, 255), 1);

    start = cv::Point(measure.waveLength.start.x, measure.waveLength.start.y);
    end = cv::Point(measure.waveLength.end.x, measure.waveLength.end.y);
    cv::line(img, start, end, cv::Scalar(0, 0, 255), 3);

    // Text
    char text[256];

    snprintf(text, sizeof(text), "Whole outline");
    cv::putText(img, text, cv::Point(20, 30), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 255, 0), 2);

    snprintf(text, sizeof(text), "Diameter=%.2f", topRadius.length() + bottomRadius.length());
    cv::putText(img, text, cv::Point(20, 60), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(255, 0, 0), 2);

    snprintf(text, sizeof(text), "Wave length=%.2f", measure.waveLength.length());
    cv::putText(img, text, cv::Point(20, 90), cv::FONT_HERSHEY_SIMPLEX, 1, cv::Scalar(0, 0, 255), 2);
}
