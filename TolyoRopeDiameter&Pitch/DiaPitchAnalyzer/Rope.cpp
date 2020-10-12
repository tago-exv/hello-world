#include "Rope.h"
#include "Outline.h"

using namespace exv;
using namespace rope;

Rope::Result exv::rope::Rope::create(std::vector<double_t>& topOutline, std::vector<double_t>& bottomOutline, double_t cleanRatio)
{
    this->topOutline = Outline(topOutline);
    this->bottomOutline = Outline(bottomOutline);
    this->cleanRatio = cleanRatio;

    // create the member "this->centerline"
    Centerline::Result centerlineResult;
    centerlineResult = centerline.create(this->topOutline, this->bottomOutline, cleanRatio);
    if (centerlineResult != Centerline::Result::Ok) {
        return Rope::Result::Ng;
    }

    // levelOutlin =  - (topOutline - centerline) 
    Outline levelOutline;
    centerlineResult = centerline.subtractMinus(levelOutline, this->topOutline);
    if (centerlineResult != Centerline::Result::Ok) {
        return Rope::Result::Ng;
    }

    // Create the member "this->topWave"
    topWave = Wave(levelOutline);

    // levelOutline =  bottomOutline - centerline
    centerlineResult = centerline.subtract(levelOutline, this->bottomOutline);
    if (centerlineResult != Centerline::Result::Ok) {
        return Rope::Result::Ng;
    }

    // Create the member "this->bottomWave"
    bottomWave = Wave(levelOutline);


    // levelOutline =  bottomOutline - topOutline
    Outline::Result outlineResult;
    outlineResult = this->bottomOutline.subtract(levelOutline, this->topOutline, INVALID_VALUE);
    if (outlineResult != Outline::Result::Ok) {
        return Rope::Result::Ng;
    }

    // Create the member "this->wholeWave"
    wholeWave = Wave(levelOutline);

    return Rope::Result::Ok;
}

Rope::Result Rope::analyze(measure& output, const int32_t specCycle, double_t cleanRatio, const double_t measurePressRatio)
{
    Wave::Result waveResult;
    rope::Line tmpLine;

    // save the specific cycle in the member "this->specCycle"
    this->specCycle = specCycle;

    // Analyze wave cycle of topWave and save them in the member "this->cycle.top"
    waveResult = topWave.analyzeCycle(cycle.top, specCycle, cleanRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng;
    }
    // waveLength of top wave cycle 
    tmpLine.start = Point((cycle.top.cycle[0].start.x + cycle.top.cycle[0].end.x) / 2, 0);
    if (cycle.top.cycle[1].end.x == exv::rope::INVALID_VALUE) {
        tmpLine.end = tmpLine.start;  // if only 1 cycle, line length 0
    }
    else {
        tmpLine.end = Point((cycle.top.cycle[1].start.x + cycle.top.cycle[1].end.x) / 2, 0);
    }
    centerline.add(output.topWavelength, tmpLine); // put the line on the centerline

    // Analyze wave cycle of bottomWave and save them in the member "this->cycle.bottom"
    waveResult = bottomWave.analyzeCycle(cycle.bottom, specCycle, cleanRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng;
    }
    // waveLength of bottom wave cycle 
    tmpLine.start = Point((cycle.bottom.cycle[0].start.x + cycle.bottom.cycle[0].end.x) / 2, 0);
    if (cycle.bottom.cycle[1].end.x == exv::rope::INVALID_VALUE) {
        tmpLine.end = tmpLine.start;  // if only 1 cycle, line length 0
    }
    else {
        tmpLine.end = Point((cycle.bottom.cycle[1].start.x + cycle.bottom.cycle[1].end.x) / 2, 0);
    }
    centerline.add(output.bottomWavelength, tmpLine); // put the line on the centerline

    // Analyze wave cycle of wholeWave and save them in the member "this->cycle.whole"
    waveResult = wholeWave.analyzeCycle(cycle.whole, specCycle, cleanRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng;
    }
    // waveLength of whole wave cycle 
    tmpLine.start = Point((cycle.whole.cycle[0].start.x + cycle.whole.cycle[0].end.x) / 2, 0 );
    if (cycle.whole.cycle[1].end.x == exv::rope::INVALID_VALUE) {
        tmpLine.end = tmpLine.start;  // if only 1 cycle, line length 0
    }
    else {
        tmpLine.end = Point((cycle.whole.cycle[1].start.x + cycle.whole.cycle[1].end.x) / 2, 0);
    }
    centerline.add(output.waveLength, tmpLine); // put the line on the centerline

    // Analyze diameter and radius

    int32_t leftSholder = cycle.whole.cycle[0].start.x;
    int32_t rightSholder = cycle.whole.cycle[0].end.x;

    rope::Point peak;

    waveResult = wholeWave.analyzeRadius(peak, leftSholder, rightSholder, measurePressRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng; // NG
    }
    int32_t idxPeak = peak.x;
    output.diameter = rope::Line(rope::Point(idxPeak, topOutline.data[idxPeak]), rope::Point(idxPeak, bottomOutline.data[idxPeak]));

    waveResult = topWave.analyzeRadius(peak, leftSholder, rightSholder, measurePressRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng; // NG
    }
    tmpLine = rope::Line(Point(peak.x, 0), peak);
    centerline.addMinus(output.topRadius, tmpLine);

    waveResult = bottomWave.analyzeRadius(peak, leftSholder, rightSholder, measurePressRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng; // NG
    }
    tmpLine = rope::Line(Point(peak.x, 0), peak);
    centerline.add(output.bottomRadius, tmpLine);

    // Compatible for old vertion
    waveResult = topWave.analyzeRadius(peak, 0, measurePressRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng; // NG
    }
    tmpLine = rope::Line(Point(peak.x, 0), peak);
    centerline.addMinus(output.topRadius, tmpLine);

    waveResult = bottomWave.analyzeRadius(peak, 0, measurePressRatio);
    if (waveResult != Wave::Result::Ok) {
        return Rope::Result::Ng; // NG
    }
    tmpLine = rope::Line(Point(peak.x, 0), peak);
    centerline.add(output.bottomRadius, tmpLine);

    // centerline
    centerline.get(output.centerline);

    return Result();
}


void exv::rope::Rope::print(std::ostream& out)
{
    out << "Raw outline from the image\n";
    topOutline.print("Top outline", out);
    bottomOutline.print("Bottom outline", out);

    out << "Top wave data\n";
    topWave.print(out);

    out << "Bottom wave data\n";
    bottomWave.print(out);

    out << "Whole wave data\n";
    wholeWave.print(out);
}
