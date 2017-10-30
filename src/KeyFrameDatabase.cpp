#include "KeyFrameDatabase.h"

using namespace std;

KeyFrameDatabase::KeyFrameDatabase()
{
    mpVoc=(BOW_Object::mpORBvocabulary);
    mvInvertedFile.resize(mpVoc->size());
}

void KeyFrameDatabase::add(FrameID pKF,GSLAM::BowVector& vec)
{
    pi::ScopedMutex lock(mMutex);
    for(GSLAM::BowVector::const_iterator vit= vec.begin(), vend=vec.end(); vit!=vend; vit++)
        mvInvertedFile[vit->first].push_back(pKF);
}

void KeyFrameDatabase::erase(FrameID pKF,GSLAM::BowVector& vec)
{
    // Erase elements in the Inverse File for the entry
    for(GSLAM::BowVector::const_iterator vit=vec.begin(), vend=vec.end(); vit!=vend; vit++)
    {
        // List of keyframes that share the word
        list<FrameID> &lKFs =   mvInvertedFile[vit->first];

        for(list<FrameID>::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
        {
            if(pKF==*lit)
            {
                lKFs.erase(lit);
                break;
            }
        }
    }
}

void KeyFrameDatabase::clear()
{
    mvInvertedFile.clear();
    mvInvertedFile.resize(mpVoc->size());
}

map<FrameID,uint> KeyFrameDatabase::DetectRelocalisationCandidates(BOW_Object* F)
{
    map<FrameID,uint> lKFsSharingWords;

    // Search all keyframes that share a word with current frame

    uint maxCommonWords=1;

    {
        pi::ScopedMutex lock(mMutex);

        for(GSLAM::BowVector::const_iterator vit=F->mBowVec.begin(), vend=F->mBowVec.end(); vit != vend; vit++)
        {
            list<FrameID> &lKFs =   mvInvertedFile[vit->first];

            for(list<FrameID>::iterator lit=lKFs.begin(), lend= lKFs.end(); lit!=lend; lit++)
            {
                FrameID pKFi=*lit;
                map<FrameID,uint>::iterator it=lKFsSharingWords.find(pKFi);
                if(it==lKFsSharingWords.end()) lKFsSharingWords[pKFi]=1;
                else
                {
                    uint& count=it->second;
                    count++;
                    if(count>maxCommonWords) maxCommonWords=count;
                }
            }
        }
    }

    return lKFsSharingWords;
}
