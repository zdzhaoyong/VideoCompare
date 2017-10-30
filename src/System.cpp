#include "System.h"
#include <base/Svar/Svar.h>
#include <base/Svar/Scommand.h>
#include <base/Types/VecParament.h>
#include <opencv2/highgui/highgui.hpp>
#include "VideoCompare.h"
#include "VideoFrame.h"
#include <base/Time/Global_Timer.h>

using namespace std;

void SystemHandle(void *ptr,string cmd,string para)
{
    if(cmd=="System.Start")
    {
        svar.GetInt("PathSetted")=1;
        svar.GetString("Act","")=para;
        System* sys=(System*)ptr;
        if(!sys->isRunning())
            sys->start();
        return;
    }
    else if(cmd=="System.Stop")
    {
        svar.GetInt("PathSetted")=0;
        System* sys=(System*)ptr;
        sys->stop();
        return;
    }
    else if(cmd=="System.SaveVocabulary")
    {
        if(BOW_Object::mpORBvocabulary.get())
        {
            string voc2save=para;
            cerr<<"Saving vocabulary to file "<<voc2save<<endl;
            BOW_Object::mpORBvocabulary->save(voc2save);
            cerr<<"Vocabulary saved.\n";
        }
        return;
    }


}

System::System()
{
    scommand.RegisterCommand("System.Start",SystemHandle,this);
    scommand.RegisterCommand("System.Stop",SystemHandle,this);
    scommand.RegisterCommand("System.SaveVocabulary",SystemHandle,this);
    if(svar.GetInt("WithQt"))
    {
        mainWindow=SPtr<MainWindow>(new MainWindow());
        mainWindow->call("Show");
    }
}


System::~System()
{
    stop();
    if(isRunning())
    {
        sleep(10);
    }
    join();
}

void System::complishVideoCompare()
{

    if(svar.GetInt("WithQt"))
    {
        int& pathSetted=svar.GetInt("PathSetted");
        while(!pathSetted) sleep(10);
    }

    std::string& refVideoPath=svar.GetString("VideoRef.VideoPath","");
    if(!refVideoPath.size()){
        cerr<<"Please set the \"VideoRef.VideoPath\"!\n";
        return;
    }

    // load vocabulary
    string vocFile=svar.GetString("ORBVocabularyFile","");
    if(vocFile.empty())
    {
        cerr<<"Please set 'ORBVocabularyFile'.\n";
        return;
    }

    cout <<"Loading vocabulary: " << vocFile << " ... ";

    BOW_Object::mpORBvocabulary=SPtr<GSLAM::Vocabulary>(new GSLAM::Vocabulary);
    BOW_Object::mpORBvocabulary->load(vocFile);

    if(vocFile.find(".gbow")==std::string::npos)
    {

    }
    cout << endl;

    // videoref from video or file
    std::string videoRefPath=svar.GetString("VideoRef.DataPath",refVideoPath+".ref");
    videoRef=SPtr<VideoRef>(new VideoRef(videoRefPath));
    if(!videoRef->loadFromFile(videoRefPath))
    {
        pi::timer.enter("TrainTotal");
        trainVideoRef(*videoRef,refVideoPath);
        pi::timer.leave("TrainTotal");
    }
    else
    {
        cout<<"VideoRef loaded from "<<videoRefPath<<"."<<endl;
    }

    // compare video difference
    findVideoDiff(*videoRef,svar.GetString("Video2Compare",""));

    videoRef->save2File(videoRefPath);

    cout<<"System stoped.\n";
}

void System::run()
{
    string act=svar.GetString("Act","");
    if(act=="TrainVocabulary")
    {
        trainVocabulary();
    }
    else complishVideoCompare();
}

