/*
 * Copyright (c) 2015, Marko Linna
 * All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * 1. Redistributions of source code must retain the above copyright notice,
 *    this list of conditions and the following disclaimer.
 *
 * 2. Redistributions in binary form must reproduce the above copyright notice,
 *    this list of conditions and the following disclaimer in the documentation
 *    and/or other materials provided with the distribution.
 *
 * 3. Neither the name of the copyright holder nor the names of its contributors
 *    may be used to endorse or promote products derived from this software
 *    without specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
 * LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
 * CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
 * SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
 * INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
 * CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
 * POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef LBPIMAGE_H
#define LBPIMAGE_H

#include "opencv2/opencv.hpp"

class LBPImage
{
public:
    LBPImage();
    LBPImage(const cv::Mat &img);

    void create(const cv::Mat &img);

    const cv::Mat& image() const        { return lbpImage; }
    const cv::Mat& histogram() const    { return lbpHistogram; }

    /**
     * @brief Compare two 7x7 uniform spatial histograms.
     *
     * Calculates weighted Chi square distance between given histograms.
     *
     * The following weighting is used:
     *    Weight = 4: patches 8, 9, 11, 12
     *    Weight = 2: patches 0, 6, 7, 13, 31
     *    Weight = 1: rest of the patches
     *
     *   0  1   2   3   4   5   6
     *   7  8   9   10  11  12  13
     *  14  15  16      17  18  19
     *      20  21      22  23
     *      24  25  26  27  28
     *      29  30  31  32  33
     *      34  35  36  37  38
     *
     * @param lbpHistogram1     A uniform spatial histogram to compare.
     * @param lbpHistogram2     A uniform spatial histogram to compare.
     * @return float    A weighted Chi square distance between given histograms.
     */
    static float distance(const cv::Mat &lbpHistogram1, const cv::Mat &lbpHistogram2);

private:
    /**
     * @brief Calculate LBP image from the given source image.
     *
     * NOTE: This function uses so called extended LBP, which uses neighborhood of
     * the given size.
     *
     * @param img               The source image.
     * @param radius            Radius (in pixels).
     * @param samplingPoints    Number of sampling points on a circle of radius.
     * @return cv::Mat          The LBP image.
     */
    cv::Mat calcExtendedLBP(const cv::Mat &img, const int radius, const int samplingPoints);

    /**
     * @brief Create 7x7 uniform spatial histogram.
     *
     * Create uniform spatial histogram from the given image, using patch size
     * of 7x7 and 59 bins per patch. 10 patches are left out (because their
     * weight is zero), so total number of patches is 7 * 7 - 10 = 39. Total
     * size of the histogram is then 39 * 59 = 2301 bytes.
     *
     * Check the following paper for more information:
     *     Ahonen, T., Hadid, A. and Pietikäinen, M. (2006), Face Description
     *     with Local Binary Patterns: Application to Face Recognition.
     *     http://www.ee.oulu.fi/mvg/files/pdf/pdf_730.pdf
     *
     * Patches that are removed are the following (see the patch order below):
     *   Nose area: 17, 24
     *   Left side: 21, 28, 35, 42
     *   Right side: 27, 34, 41, 48
     *
     * Original patch order:
     *
     *   0  1   2   3   4   5   6
     *   7  8   9   10  11  12  13
     *  14  15  16 (17) 18  19  20
     * (21) 22  23 (24) 25  26 (27)
     * (28) 29  30  31  32  33 (34)
     * (35) 36  37  38  39  40 (41)
     * (42) 43  44  45  46  47 (48)
     *
     * New patch order (after patches are removed):
     *
     *   0  1   2   3   4   5   6
     *   7  8   9   10  11  12  13
     *  14  15  16      17  18  19
     *      20  21      22  23
     *      24  25  26  27  28
     *      29  30  31  32  33
     *      34  35  36  37  38
     *
     * @param img       A grayscale and 8-bit image.
     * @return cv::Mat  The histogram in 1x2301 matrix.
     */
    cv::Mat calcSpatialHistogram(const cv::Mat &img);

private:
    cv::Mat lbpImage;
    cv::Mat lbpHistogram;

};

#endif // LBPIMAGE_H
