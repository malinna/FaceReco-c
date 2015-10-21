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

#include "LBPImage.h"
#include <limits>

const int NUM_PATTERNS = 59;
const int GRID_X = 7;
const int GRID_Y = 7;
const int NUM_PATCHES = GRID_X * GRID_Y - 10;
const int EXTENDED_LBP_RADIUS = 2;
const int EXTENDED_LBP_SAMPLING_POINTS = 8;

// The original uniform2 pattern.
int UNIFORM_PATTERN[256] =
{
   0,1,2,3,4,58,5,6,7,58,58,58,8,58,9,10,11,58,58,58,58,58,58,58,12,58,58,58,13,58,
   14,15,16,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,17,58,58,58,58,58,58,58,18,
   58,58,58,19,58,20,21,22,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
   58,58,58,58,58,58,58,58,58,58,58,58,23,58,58,58,58,58,58,58,58,58,58,58,58,58,
   58,58,24,58,58,58,58,58,58,58,25,58,58,58,26,58,27,28,29,30,58,31,58,58,58,32,58,
   58,58,58,58,58,58,33,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,34,58,58,58,58,
   58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,58,
   58,35,36,37,58,38,58,58,58,39,58,58,58,58,58,58,58,40,58,58,58,58,58,58,58,58,58,
   58,58,58,58,58,58,41,42,43,58,44,58,58,58,45,58,58,58,58,58,58,58,46,47,48,58,49,
   58,58,58,50,51,52,58,53,54,55,56,57
};

// The weight map.
int WEIGHT_MAP[39] =
    { 2, 1, 1, 1, 1, 1, 2,
      2, 4, 4, 1, 4, 4, 2,
      1, 1, 1,    1, 1, 1,
         1, 1,    1, 1,
         1, 1, 1, 1, 1,
         1, 1, 2, 1, 1,
         1, 1, 1, 1, 1};

LBPImage::LBPImage()
{
}

LBPImage::LBPImage(const cv::Mat &img)
{
    create(img);
}

void LBPImage::create(const cv::Mat &img)
{
    if (img.empty() || img.type() != CV_8UC1)
    {
        return;
    }

    lbpImage = calcExtendedLBP(img, EXTENDED_LBP_RADIUS, EXTENDED_LBP_SAMPLING_POINTS);
    lbpHistogram = calcSpatialHistogram(lbpImage);
}

float LBPImage::distance(const cv::Mat &lbpHistogram1, const cv::Mat &lbpHistogram2)
{
    if (lbpHistogram1.cols != 2301 || lbpHistogram2.cols != 2301 ||
        lbpHistogram1.rows != 1 || lbpHistogram2.rows != 1)
    {
        return std::numeric_limits<float>::max();
    }

    float distance = 0.0;
    int index = 0;
    for (int i = 0; i < NUM_PATCHES; i++)
    {
        const float weight = static_cast<float>(WEIGHT_MAP[i]);
        for (int j = 0; j < NUM_PATTERNS; j++)
        {
            const float v1 = lbpHistogram1.at<float>(index);
            const float v2 = lbpHistogram2.at<float>(index);
            const float sum = v1 + v2;
            if (sum > 0.0)
            {
                float diff = v1 - v2;
                distance += weight * (diff * diff) / sum;
            }

            index++;
        }
    }

    return distance;
}

cv::Mat LBPImage::calcExtendedLBP(const cv::Mat &img, const int radius, const int samplingPoints)
{
    cv::Mat result = cv::Mat::zeros(img.rows - 2 * radius, img.cols - 2 * radius, CV_8UC1);

    for (int n = 0; n < samplingPoints; n++)
    {
        const float x = static_cast<float>(cos(2.0 * CV_PI * n / samplingPoints) * radius);
        const float y = static_cast<float>(sin(2.0 * CV_PI * n / samplingPoints) * -radius);

        const int fx = static_cast<int>(floor(x));
        const int fy = static_cast<int>(floor(y));
        const int cx = static_cast<int>(ceil(x));
        const int cy = static_cast<int>(ceil(y));

        const float ty = y - fy;
        const float tx = x - fx;

        const float w1 = (1 - tx) * (1 - ty);
        const float w2 = tx  * (1 - ty);
        const float w3 = (1 - tx) * ty;
        const float w4 = tx  * ty;

        for (int i = radius; i < img.rows - radius; i++)
        {
            for (int j = radius; j < img.cols - radius; j++)
            {
                const float t =
                    w1 * img.at<unsigned char>(i + fy, j + fx) +
                    w2 * img.at<unsigned char>(i + fy, j + cx) +
                    w3 * img.at<unsigned char>(i + cy, j + fx) +
                    w4 * img.at<unsigned char>(i + cy, j + cx);

                result.at<unsigned char>(i - radius, j - radius) += ((t > img.at<unsigned char>(i, j)) || (std::abs(t - img.at<unsigned char>(i, j)) < std::numeric_limits<float>::epsilon())) << n;
            }
        }
    }

    return result;
}

cv::Mat LBPImage::calcSpatialHistogram(const cv::Mat &img)
{
    cv::Mat result = cv::Mat::zeros(1, NUM_PATCHES * NUM_PATTERNS, CV_32FC1);

    const int width = img.cols / GRID_X;
    const int height = img.rows / GRID_Y;

    int patchIndex = 0;
    for (int i = 0; i < GRID_Y; i++)
    {
        for(int j = 0; j < GRID_X; j++)
        {
            if (((j == 0 || j == 6) && (i == 3 || i == 4 || i == 5 || i == 6)) ||
                ((j == 3 && (i == 2 || i == 3))))
            {
                continue;
            }

            const cv::Mat patch = cv::Mat(img, cv::Range(i * height, (i + 1) * height), cv::Range(j * width, (j + 1) * width));
            const int histogramIndex = patchIndex * NUM_PATTERNS;

            for (int px = 0; px < patch.cols; px++)
            {
                for (int py = 0; py < patch.rows; py++)
                {
                    result.at<float>(histogramIndex + UNIFORM_PATTERN[patch.at<uchar>(py, px)])++;
                }
            }

            patchIndex++;
        }
    }

    // Normalize.
    result /= static_cast<float>(img.total());

    return result;
}
