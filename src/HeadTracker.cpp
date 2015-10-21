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

#include "HeadTracker.h"
#include "Util.h"
#include "Constants.h"

using namespace cv;
using namespace FaceReco;

void HeadTracker::doPostprocessing(const Mat &frame)
{
    faceROI = resolveFaceROI(frame.size(), leftEye, rightEye, pitch, yaw, roll);

    faceImage = imGetSubImage(frame, faceROI);

    // Align the face to get rid of the yaw, pitch and roll of the face (at
    // least to some extent).
    alignedFacialLandmarks = alignLandmarks(facialLandmarks, faceROI);
    alignedFaceImage = alignFace(frame, faceROI, ALIGNED_FACE_IMAGE_SIZE);
}

Mat HeadTracker::resolveFaceROI(const Size imageSize, Point2f leftEye, Point2f rightEye, const float pitch, const float yaw, const float roll)
{
    // Distance between eyes.
    const float D = euclideanDist(leftEye, rightEye);

    Point2f center(imageSize.width / 2, imageSize.height / 2);
    Point2f p1(center.x - D * LEFT_DISTANCE_FACTOR, center.y - D * UP_DISTANCE_FACTOR);
    Point2f p2(center.x + D * RIGHT_DISTANCE_FACTOR, center.y + D * DOWN_DISTANCE_FACTOR);

    Mat ROI;
    Mat normalizedROI = (Mat_<Vec2f>(4, 1) <<
        Vec2f(p1.x, p1.y),
        Vec2f(p2.x, p1.y),
        Vec2f(p2.x, p2.y),
        Vec2f(p1.x, p2.y));

    Mat T = getHeadPlaneTransformation(imageSize, leftEye, rightEye, pitch, yaw, roll);

    perspectiveTransform(normalizedROI, ROI, T);

    return ROI;
}

Mat HeadTracker::getHeadPlaneTransformation(Size imageSize, Point2f leftEye, Point2f rightEye, const float pitch, const float yaw, const float roll)
{
    const Point2f middleOfEyes((leftEye.x + rightEye.x) / 2, (leftEye.y + rightEye.y) / 2);
    const Point2f center(imageSize.width / 2, imageSize.height / 2);
    const double F = 600;
    const double a = pitch * CV_PI / 180;
    const double b = yaw * CV_PI / 180;
    const double c = roll * CV_PI / 180;
    const double dx = middleOfEyes.x - center.x;
    const double dy = middleOfEyes.y - center.y;
    const double dz = F;

    // Projection matrix (from 2D to 3D).
    Mat A1 = (Mat_<double>(4,3) <<
              1, 0, -imageSize.width / 2,
              0, 1, -imageSize.height / 2,
              0, 0, 0,
              0, 0, 1);

    // Rotation matrices.
    Mat RX = (Mat_<double>(4, 4) <<
              1, 0,       0,      0,
              0, cos(a), -sin(a), 0,
              0, sin(a),  cos(a), 0,
              0, 0,       0,      1);
    Mat RY = (Mat_<double>(4, 4) <<
              cos(b), 0, -sin(b), 0,
              0,      1, 0,       0,
              sin(b), 0,  cos(b), 0,
              0,      0,  0,      1);
    Mat RZ = (Mat_<double>(4, 4) <<
              cos(c), -sin(c), 0, 0,
              sin(c),  cos(c), 0, 0,
              0,       0,      1, 0,
              0,       0,      0, 1);
    Mat R = RX * RY * RZ;

    // Translation matrix.
    Mat T = (Mat_<double>(4, 4) <<
        1, 0, 0, dx,
        0, 1, 0, dy,
        0, 0, 1, dz,
        0, 0, 0, 1);

    // Projection matrix (from 3D to 2D).
    Mat A2 = (Mat_<double>(3,4) <<
              F, 0, imageSize.width / 2,  0,
              0, F, imageSize.height / 2, 0,
              0, 0, 1,                    0);

    // Final transformation matrix.
    return A2 * (T * (R * A1));
}

Mat HeadTracker::alignLandmarks(const Mat &landmarks, const Mat &ROI)
{
    Point2f srcQuad[4];
    Point2f dstQuad[4];

    srcQuad[0] = ROI.at<Point2f>(0);
    dstQuad[0] = Point2f(0, 0);
    srcQuad[1] = ROI.at<Point2f>(1);
    dstQuad[1] = Point2f(1, 0);
    srcQuad[2] = ROI.at<Point2f>(2);
    dstQuad[2] = Point2f(1, 1);
    srcQuad[3] = ROI.at<Point2f>(3);
    dstQuad[3] = Point2f(0, 1);

    Mat T = getPerspectiveTransform(srcQuad, dstQuad);
    Mat alignedLandmarks;

    perspectiveTransform(landmarks, alignedLandmarks, T);

    return alignedLandmarks;
}

Mat HeadTracker::alignFace(const Mat &img, const Mat &ROI, const Size imageSize)
{
    Point2f srcQuad[4];
    Point2f dstQuad[4];

    srcQuad[0] = ROI.at<Point2f>(0);
    dstQuad[0] = Point2f(0, 0);
    srcQuad[1] = ROI.at<Point2f>(1);
    dstQuad[1] = Point2f(imageSize.width - 1, 0);
    srcQuad[2] = ROI.at<Point2f>(2);
    dstQuad[2] = Point2f(imageSize.width - 1, imageSize.height - 1);
    srcQuad[3] = ROI.at<Point2f>(3);
    dstQuad[3] = Point2f(0, imageSize.height - 1);

    Mat T = getPerspectiveTransform(srcQuad, dstQuad);
    Mat alignedFaceImg(imageSize, img.type());

    warpPerspective(img, alignedFaceImg, T, imageSize);

    return alignedFaceImg;
}
