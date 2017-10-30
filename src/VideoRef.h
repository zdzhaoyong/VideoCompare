#ifndef VIDEOREF_H
#define VIDEOREF_H
#include "VideoFrame.h"
#include "KeyFrameDatabase.h"

class VideoRef
{
public:
    VideoRef(const std::string& path);

    void addKeyframe(SPtr<VideoFrame>& frame);

    bool loadFromFile(const std::string& path);
    bool save2File(const std::string& path);

    pi::MutexRW                mutex;
    std::vector<SPtr<VideoFrame> >  frames;     // keyframes for normal tracking
    KeyFrameDatabase           dbow;       // for relocalization
    std::string                dataPath;   // where to save all the data
    FrameID                    nowId;
};

#endif // VIDEOREF_H
