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

#ifndef UTIL_H
#define UTIL_H

#include "opencv2/opencv.hpp"
#include <QString>

namespace FaceReco {

/******************************************************************************
 **
 ***  Image manipulation
 **
 ******************************************************************************/

void imPlotFPS(cv::Mat &img, int FPS);
void imPlotPose(cv::Mat &img, const float pitch, const float yaw, const float roll);
void imPlotLandmarks(cv::Mat &img, const cv::Mat &landmarks);
void imPlotROI(cv::Mat &img, const cv::Mat &quadrangle, int index=-1, QString hint=QString());
void imPlotCenteredText(cv::Mat &img, cv::Rect &r, const QString &text, double scale, int fontFace, cv::Scalar textColor, cv::Scalar backgroundColor);

/**
 * @brief Plot a sub-frame.
 *
 * Plot the given frame to the grid-shaped image, replacing the old frame (if any).
 *
 * @param img               The image where the frame is plotted.
 * @param frame             The frame image to be plotted.
 * @param keyFrameIndex     A zero-based position index of where the frame is plotted.
 * @param trackIndex        The index of the track where the frame belongs (optional)
 * @param frameIndex        The index of the frame (optional)
 * @param histogramDistance A distance of LBP histograms between this frame and the last
 *                          key frame (optional).
 * @param maxDelta          The max delta vector size between this frame and the last
 *                          key frame (optional).
 * @param border            Border size (default: 2)
 */
void imPlotFrame(cv::Mat &img, const cv::Mat &frame, int keyFrameIndex, int trackIndex=-1, int frameIndex=-1, float histogramDistance=-1.0f, double maxDelta=-1.0, int border=2);

void imPlotStatus(cv::Mat &img, const QString &status, int keyFrameIndex);
void imPlotGrid(const cv::Mat &img, int gridX, int gridY);

cv::Mat imGetSubImage(const cv::Mat &img, const cv::Mat &quadrangle);
cv::Mat imCreateImageFromLandmarks(const cv::Mat &landmarks, cv::Point2f leftEye, cv::Point2f rightEye, cv::Size imageSize);
cv::Mat imCreateImageFromDeltaVector(const cv::Mat &landmarks, cv::Point2f leftEye, cv::Point2f rightEye, const cv::Mat &delta, cv::Size imageSize);

/******************************************************************************
 **
 ***  Geometry
 **
 ******************************************************************************/

/**
 * @brief Calculate euclidean distance between two points.
 *
 * @param p1        Point 1.
 * @param p2        Point 2
 * @return float    Euclidean distance.
 */
float euclideanDist(cv::Point2f &p1, cv::Point2f &p2);

/**
 * @brief Calculate center point of a point group.
 *
 * Points must be given in Nx1 2-channel matrix so that channel 0 is x
 * coordinate and channel 1 is y coordinate of the point.
 *
 * @param points    A point group.
 * @return Point2f  The center point.
 */
cv::Point2f centerPoint(cv::Mat &points);

/**
 * @brief Get length of the longest vector in given matrix.
 *
 * Vectors are passed in a Nx1 2-channel format (channel 0 is x and channel 1 is
 * y).
 *
 * @param v         The vectors.
 * @return double   The length of the longest vector.
 */
double maxVectorLength(cv::Mat &v);

}

#endif // UTIL_H
