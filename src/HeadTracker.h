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

#ifndef HEADTRACKER_H
#define HEADTRACKER_H

#include "opencv2/opencv.hpp"

class HeadTracker
{
public:
    HeadTracker()             {}
    virtual ~HeadTracker()    {}

    virtual bool init()     { return true; }
    virtual bool reset()    { return true; }

    virtual bool track(cv::Mat frame) = 0;

    float getPitch() const { return pitch; }
    float getYaw() const   { return yaw; }
    float getRoll() const  { return roll; }

    const cv::Point2f& getLeftEye() const    { return leftEye; }
    const cv::Point2f& getRightEye() const   { return rightEye; }
    const cv::Point2f& getAlignedLeftEye() const    { return alignedLeftEye; }
    const cv::Point2f& getAlignedRightEye() const   { return alignedRightEye; }
    const cv::Mat& getFacialLandmarks() const { return facialLandmarks; }
    const cv::Mat& getAlignedFacialLandmarks() const { return alignedFacialLandmarks; }
    const cv::Mat& getFaceROI() const { return faceROI; }
    const cv::Mat& getFaceImage() const { return faceImage; }
    const cv::Mat& getAlignedFaceImage() const { return alignedFaceImage; }

protected:
    void doPostprocessing(const cv::Mat &frame);


    /**
     * @brief Resolve face ROI.
     *
     * Returns coordinates of a region of interest (ROI) in given image that
     * contains a face.
     * Note that the returned ROI is quadrangle, not rectangle.
     *
     * @param imageSize The size of the source image, where the face is located.
     * @param leftEye   The center point of the left eye.
     * @param rightEye  The center point of the right eye.
     * @param pitch     Pitch of the head in degrees.
     * @param yaw       Yaw of the head in degrees.
     * @param roll      Roll of the head in degrees.
     * @return cv::Mat  A 4x1 matrix containing vertices of the face ROI.
     */
    cv::Mat resolveFaceROI(const cv::Size imageSize, cv::Point2f leftEye, cv::Point2f rightEye, const float pitch, const float yaw, const float roll);

    /**
     * @brief Get head plane transformation matrix.
     *
     * Returns transformation matrix of the head plane. It is calculated based on
     * eye positions and 3D head pose estimation.
     *
     * @param imageSize Size of the image.
     * @param leftEye   The center point of the left eye.
     * @param rightEye  The center point of the right eye.
     * @param pitch     Pitch of the head in degrees.
     * @param yaw       Yaw of the head in degrees.
     * @param roll      Roll of the head in degrees.
     * @return cv::Mat  A 4x4 matrix.
     */
    cv::Mat getHeadPlaneTransformation(cv::Size imageSize, cv::Point2f leftEye, cv::Point2f rightEye, const float pitch, const float yaw, const float roll);

    cv::Mat alignLandmarks(const cv::Mat &landmarks, const cv::Mat &ROI);

    /**
     * @brief Align face.
     *
     * Returns image containing aligned face. Alignment is done using perspective
     * transformation. The transformation matrix is calculated based on the given
     * face ROI in source image.
     *
     * @param img       The source image, where the face is located.
     * @param ROI       A 4x1 matrix containing vertices of the face ROI in source
     *                  image.
     * @param imageSize The size of the final aligned face image (in pixels).
     * @return cv::Mat  Aligned face image.
     */
    cv::Mat alignFace(const cv::Mat &img, const cv::Mat &ROI, const cv::Size imageSize);

protected:
    float pitch;
    float yaw;
    float roll;

    cv::Point2f leftEye;
    cv::Point2f rightEye;
    cv::Point2f alignedLeftEye;
    cv::Point2f alignedRightEye;

    // NxPoint matrix containing landmark points:
    // Example:
    // [ point0 ]
    // [ point1 ]
    // [   ..   ]
    // [ PointN ]
    cv::Mat facialLandmarks;
    cv::Mat alignedFacialLandmarks;

    // Quadrangle around the face.
    cv::Mat faceROI;

    cv::Mat faceImage;
    cv::Mat alignedFaceImage;

};

#endif // HEADTRACKER_H
