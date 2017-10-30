#ifndef SYSTEM_H
#define SYSTEM_H
#include "VideoRef.h"
#include <base/Thread/Thread.h>
#include "MainWindow.h"

class System : public pi::Thread
{
public:
    System();
    ~System();
    void run();

    bool trainVideoRef(VideoRef& refVideo,const std::string& videoPath);
    bool findVideoDiff(VideoRef& refVideo,const std::string& videoPath);
    void complishVideoCompare();

    bool trainVocabulary();

    SPtr<MainWindow> mainWindow;
    SPtr<VideoRef>   videoRef;
};

#endif // SYSTEM_H
