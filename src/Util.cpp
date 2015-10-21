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

#include "Util.h"
#include "Constants.h"
#include <QtGlobal>
#include <QDebug>

#ifdef max
#undef max
#endif

using namespace std;
using namespace cv;

namespace FaceReco {

void imPlotFPS(Mat &img, int FPS)
{
    QString str = QString("FPS: %1").arg(FPS);
    Point pos(10, 20);
    Scalar color(0,255,255);
    int fontFace = CV_FONT_HERSHEY_DUPLEX;
    double fontScale = 0.4;

    putText(img, str.toStdString(), pos, fontFace, fontScale, color, 1, CV_AA);
}

void imPlotPose(Mat &img, const float pitch, const float yaw, const float roll)
{
    QString str1 = QString("Pitch: %1").arg((int)(pitch + 0.5));
    QString str2 = QString("Yaw: %1").arg((int)(yaw + 0.5));
    QString str3 = QString("Roll: %1").arg((int)(roll + 0.5));
    Point pos1(10, 40);
    Point pos2(10, 60);
    Point pos3(10, 80);
    Scalar color(0,255,255);
    int fontFace = CV_FONT_HERSHEY_DUPLEX;
    double fontScale = 0.4;

    putText(img, str1.toStdString(), pos1, fontFace, fontScale, color, 1, CV_AA);
    putText(img, str2.toStdString(), pos2, fontFace, fontScale, color, 1, CV_AA);
    putText(img, str3.toStdString(), pos3, fontFace, fontScale, color, 1, CV_AA);
}

void imPlotLandmarks(Mat &img, const Mat &landmarks)
{
    Scalar c = CV_RGB(0,255,0);

    for (int i = 0; i < landmarks.rows; i++)
    {
        Point2f p = landmarks.at<Point2f>(i);
        circle(img, p, 1, c, -1);

#ifdef DISPLAY_LANDMARK_LABELS
        putText(img, to_string(i), p + Point2f(2, 0), FONT_HERSHEY_SIMPLEX, 0.3f, c);
#endif
    }
}

void imPlotROI(Mat &img, const Mat &quadrangle, int index, QString hint)
{
    Scalar c = CV_RGB(255,255,0);
    Point2f p1;
    Point2f p2;

    // Draw the quadrangle.
    for (int i = 0; i < 4; i++)
    {
        p1 = quadrangle.at<Point2f>(i);
        p2 = quadrangle.at<Point2f>((i + 1) % 4);
        line(img, p1, p2, c);
    }

    if (index != -1)
    {
        // Print label to the middle left of the ROI.
        p1.x = ((quadrangle.at<Point2f>(0).x + quadrangle.at<Point2f>(3).x) / 2) - 14;
        p1.y = ((quadrangle.at<Point2f>(0).y + quadrangle.at<Point2f>(3).y) / 2) - 8;
        p2.x = p1.x + 28;
        p2.y = p1.y + 16;
        imPlotCenteredText(img, Rect(p1, p2), QString::number(index), 0.4, FONT_HERSHEY_SIMPLEX, CV_RGB(0,0,255), c);
    }

    if (!hint.isEmpty())
    {
        // Print hint to the upper right corner of the ROI.
        p1.x = quadrangle.at<Point2f>(1).x - 30;
        p1.y = quadrangle.at<Point2f>(1).y - 8;
        p2.x = p1.x + 60;
        p2.y = p1.y + 16;
        imPlotCenteredText(img, Rect(p1, p2), hint, 0.34, FONT_HERSHEY_SIMPLEX, CV_RGB(0,0,255), c);
    }
}

void imPlotCenteredText(Mat &img, Rect &r, const QString &text, double scale, int fontFace, Scalar textColor, Scalar backgroundColor)
{
    int baseline = 0;
    Size textSize = getTextSize(text.toStdString(), fontFace, scale, 1, &baseline);
    Point p(r.x + (r.width - textSize.width) / 2, r.y + (r.height + textSize.height) / 2);

    rectangle(img,
              p + Point(-5, baseline - 1),
              p + Point(textSize.width + 5, -textSize.height - 2),
              backgroundColor, CV_FILLED);

    putText(img, text.toStdString(), p, fontFace, scale, textColor, 1, CV_AA);
}

void imPlotFrame(Mat &img, const Mat &frame, int keyFrameIndex, int trackIndex, int frameIndex, float histogramDistance, double maxDelta, int border)
{
    const int BORDERED_WIDTH  = img.cols / TRACK_WINDOW_GRID_X;
    const int BORDERED_HEIGHT = img.rows / TRACK_WINDOW_GRID_Y;
    const int WIDTH = BORDERED_WIDTH - border * 2;
    const int HEIGHT = BORDERED_HEIGHT - border * 2;
    const int Y = keyFrameIndex / TRACK_WINDOW_GRID_X;
    const int X = keyFrameIndex % TRACK_WINDOW_GRID_X;
    const int PX = X * BORDERED_WIDTH;
    const int PY = Y * BORDERED_HEIGHT;

    // Calculate scale factor for resize.
    double scale = (double)HEIGHT / frame.rows;
    if ((int)(scale * frame.cols + 0.5) > WIDTH)
    {
        scale = (double)WIDTH / frame.cols;
    }

    // Erase the frame area.
    Scalar c;
    c = CV_RGB(0,0,0);
    Point p1(PX, PY);
    Point p2(PX + BORDERED_WIDTH - 1, PY + BORDERED_HEIGHT - 1);
    rectangle(img, p1, p2, c, CV_FILLED);

    // Mark the first frame of a track with yellow bounding rectangle.
    if (frameIndex == 0 && keyFrameIndex >= TRACK_WINDOW_GRID_X)
    {
        c = CV_RGB(255,255,0);
        rectangle(img, p1, p2, c, 1);
    }

    // Plot the new frame data.
    int scaledFrameWidth = (int)(scale * frame.cols + 0.5);
    int scaledFrameHeight = (int)(scale * frame.rows + 0.5);
    Mat s;
    if (frame.channels() == 1)
    {
        cvtColor(frame, s, CV_GRAY2BGR);
    }
    else
    {
        s = frame;
    }
    resize(s, s, Size(scaledFrameWidth, scaledFrameHeight));
    int dx = (BORDERED_WIDTH - scaledFrameWidth) / 2;
    int dy = (BORDERED_HEIGHT - scaledFrameHeight) / 2;
    s.copyTo(img(Rect(PX + dx, PY + dy, scaledFrameWidth, scaledFrameHeight)));

#ifdef DISPLAY_FRAME_INFO
    c = CV_RGB(255,255,0);
    p1 = Point(PX + 4, PY + BORDERED_HEIGHT - 36);
    if (histogramDistance != -1.0)
    {
        putText(img, string("Distance: ") + to_string(histogramDistance), p1, FONT_HERSHEY_SIMPLEX, 0.34f, c, 1, CV_AA);
    }

    p1.y += 10;
    if (maxDelta != -1.0)
    {
        putText(img, string("Delta: ") + to_string(maxDelta), p1, FONT_HERSHEY_SIMPLEX, 0.34f, c, 1, CV_AA);
    }
    p1.y += 10;
    if (trackIndex != -1)
    {
        putText(img, string("Track: ") + to_string(trackIndex), p1, FONT_HERSHEY_SIMPLEX, 0.34f, c, 1, CV_AA);
    }
    p1.y += 10;
    if (frameIndex != -1)
    {
        putText(img, string("Frame: ") + to_string(frameIndex), p1, FONT_HERSHEY_SIMPLEX, 0.34f, c, 1, CV_AA);
    }
#endif
}

void imPlotStatus(Mat &img, const QString &status, int keyFrameIndex)
{
    const int BORDERED_WIDTH  = img.cols / TRACK_WINDOW_GRID_X;
    const int BORDERED_HEIGHT = img.rows / TRACK_WINDOW_GRID_Y;
    const int Y = keyFrameIndex / TRACK_WINDOW_GRID_X;
    const int X = keyFrameIndex % TRACK_WINDOW_GRID_X;
    const int PX = X * BORDERED_WIDTH;
    const int PY = Y * BORDERED_HEIGHT;

    // Erase the frame area.
    Scalar c = CV_RGB(0,0,0);
    Point p1(PX, PY);
    Point p2(PX + BORDERED_WIDTH - 1, PY + BORDERED_HEIGHT - 1);
    rectangle(img, p1, p2, c, CV_FILLED);

    // Print status.
    imPlotCenteredText(img, Rect(p1, p2), status, 0.4, FONT_HERSHEY_SIMPLEX, CV_RGB(255,255,0), c);
}

void imPlotGrid(const Mat &img, int gridX, int gridY)
{
    Mat modifiedImg(img);

    const int GRID_SIZE_X = modifiedImg.cols / gridX;
    const int GRID_SIZE_Y = modifiedImg.rows / gridY;
    Scalar c = CV_RGB(255,255,0);

    for (int i = 1; i < gridX; i++)
    {
        const int x = i * GRID_SIZE_X;
        Point p1(x, 0);
        Point p2(x, modifiedImg.rows - 1);
        line(modifiedImg, p1, p2, c);
    }

    for (int i = 1; i < gridY; i++)
    {
        const int y = i * GRID_SIZE_Y;
        Point p1(0, y);
        Point p2(modifiedImg.cols - 1, y);
        line(modifiedImg, p1, p2, c);
    }
}

Mat imGetSubImage(const Mat &img, const Mat &quadrangle)
{
    Rect brect = boundingRect(quadrangle);
    Point p1(brect.x, brect.y);
    Point p2(brect.x + brect.width - 1, brect.y + brect.height - 1);
    p1.x = max(p1.x, 0);
    p1.y = max(p1.y, 0);
    p2.x = min(p2.x, img.cols - 1);
    p2.y = min(p2.y, img.rows - 1);
    return img(Rect(p1, p2));
}

Mat imCreateImageFromLandmarks(const Mat &landmarks, Point2f leftEye, Point2f rightEye, Size imageSize)
{
    Mat img(imageSize, CV_8UC3);
    img = 0;

    // Use the point in the middle of the eyes as an anchor point.
    Point2f middleOfEyes((leftEye.x + rightEye.x) / 2, (leftEye.y + rightEye.y) / 2);
    Point2f translation(-middleOfEyes.x, -middleOfEyes.y);
    Point2f anchorPoint(img.cols / 2, img.rows / 3);

    // Get scale factor from distance between eyes.
    const float d1 = euclideanDist(leftEye, rightEye);
    const float d2 = img.cols / 3;
    const float scale = d2 / d1;

    // Draw the landmarks.
    Scalar c = CV_RGB(0,255,0);
    for (int i = 0; i < landmarks.rows; i++)
    {
        Point2f p1 = (landmarks.at<Point2f>(i) + translation) * scale + anchorPoint;
        circle(img, p1, 1, c, -1);
    }

    return img;
}

Mat imCreateImageFromDeltaVector(const Mat &landmarks, Point2f leftEye, Point2f rightEye, const Mat &delta, Size imageSize)
{
    Mat img(imageSize, CV_8UC3);
    img = 0;

    // Use the point in the middle of the eyes as "an anchor point".
    Point2f middleOfEyes((leftEye.x + rightEye.x) / 2, (leftEye.y + rightEye.y) / 2);
    Point2f translation(-middleOfEyes.x, -middleOfEyes.y);
    Point2f anchorPoint(img.cols / 2, img.rows / 3);

    // Get scale factor from distance between eyes.
    const float d1 = euclideanDist(leftEye, rightEye);
    const float d2 = img.cols / 3;
    const float scale = d2 / d1;

    // Draw the vectors.
    double largestDelta = 0.0;
    Scalar c = CV_RGB(255,255,0);
    for (int i = 0; i < landmarks.rows; i++)
    {
        Point2f d = delta.at<Point2f>(i);
        double norm = cv::norm(d);
        if (norm > largestDelta)
        {
            largestDelta = norm;
        }
        Point2f p1 = (landmarks.at<Point2f>(i) + translation) * scale + anchorPoint;
        Point2f p2 = p1 + d * scale;
        line(img, p1, p2, c);
    }

    Point2f p1 = Point(4, img.rows - 6);
    putText(img, string("Max length: ") + to_string(largestDelta), p1, FONT_HERSHEY_SIMPLEX, 0.36f, c, 1, CV_AA);

    return img;
}

float euclideanDist(Point2f &p1, Point2f &p2)
{
    Point2f diff = p1 - p2;
    return cv::sqrt(diff.x*diff.x + diff.y*diff.y);
}

Point2f centerPoint(Mat &points)
{
    return Point2f((float)sum(points)[0] / points.rows,
                   (float)sum(points)[1] / points.rows);
}

double maxVectorLength(Mat &v)
{
    double maxLength = 0.0f;
    for (int i = 0; i < v.rows; i++)
    {
        double norm = cv::norm(v.at<Vec2f>(i));
        maxLength = cv::max(norm, maxLength);
    }
    return maxLength;
}

}
