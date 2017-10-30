#include <iostream>
#include <base/Svar/Svar.h>
#include <base/Time/Global_Timer.h>
#include "System.h"
#include <QApplication>

using namespace std;

int main(int argc,char** argv)
{
    pi::timer.enter("GlobalAll");
    svar.ParseMain(argc,argv);

    if(svar.GetInt("WithQt",1))
    {
        svar.ParseLine("MatNames?=[VideoCompare.RefImageHere,VideoCompare.TrackImage,VideoCompare.DiffImage,VideoCompare.RefImage,VideoCompare.WarpImage]");
        QApplication app(argc,argv);
        System system;
//        system.start();
        app.exec();
    }
    else
    {
        System system;
        system.run();
    }

    pi::timer.leave("GlobalAll");

    return 0;
}

