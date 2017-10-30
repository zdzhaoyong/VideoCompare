#include "MainWindow.h"
#include <QPalette>
#include <base/Types/VecParament.h>

#include <base/Svar/Scommand.h>
#include <gui/widgets/SvarTable.h>
#include <opencv2/imgproc/imgproc.hpp>

using namespace std;

void GuiHandle(void *ptr,string cmd,string para)
{
    if(cmd=="MainWindow.Show")
    {
        MainWindow* mainwindow=(MainWindow*)ptr;
        mainwindow->call("Show");
        return;
    }
    else if(cmd=="MainWindow.Update")
    {
        MainWindow* mainwindow=(MainWindow*)ptr;
        mainwindow->call("Update");
        return;
    }

}

FileChooseWidget::FileChooseWidget(const QString& VarName,QWidget *parent,int ChooseType)
    :QWidget(parent),varName(VarName),type(ChooseType)
{
    setupUI();
}

void FileChooseWidget::setupUI()
{
    QLabel *labFilter = new QLabel(this);
    labFilter->setText(varName+":");

    m_edtFilter = new QLineEdit(this);
    m_edtFilter->setText(QString::fromStdString(svar.GetString(varName.toStdString(),"")));
    m_edtFilter->setMinimumWidth(500);

    QPushButton* m_btnLoad = new QPushButton(this);
    m_btnLoad->setText("Choose File");
    m_btnLoad->setToolTip("Reload variables");
    // vbox layout

    QHBoxLayout *hLayout=new QHBoxLayout(this);
    hLayout->addWidget(labFilter);
    hLayout->addWidget(m_edtFilter);
    hLayout->addWidget(m_btnLoad);

    // setup signals/slots
    connect(m_btnLoad, SIGNAL(clicked(bool)),
            this, SLOT(btnLoad_clicked(bool)));
    connect(m_edtFilter, SIGNAL(editingFinished()),
            this, SLOT(edtFilter_editingFinished()));
}

void FileChooseWidget::btnLoad_clicked(bool checked)
{
    QString fileName;
    switch (type) {
    case 0:
        fileName= QFileDialog::getOpenFileName(this, tr("Choose a file.\n"), " ",
                                               QString::fromStdString(svar.GetString(varName.toStdString()+".filter","Allfile(*.*);;")));
        break;
    case 1:
    {
        QStringList fileList=QFileDialog::getOpenFileNames(this, tr("Choose a file.\n"), " ",
                                                QString::fromStdString(svar.GetString(varName.toStdString()+".filter","Allfile(*.*);;")));
        if(!fileList.size()) return;
        int num=fileList.size();
        fileName="[";
        for(QStringList::iterator it=fileList.begin();it!=fileList.end();it++,num--)
        {
            if(num>1) fileName+=*it+",";
            else fileName+=*it+"]";
        }
    }
        break;
    case 2:
        fileName = QFileDialog::getSaveFileName(this, tr("Choose a file.\n"), " ",
                                                QString::fromStdString(svar.GetString(varName.toStdString()+".filter","Allfile(*.*);;")));
    default:
        break;
    }
     if(fileName.size()>0)
    {
        svar.GetString(varName.toStdString(),"")=fileName.toStdString();
        m_edtFilter->setText(fileName);
    }

}

void FileChooseWidget::edtFilter_editingFinished(void)
{
    svar.GetString(varName.toStdString(),"")=m_edtFilter->text().toStdString();
}

MatWidget::MatWidget(const QString& MatName,QWidget *parent)
    :QWidget(parent),matName(MatName),
      imgWidth(svar.GetInt("MatWidget.ImageWidth",400))
{
    setupUI();
    SvarWithType<SPtr<pi::MutexRW> >& inst=SvarWithType<SPtr<pi::MutexRW> >::instance();
    if(!inst.exist("VideoCompare.MutexImage"))
    {
        inst.insert("VideoCompare.MutexImage",SPtr<pi::MutexRW>(new pi::MutexRW));
    }
    mutex=inst["VideoCompare.MutexImage"];
}

void MatWidget::paintEvent( QPaintEvent * event)
{
    updateImage();
//    QWidget::paint();
}

void MatWidget::setupUI()
{
    // set widget
    //setWindowTitle("MatShow");
//    setAutoFillBackground(true);
//    QPalette palet;
//    palet.setColor(QPalette::Background, QColor(255, 0, 0));
//    this->setPalette(palet);

    //botton pannel
    QWidget *btnPannel = new QWidget(this);
    QHBoxLayout* hLayout=new QHBoxLayout();
    //btnPannel->setLayout(hLayout);

    QLabel *labFilter = new QLabel(btnPannel);
    labFilter->setText("MatName:");

    m_edtFilter = new QLineEdit(btnPannel);
    m_edtFilter->setText(matName);

    QPushButton* m_btnLoad = new QPushButton(btnPannel);
    m_btnLoad->setText("Refresh");
    m_btnLoad->setToolTip("Reload variables");

    hLayout->addWidget(labFilter);
    hLayout->addWidget(m_edtFilter);
    hLayout->addWidget(m_btnLoad);
    btnPannel->setLayout(hLayout);

    img=new QLabel(this);
    img->setText(matName);
    // vbox layout

    QVBoxLayout *verticalLayout = new QVBoxLayout(this);

    verticalLayout->addWidget(btnPannel);
    verticalLayout->addWidget(img,0,Qt::AlignCenter);
    setLayout(verticalLayout);

    // setup signals/slots
    connect(m_btnLoad, SIGNAL(clicked(bool)),
            this, SLOT(btnLoad_clicked(bool)));
    connect(m_edtFilter, SIGNAL(editingFinished()),
            this, SLOT(edtFilter_editingFinished()));
}

