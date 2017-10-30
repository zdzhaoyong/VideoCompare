#ifndef BOW_OBJECT_H
#define BOW_OBJECT_H

#define ORB_BOW
#ifdef ORB_BOW
#include <opencv2/opencv.hpp>

#include "Vocabulary.h"

#include <base/Types/SPtr.h>
#include <base/Thread/Thread.h>

class BOW_Object
{
public:
    BOW_Object(){}
    BOW_Object(BOW_Object& obj)
//        :mBowVec(obj.GetBowVector()),mFeatVec(obj.GetFeatureVector())
    {
        mBowVec=obj.GetBowVector();
        mFeatVec=obj.GetFeatureVector();
    }

    virtual ~BOW_Object()
    {
    }

    // Bag of Words Representation
    void ComputeBoW(const GSLAM::GImage &mDes_)
    {
        if(mBowVec.empty()||mFeatVec.empty())
        {
            mpORBvocabulary->transform(mDes_,mBowVec,mFeatVec);
        }
    }

    GSLAM::FeatureVector GetFeatureVector()
    {
        pi::ScopedMutex lock(mMutexBow);
        return mFeatVec;
    }

    GSLAM::BowVector GetBowVector()
    {
        pi::ScopedMutex lock(mMutexBow);
        return mBowVec;
    }

    //BoW
    GSLAM::BowVector     mBowVec;
    GSLAM::FeatureVector mFeatVec;
    pi::Mutex            mMutexBow;

    static SPtr<GSLAM::Vocabulary> mpORBvocabulary;
};
#endif

#endif // BOW_OBJECT_H
