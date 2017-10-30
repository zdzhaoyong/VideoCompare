/**
* This file is part of ORB-SLAM.
*
* Copyright (C) 2014 Raúl Mur-Artal <raulmur at unizar dot es> (University of Zaragoza)
* For more information see <http://webdiis.unizar.es/~raulmur/orbslam/>
*
* ORB-SLAM is free software: you can redistribute it and/or modify
* it under the terms of the GNU General Public License as published by
* the Free Software Foundation, either version 3 of the License, or
* (at your option) any later version.
*
* ORB-SLAM is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
* GNU General Public License for more details.
*
* You should have received a copy of the GNU General Public License
* along with ORB-SLAM. If not, see <http://www.gnu.org/licenses/>.
*/

#ifndef KEYFRAMEDATABASE_H
#define KEYFRAMEDATABASE_H

#include <vector>
#include <list>
#include <set>

#include "BOW_Object.h"

#include <base/Thread/Thread.h>

class Map;
typedef uint FrameID;

class KeyFrameDatabase
{
public:
    KeyFrameDatabase();

   void add(FrameID pKF,GSLAM::BowVector& vec);

   void erase(FrameID pKF,GSLAM::BowVector& vec);

   void clear();

   // Relocalisation
  std::map<FrameID,uint> DetectRelocalisationCandidates(BOW_Object* F);

protected:

  // Associated vocabulary
  SPtr<GSLAM::Vocabulary>           mpVoc;

  // Inverted file
  std::vector<std::list<FrameID> >   mvInvertedFile;


  // Mutex
  pi::Mutex                     mMutex;
};

#endif