void MatWidget::updateImage(bool first)
{
    // update only if needed
    int& updated=svar.GetInt(matName.toStdString()+".Updated");
    if(!updated)
        return ;
    updated=0;

    cv::Mat mat;
    if(mutex.get())
    {
        pi::ReadMutex lock(*mutex);
        mat=SvarWithType<cv::Mat>::instance()[matName.toStdString()].clone();
    }
    else return;

    if(!mat.empty())
    {
        cv::resize(mat,mat,cv::Size(imgWidth,imgWidth*mat.rows/mat.cols));
    }
    else if(first)
        mat=cv::Mat(imgWidth*0.75,imgWidth,CV_8UC1,cv::Scalar::all(128));

    if(!mat.empty())
    {
        if(mat.channels()==3)
        {
            cv::cvtColor(mat,mat,CV_BGR2RGB);
            QImage qimage(mat.data,mat.cols,mat.rows,QImage::Format_RGB888);
            img->setPixmap(QPixmap::fromImage(qimage));
        }
        else if(mat.channels()==1)
        {
            QImage qimage(mat.data,mat.cols,mat.rows,QImage::Format_Indexed8);
            img->setPixmap(QPixmap::fromImage(qimage));
        }
    }
}

void MatWidget::btnLoad_clicked(bool checked)
{
    edtFilter_editingFinished();
}

void MatWidget::edtFilter_editingFinished(void)
{
    matName=m_edtFilter->text();
}


////////////////////////////////////////////////////////////////////////////////
MainWindow::MainWindow(QWidget *parent):QMainWindow(parent)
{
    // set window minimum size
    this->setMinimumSize(1366, 700);

    // window title
    setWindowTitle("Innno VideoCompare");

    scommand.RegisterCommand("MainWindow.Show",GuiHandle,this);
    scommand.RegisterCommand("MainWindow.Update",GuiHandle,this);

    VecParament<std::string> strVec=svar.get_var("MatNames",VecParament<std::string>());
    for(size_t i=0;i<strVec.size();i++)
        MatNames.push_back(QString::fromStdString(strVec[i]));

    // setup layout
    setupLayout();
    connect(this, SIGNAL(call_signal(QString) ), this, SLOT(call_slot(QString)) );
}

QWidget* MainWindow::getVideoCompareWidget()
{
    /// 1. VideoCompare tab
    QWidget* wVideoCompare =new QWidget(this);
    QVBoxLayout *layoutVideoCompare=new QVBoxLayout();
    wVideoCompare->setLayout(layoutVideoCompare);

    // 1.1. Mat show
    QHBoxLayout *layoutShowMats = new QHBoxLayout();
    size_t matwidgetNum=svar.GetInt("MatWidgetNumber",3);
    for(size_t i=0;i<matwidgetNum&&i<MatNames.size();i++)
    {
        MatWidget* mat=new MatWidget(MatNames[i],this);
        layoutShowMats->addWidget(mat);
        mats.push_back(mat);
    }
    layoutVideoCompare->addLayout(layoutShowMats);

    // 1.2. VideoCompare interface
    QTabWidget* tabVideoCompare=new QTabWidget(wVideoCompare);
    layoutVideoCompare->addWidget(tabVideoCompare);

    // 1.2.1 Path set up
    QWidget* wPathSetUp=new QWidget(wVideoCompare);
    QVBoxLayout* wPathLayout=new QVBoxLayout(wPathSetUp);
    tabVideoCompare->addTab(wPathSetUp,"Path Settings");

    FileChooseWidget* VocabularyFileWidget=new FileChooseWidget("ORBVocabularyFile",wPathSetUp);
    svar.GetString("ORBVocabularyFile.filter","")="vocabulary(*.voc)";
    FileChooseWidget* RefWidget=new FileChooseWidget("VideoRef.VideoPath",wPathSetUp);
//    svar.GetString("VideoRef.VideoPath.filter","")="video(*.avi)";
    FileChooseWidget* Video2CompareWidget=new FileChooseWidget("Video2Compare",wPathSetUp);
//    svar.GetString("Video2Compare.filter","")="vocabulary(*.voc)";


    QPushButton* buttomStart=new QPushButton(wPathSetUp);
    buttomStart->setText("Start");
    connect(buttomStart, SIGNAL(clicked(bool)),
            this, SLOT(btnStart_clicked(bool)));

    QPushButton* buttomPause=new QPushButton(wPathSetUp);
    buttomPause->setText("Pause");
    connect(buttomPause, SIGNAL(clicked(bool)),
            this, SLOT(btnPause_clicked(bool)));

    QPushButton* buttomStop=new QPushButton(wPathSetUp);
    buttomStop->setText("Stop");
    connect(buttomStop, SIGNAL(clicked(bool)),
            this, SLOT(btnStop_clicked(bool)));


    wPathLayout->addWidget(VocabularyFileWidget);
    wPathLayout->addWidget(RefWidget);
    wPathLayout->addWidget(Video2CompareWidget);
    QHBoxLayout* buttomLayout=new QHBoxLayout();
    buttomLayout->addWidget(buttomStart);
    buttomLayout->addWidget(buttomPause);
    buttomLayout->addWidget(buttomStop);
    wPathLayout->addLayout(buttomLayout);
    return wVideoCompare;
}

