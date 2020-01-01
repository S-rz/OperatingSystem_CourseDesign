#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QtCore>
#include <time.h>

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(400,250);                    // 禁止拖动窗口大小
    ui->setupUi(this);

    QTimer *timer=new QTimer(this);  //新建定时器
    connect(timer,SIGNAL(timeout()),this,SLOT(timeUpdate()));//关联定时器计满信号以及槽函数
    timer->start(1);    //更新时间，1000ms=1s
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timeUpdate()
{
    QDateTime time=QDateTime::currentDateTime();    //获取系统时间
    QString str=time.toString("yyyy-MM-dd hh:mm:ss dddd");  //时间显示格式
    ui->label->setText(str);
}
