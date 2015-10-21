#-------------------------------------------------
#
# Project created by QtCreator 2015-01-16T15:23:06
#
#-------------------------------------------------

QT       += core gui
greaterThan(QT_MAJOR_VERSION, 4): QT += widgets

TARGET = FaceReco
CONFIG   += console
CONFIG   -= app_bundle

TEMPLATE = app

# Define OpenCV's root path here.
# Also define separately Haar Cascade path and model name.
OPENCV_ROOT = C:/OpenCV_2.4.9
OPENCV_HAAR_CASCADE_PATH = $${OPENCV_ROOT}/opencv/sources/data/haarcascades
OPENCV_HAAR_CASCADE = haarcascade_frontalface_alt_tree.xml

# Define Chehra v0.2 root path here.
CHEHRA_ROOT = C:/chehra_v0.2
CHEHRA_MODEL = Chehra_t1.0.model

HEADERS += \
    CaptureSource.h \
    Util.h \
    Database.h \
    Track.h \
    Constants.h \
    MainWindow.h \
    FrameProcesser.h \
    SearchEngine.h \
    Person.h \
    HistogramWriter.h \
    HeadTracker.h \
    ChehraHeadTracker.h \
    LBPImage.h

SOURCES += main.cpp \
    CaptureSource.cpp \
    Util.cpp \
    Database.cpp \
    Track.cpp \
    MainWindow.cpp \
    FrameProcesser.cpp \
    SearchEngine.cpp \
    Person.cpp \
    HistogramWriter.cpp \
    HeadTracker.cpp \
    ChehraHeadTracker.cpp \
    LBPImage.cpp

FORMS += \
    MainWindow.ui

INCLUDEPATH += $${CHEHRA_ROOT}/include
INCLUDEPATH += $${OPENCV_ROOT}/opencv/build/include

win32 {
    CONFIG( debug, debug|release ) {
        # debug
        LIBS += -L"$${OPENCV_ROOT}/opencv/build/x64/vc11/lib" -lopencv_core249d -lopencv_highgui249d -lopencv_imgproc249d -lopencv_objdetect249d
        LIBS += -L"$${CHEHRA_ROOT}/Lib/x64//VS2012" -lChehra_d

        DEST_DIR = $${OUT_PWD}/debug

        # Copy Qt DLLs to build dir.
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/Qt5Cored.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/Qt5Cored.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/Qt5Guid.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/Qt5Guid.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/Qt5Widgetsd.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/Qt5Widgetsd.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))

        # Copy OpenCV DLLs to build dir.
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_core249d.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_core249d.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_highgui249d.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_highgui249d.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_imgproc249d.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_imgproc249d.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_objdetect249d.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_objdetect249d.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))

    } else {
        # release
        LIBS += -L"$${OPENCV_ROOT}/opencv/build/x64/vc11/lib" -lopencv_core249 -lopencv_highgui249 -lopencv_imgproc249 -lopencv_objdetect249
        LIBS += -L"$${CHEHRA_ROOT}/Lib/x64//VS2012" -lChehra_r

        DEST_DIR = $${OUT_PWD}/release

        # Copy Qt DLLs to build dir.
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/Qt5Core.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/Qt5Core.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/Qt5Gui.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/Qt5Gui.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/Qt5Widgets.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/Qt5Widgets.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))

        # Copy OpenCV DLLs to build dir.
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_core249.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_core249.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_highgui249.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_highgui249.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_imgproc249.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_imgproc249.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
        QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/opencv_objdetect249.dll)) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_ROOT}/opencv/build/x64/vc11/bin/opencv_objdetect249.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
    }

    # Copy ICU DLLs to build dir.
    QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/icudt*.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/icudt*.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/icuin*.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/icuin*.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
    QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/icuuc*.dll)) $$quote($$QMAKE_COPY $$shell_path($(QTDIR)/bin/icuuc*.dll) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
}

# Copy Chehra and Haar Cascade models to build dir.
QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/$${CHEHRA_MODEL})) $$quote($$QMAKE_COPY $$shell_path($${CHEHRA_ROOT}/Data/model/$${CHEHRA_MODEL}) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
QMAKE_POST_LINK += $$sprintf($$QMAKE_CHK_EXISTS, $$shell_path($${DEST_DIR}/$${OPENCV_HAAR_CASCADE})) $$quote($$QMAKE_COPY $$shell_path($${OPENCV_HAAR_CASCADE_PATH}/$${OPENCV_HAAR_CASCADE}) $$shell_path($${DEST_DIR})$$escape_expand(\n\t))
