#include "VideoFrame.h"
#include <base/Svar/Svar.h>

SPtr<GSLAM::Vocabulary> BOW_Object::mpORBvocabulary;

float VideoFrame::GridWidthInv=0;
float VideoFrame::GridHeightInv;
int   VideoFrame::mnScaleLevels;
float VideoFrame::mfScaleFactor;
std::vector<float> VideoFrame::mvScaleFactors,
VideoFrame::mvLevelSigma2,VideoFrame::mvInvLevelSigma2;
int VideoFrame::mnMinX,VideoFrame::mnMaxX,VideoFrame::mnMinY,VideoFrame::mnMaxY;

using namespace std;

VideoFrame::VideoFrame()
{
}

vector<size_t> VideoFrame::GetFeaturesInArea(const float &x, const float  &y, const float  &r,
                                             const int minLevel, const int maxLevel) const
{
    vector<size_t> vIndices;

    int nMinCellX = floor((x-mnMinX-r)*GridWidthInv);
    nMinCellX = max(0,nMinCellX);
    if(nMinCellX>=FRAME_GRID_COLS)
        return vIndices;

    int nMaxCellX = ceil((x-mnMinX+r)*GridWidthInv);
    nMaxCellX = min(FRAME_GRID_COLS-1,nMaxCellX);
    if(nMaxCellX<0)
        return vIndices;

    int nMinCellY = floor((y-mnMinY-r)*GridHeightInv);
    nMinCellY = max(0,nMinCellY);
    if(nMinCellY>=FRAME_GRID_ROWS)
        return vIndices;

    int nMaxCellY = ceil((y-mnMinY+r)*GridHeightInv);
    nMaxCellY = min(FRAME_GRID_ROWS-1,nMaxCellY);
    if(nMaxCellY<0)
        return vIndices;

    vIndices.reserve(mKeyPoints.size());

    bool bCheckLevels=true;
    bool bSameLevel=false;
    if(minLevel==-1 && maxLevel==-1)
        bCheckLevels=false;
    else
        if(minLevel==maxLevel)
            bSameLevel=true;

    for(int ix = nMinCellX; ix<=nMaxCellX; ix++)
    {
        for(int iy = nMinCellY; iy<=nMaxCellY; iy++)
        {
            vector<size_t> vCell = mGrid[ix][iy];
            if(vCell.empty())
                continue;

            for(size_t j=0, jend=vCell.size(); j<jend; j++)
            {
                if(vCell[j]>=mKeyPoints.size()) continue;
                const cv::KeyPoint &kpUn = mKeyPoints[vCell[j]];
                if(bCheckLevels && !bSameLevel)
                {
                    if(kpUn.octave<minLevel || kpUn.octave>maxLevel)
                        continue;
                }
                else if(bSameLevel)
                {
                    if(kpUn.octave!=minLevel)
                        continue;
                }

                if(abs(kpUn.pt.x-x)>r || abs(kpUn.pt.y-y)>r)
                    continue;

                vIndices.push_back(vCell[j]);
            }
        }
    }

    return vIndices;
}

void VideoFrame::computeStatics()
{

    mnMinX=0;
    mnMaxX=img.cols;
    mnMinY=0;
    mnMaxY=img.rows;
    //Grid value
    GridWidthInv=(double)FRAME_GRID_COLS/(double)(mnMaxX-mnMinX);
    GridHeightInv=(double)FRAME_GRID_ROWS/(double)(mnMaxY-mnMinY);

    //Scale levels info
    mnScaleLevels = svar.GetInt("ORBextractor.nLevels",8);
    mfScaleFactor = svar.GetDouble("ORBextractor.scaleFactor",1.2);

    mvScaleFactors.resize(mnScaleLevels);
    mvLevelSigma2.resize(mnScaleLevels);
    mvScaleFactors[0]=1.0f;
    mvLevelSigma2[0]=1.0f;
    for(int i=1; i<mnScaleLevels; i++)
    {
        mvScaleFactors[i]=mvScaleFactors[i-1]*mfScaleFactor;
        mvLevelSigma2[i]=mvScaleFactors[i]*mvScaleFactors[i];
    }

    mvInvLevelSigma2.resize(mvLevelSigma2.size());
    for(int i=0; i<mnScaleLevels; i++)
        mvInvLevelSigma2[i]=1/mvLevelSigma2[i];
}


void VideoFrame::extractorFeatures(ORBextractor& extractor)
{
    if(GridWidthInv==0)
    {
        computeStatics();
    }

    if(img_gray.empty())
        cv::cvtColor(img,img_gray,CV_BGR2GRAY);

    extractor(img_gray,cv::Mat(),mKeyPoints,mDescriptors);

    // Assign Features to Grid Cells
    int nReserve = 0.5*mKeyPoints.size()/(FRAME_GRID_COLS*FRAME_GRID_ROWS);
    for(unsigned int i=0; i<FRAME_GRID_COLS;i++)
        for (unsigned int j=0; j<FRAME_GRID_ROWS;j++)
            mGrid[i][j].reserve(nReserve);


    for(size_t i=0,iend=mKeyPoints.size();i<iend;i++)
    {
        cv::KeyPoint &kp = mKeyPoints[i];

        int posX, posY;
        posX = round((kp.pt.x-mnMinX)*GridWidthInv);
        posY = round((kp.pt.y-mnMinY)*GridHeightInv);

        //Keypoint's coordinates are undistorted, which could cause to go out of the image
        if(posX<0 || posX>=FRAME_GRID_COLS || posY<0 || posY>=FRAME_GRID_ROWS)
            continue;
        mGrid[posX][posY].push_back(i);
    }
}
