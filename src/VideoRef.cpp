#include "VideoRef.h"
#include <vector>

VideoRef::VideoRef(const std::string& path)
    :dataPath(path),nowId(0)
{

}

void VideoRef::addKeyframe(SPtr<VideoFrame>& frame)
{
    pi::WriteMutex lock(mutex);
    frame->ComputeBoW();
    frame->id=nowId++;
    dbow.add(frame->id,frame->mBowVec);
    std::cout<<"added keyframe "<<nowId<<std::endl;
    frames.push_back(frame);
}

bool VideoRef::loadFromFile(const std::string& path)
{
    return false;
}

bool VideoRef::save2File(const std::string& path)
{
    return false;
}