QWidget* MainWindow::getTrainBowWidget()
{
    QWidget* tabTrainBow=new QWidget(this);
    QVBoxLayout* tabLayout=new QVBoxLayout();
    tabTrainBow->setLayout(tabLayout);

    tabLayout->addWidget(new MatWidget("TrainVocabulary.CurrentImage",tabTrainBow));
    tabLayout->addWidget(new FileChooseWidget("TrainVocabulary.Videos",tabTrainBow,1));
    QPushButton* buttomStart=new QPushButton("Start train",tabTrainBow);
    QPushButton* buttomSave =new QPushButton("Save",tabTrainBow);
    tabLayout->addWidget(buttomStart);
    tabLayout->addWidget(buttomSave);

    connect(buttomStart, SIGNAL(clicked(bool)),
            this, SLOT(btnStartTrain_clicked(bool)));

    connect(buttomSave, SIGNAL(clicked(bool)),
            this, SLOT(btnSaveVocabulary_clicked(bool)));

    return tabTrainBow;
}

int MainWindow::setupLayout(void)
{
    QTabWidget* m_tabWidget = new QTabWidget(this);

    m_tabWidget->addTab(getVideoCompareWidget(),"VideoCompare");
    m_tabWidget->addTab(getTrainBowWidget(),"TrainVocabulary");
    /// 2. SvarTable tab

    if(svar.GetInt("Draw.SvarWidget",0))
    {
        m_tabWidget->addTab(new SvarWidget(this),"SvarWidget");
    }


    /// 3. Overall layout
    setCentralWidget(m_tabWidget);
    return 0;
}

void MainWindow::btnStart_clicked(bool checked)
{
    // Vocabulary path
    svar.GetInt("PathSetted")=1;
    svar.ParseLine("System.Start VideoCompare");
}

void MainWindow::btnPause_clicked(bool checked)
{
    int& pause=svar.GetInt("System.Pause");
    pause=!pause;
}

void MainWindow::btnStop_clicked(bool checked)
{
    svar.ParseLine("System.Stop");
}

void MainWindow::btnStartTrain_clicked(bool checked)
{
    svar.ParseLine("System.Start TrainVocabulary");
}

void MainWindow::btnSaveVocabulary_clicked(bool checked)
{
    QString file2save=QFileDialog::getSaveFileName(this,"Vocabulary file to save");
    svar.ParseLine("System.SaveVocabulary "+file2save.toStdString());
}

void MainWindow::keyPressEvent(QKeyEvent *e)
{
    switch(e->key())
    {
    case Qt::Key_Space:
        break;

    case Qt::Key_Escape:
        exit(0);
        break;

    default:
        svar.i["KeyPressMsg"] = e->key();
        break;
    }
}

void MainWindow::mousePressEvent(QMouseEvent *event)
{
#if 0
    // 1 - left
    // 2 - right
    // 4 - middle
    printf("window pressed, %d, %d, %d\n", event->button(), event->pos().x(), event->pos().y());

    if( event->button() == 1 ) {

    }
#endif
}

void MainWindow::resizeEvent(QResizeEvent *e)
{
    return;
    for(size_t i=0;i<mats.size();i++)
    {
        cout<<"Set img size to "<<this->width()/mats.size()<<","<<this->width()*480/(mats.size()*640)<<endl;
        mats[i]->img->resize(this->width()/mats.size(),this->width()*480/(mats.size()*640));
    }
}

void MainWindow::timerEvent(QTimerEvent *event)
{
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    svar.ParseLine("System.Stop");
    exit(0);
}

void MainWindow::call(QString cmd)
{
    emit call_signal(cmd);
}

void MainWindow::call_slot(QString cmd)
{
    if("Show"==cmd)   show();
    if("Update"==cmd)
    {
        update();
//        for(int i=0;i<mats.size();i++)
//            mats[i]->updateImage();
    }
    else
        scommand.Call(cmd.toStdString());
}