bool System::trainVideoRef(VideoRef& refVideo,const std::string& refVideoPath)
{
    cv::VideoCapture video(refVideoPath);
    if(!video.isOpened())
    {
        cerr<<"Can't open video "<<refVideoPath<<endl;
        return false;
    }
    VideoCompare compare(refVideo);
    cout<<"Training VideoRef from video "<<refVideoPath<<".\n";

    pi::TicTac ticToc;
    ticToc.Tic();

    double scale=svar.GetDouble("ComputeScale",0.5);
    int    trainSkip=svar.GetInt("TrainSkip",5);
    int& pause=svar.GetInt("System.Pause");
    while(!shouldStop())
    {
        while(pause) sleep(10);
        SPtr<VideoFrame> frame(new VideoFrame);
        pi::timer.enter("GrabImage");
        for(int i=0;i<trainSkip;i++)
            video.grab();
        video>>frame->img;
        pi::timer.leave("GrabImage");

        if(frame->img.empty())
        {
            break;
        }

        if(scale<1)
        {
            cv::resize(frame->img,frame->img,cv::Size(frame->img.cols*scale,frame->img.rows*scale));
        }

        pi::timer.enter("TrainFrame");
        compare.train(frame);
        pi::timer.leave("TrainFrame");

        if(!svar.GetInt("WithQt"))
        {
            if(!compare.refImg.empty()&&svar.GetInt("Draw.RefImage",1))
                cv::imshow("RefImage",compare.refImg);
            if(!compare.trackImg.empty()&&svar.GetInt("Draw.TrackImage",1))
                cv::imshow("TrackImage",compare.trackImg);
            if(!compare.refImgHere.empty()&&svar.GetInt("Draw.RefImageHere"))
                cv::imshow("RefImageHere",compare.refImgHere);
            if(!compare.warpImg.empty()&&svar.GetInt("Draw.WarpImage",1))
                cv::imshow("WarpImage",compare.warpImg);
            if(!compare.diffImg.empty()&&svar.GetInt("Draw.DiffImage",1))
                cv::imshow("DiffImage",compare.diffImg);
            uchar key=cv::waitKey(20);
            if(key==27)   stop();
        }
        else
        {
            mainWindow->call("Update");
        }
    }

    return true;
}

bool System::findVideoDiff(VideoRef& refVideo,const std::string& videoPath)
{
    if(videoPath.empty())
    {
        cerr<<"Video2Compare not setted. Skip comparison. \n";
        return false;
    }

    cv::VideoCapture video(videoPath);
    if(!video.isOpened())
    {
        cerr<<"Can't open video "<<videoPath<<endl;
        return false;
    }
    VideoCompare compare(refVideo);
    cout<<"Find difference at video "<<videoPath<<".\n";

    pi::TicTac ticToc;
    ticToc.Tic();

    double scale=svar.GetDouble("ComputeScale",0.5);
    int    computeSkip=svar.GetInt("ComputeSkip",5);

    int& pause=svar.GetInt("System.Pause");
    while(!shouldStop())
    {
        while(pause) sleep(10);
        SPtr<VideoFrame> frame(new VideoFrame);
        pi::timer.enter("GrabImage");
        for(int i=0;i<computeSkip;i++)
            video.grab();
        video>>frame->img;
        pi::timer.leave("GrabImage");

        if(frame->img.empty())
        {
            break;
        }

        if(scale<1)
        {
            cv::resize(frame->img,frame->img,
                       cv::Size(frame->img.cols*scale,frame->img.rows*scale));
        }

        pi::timer.enter("TrackDiff");
        compare.track(frame);
        pi::timer.leave("TrackDiff");

        if(!svar.GetInt("WithQt"))
        {
            if(!compare.refImg.empty()&&svar.GetInt("Draw.RefImage",1))
                cv::imshow("RefImage",compare.refImg);
            if(!compare.trackImg.empty()&&svar.GetInt("Draw.TrackImage",1))
                cv::imshow("TrackImage",compare.trackImg);
            if(!compare.refImgHere.empty()&&svar.GetInt("Draw.RefImageHere"))
                cv::imshow("RefImageHere",compare.refImgHere);
            if(!compare.warpImg.empty()&&svar.GetInt("Draw.WarpImage",1))
                cv::imshow("WarpImage",compare.warpImg);
            if(!compare.diffImg.empty()&&svar.GetInt("Draw.DiffImage",1))
                cv::imshow("DiffImage",compare.diffImg);
            uchar key=cv::waitKey(20);
            if(key==27)   stop();
        }
        else
        {
            mainWindow->call("Update");
        }
    }

    return true;
}

