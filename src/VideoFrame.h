#ifndef VIDEOFRAME_H
#define VIDEOFRAME_H
#include "BOW_Object.h"
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/features2d/features2d.hpp"
#include "ORBextractor.h"

#define FRAME_GRID_ROWS 48
#define FRAME_GRID_COLS 64
typedef uint FrameID;

class VideoFrame : public BOW_Object
{
public:
    VideoFrame();
    void extractorFeatures(ORBextractor& extractor);
    void ComputeBoW(){BOW_Object::ComputeBoW(mDescriptors);}

    std::vector<size_t> GetFeaturesInArea(const float &x, const float  &y, const float  &r,
                                     const int minLevel=-1, const int maxLevel=-1) const;
    int GetKeyPointScaleLevel(const size_t &idx) const{return mKeyPoints[idx].octave;}

    // Scale functions
    float inline GetScaleFactor(int nLevel=1) const{return mvScaleFactors[nLevel];}
    std::vector<float> inline GetScaleFactors() const{ return mvScaleFactors;}
    std::vector<float> inline GetVectorScaleSigma2() const{ return mvLevelSigma2;}
    float inline GetSigma2(int nLevel=1) const{ return mvLevelSigma2[nLevel];}
    float inline GetInvSigma2(int nLevel=1) const{ return mvInvLevelSigma2[nLevel];}
    int   inline GetScaleLevels() const{return mnScaleLevels;}

protected:
    void computeStatics();

public:
    //once seted, never change, don't need mutex
    FrameID id;//FrameCount in tracking and MapFrame id after inserted to Map
    double  mTimestamp;
    cv::Mat img,img_gray;
    std::vector<cv::KeyPoint> mKeyPoints;
    cv::Mat mDescriptors;
    cv::Mat H,H2Last;

    static float GridWidthInv,GridHeightInv;
    std::vector<std::size_t> mGrid[FRAME_GRID_COLS][FRAME_GRID_ROWS];

    // Scale Pyramid Info
    static int   mnScaleLevels;// default:8
    static float mfScaleFactor;// default:1.2
    static std::vector<float> mvScaleFactors;// default: 1.2^level
    static std::vector<float> mvLevelSigma2;// default: 1.2^(2*level)
    static std::vector<float> mvInvLevelSigma2;// default:1.2^(-2*level)

    // Undistorted Image Bounds (computed once)
    static int mnMinX;
    static int mnMaxX;
    static int mnMinY;
    static int mnMaxY;
};

#endif // VIDEOFRAME_H
