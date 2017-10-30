#ifndef MAINWINDOW_H
#define MAINWINDOW_H
#include <QMainWindow>
#include <QLabel>
#include <QLineEdit>
#include <base/Thread/Thread.h>
#include <QTextStream>

//class OpenWidget: public QWidget
//{
//    Q_OBJECT
//public:
//};

//class LogWidget :public QWidget
//{
//public:
//    explicit LogWidget(QWidget* parent)
//        :QWidget(parent),Qcout(stdout)
//    {
//        setupUI();
//    }
//    void     setupUI()
//    {
//        browser=new QTextEdit(this);
//    }
//    virtual void paintEvent(QPaintEvent* e)
//    {
//        browser->setText(Qcout.readAll());
//    }

//    QTextStream   Qcout;
//    QTextEdit*    browser;
//};

/**
 * @brief The MatWidget class aim to show a SvarWithTypeMat
 */
class MatWidget:public QWidget
{
    Q_OBJECT
public:
    explicit MatWidget(const QString& MatName,QWidget *parent=NULL);
    virtual ~MatWidget(){}

    void setupUI();
    void updateImage(bool first=false);
    virtual void paintEvent( QPaintEvent * event);

protected slots:
    void btnLoad_clicked(bool checked=false);
    void edtFilter_editingFinished(void);
public:
    QString        matName;
    QLineEdit*     m_edtFilter;
    QLabel*        img;
    SPtr<pi::MutexRW> mutex;
    int&           imgWidth;
};

class FileChooseWidget: public QWidget
{
    Q_OBJECT
public:
    explicit FileChooseWidget(const QString& VarName,QWidget *parent=NULL,int ChooseType=0);
    virtual ~FileChooseWidget(){}

    void setupUI();

protected slots:
    void btnLoad_clicked(bool checked=false);
    void edtFilter_editingFinished(void);
public:
    QString        varName;
    QLineEdit*     m_edtFilter;
    int            type;
};

class MainWindow:public QMainWindow
{
    Q_OBJECT
public:
    MainWindow(QWidget *parent = 0);
    virtual ~MainWindow(){}

    virtual int setupLayout(void);

    void call(QString cmd);

signals:
    void call_signal(QString cmd);

protected slots:
    void call_slot(QString cmd);

    void btnStart_clicked(bool checked=false);
    void btnPause_clicked(bool checked=false);
    void btnStop_clicked(bool checked=false);
    void btnStartTrain_clicked(bool checked=false);
    void btnSaveVocabulary_clicked(bool checked=false);

protected:
    void keyPressEvent(QKeyEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void resizeEvent(QResizeEvent *event);
    void timerEvent(QTimerEvent *event);
    void closeEvent(QCloseEvent *event);

    QWidget* getVideoCompareWidget();
    QWidget* getTrainBowWidget();

    pi::Mutex                  mutex;
    std::vector<QString>       MatNames;
    std::vector<MatWidget*>    mats;
};

#endif // MAINWINDOW_H