bool System::trainVocabulary()
{
    VecParament<string> videos;

    {
        stringstream sst(svar.GetString("TrainVocabulary.Videos",""));
        sst>>videos;
        if(!videos.size())
        {
            return false;
        }
    }


    int maxTrainFrames=svar.GetInt("MaxTrainFrames",1000);
    int computeSkip=svar.GetInt("TrainVocabulary.ComputeSkip",0);
    vector<GSLAM::GImage> trainData;
    trainData.reserve(maxTrainFrames);


    cv::Mat& img=SvarWithType<cv::Mat>::instance()["TrainVocabulary.CurrentImage"];
    int&     imgChanged=svar.GetInt("TrainVocabulary.CurrentImage.Updated",0);

    SvarWithType<SPtr<pi::MutexRW> >& inst=SvarWithType<SPtr<pi::MutexRW> >::instance();
    if(!inst.exist("VideoCompare.MutexImage"))
    {
        inst.insert("VideoCompare.MutexImage",SPtr<pi::MutexRW>(new pi::MutexRW));
    }
    SPtr<pi::MutexRW> mutex=inst["VideoCompare.MutexImage"];


    ORBextractor extractor;
    vector<cv::KeyPoint> keypoints;
    for(int i=0;i<videos.size()&&!shouldStop();i++){
        cv::VideoCapture video(videos[i]);
        while(!shouldStop())
        {
            pi::timer.enter("GrabImage");
            if(video.isOpened())
            {
                for(int i=0;i<computeSkip;i++)
                    video.grab();

                {
                    pi::WriteMutex lock(*mutex);
                    video>>img;
                    if(img.empty()) break;
                    if(img.type()!=CV_8UC1)
                    {
                        cv::cvtColor(img,img,CV_BGR2GRAY);
                    }
                }
            }
            else
            {
                img=cv::imread(videos[i],cv::IMREAD_GRAYSCALE);
                if(img.empty()) break;
            }
            pi::timer.leave("GrabImage");

            pi::timer.enter("ExtractFeatures");
            cv::Mat descriptors;
            extractor(img,cv::Mat(),keypoints,descriptors);;
            trainData.push_back(descriptors);
            stringstream sst;
            sst<<"Frame:"<<trainData.size()<<",Features:"<<descriptors.rows;
            cv::putText(img,sst.str(),cv::Point(30,30),CV_FONT_HERSHEY_COMPLEX,1.,cv::Scalar(0,0,255),2);
            imgChanged=1;
            if(svar.GetInt("WithQt"))
            {
                mainWindow->call("Update");
            }
            else
            {
                cv::imshow("TrainVocabulary.CurrentImage",img);
                cv::waitKey(10);
            }
            cout<<sst.str()<<endl;
            cout.flush();
            if(trainData.size()>maxTrainFrames) break;
            pi::timer.leave("ExtractFeatures");
        }
        if(trainData.size()>maxTrainFrames) break;
    }

    cout<<"Creating orb vocabulary...\n";cout.flush();

    if(!BOW_Object::mpORBvocabulary.get())
        BOW_Object::mpORBvocabulary=SPtr<GSLAM::Vocabulary>(new GSLAM::Vocabulary);
    pi::timer.enter("CreateVocabulary");
    BOW_Object::mpORBvocabulary=GSLAM::Vocabulary::create(trainData);
    pi::timer.leave("CreateVocabulary");

    cout<<"Vocabulary created!\n";cout.flush();
    return true;
}
