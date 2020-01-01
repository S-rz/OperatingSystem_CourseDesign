#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <unistd.h>
#include <QtCore>
#include <time.h>

int i=0;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    setWindowFlags(windowFlags()&~Qt::WindowMaximizeButtonHint);    // 禁止最大化按钮
    setFixedSize(400,250);                    // 禁止拖动窗口大小
    ui->setupUi(this);
    ui->label->clear();
    QTimer *timer=new QTimer(this);  //新建定时器
    connect(timer,SIGNAL(timeout()),this,SLOT(display()));//关联定时器计满信号以及槽函数
    timer->start(1000);    //更新时间，1000ms=1s
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::display()
{
    int num=(i++)%10;
    QString str=QString::number(num);
    ui->label->setText(str);
}
