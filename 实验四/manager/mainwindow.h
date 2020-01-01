#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <qcustomplot.h>
#include <QVector>
#include <dirent.h>
#include <sys/stat.h>
#include "QTimer"
#include "QTime"
#include "QDateTime"
#include "QFile"
#include "QMessageBox"
#include "QDebug"
#include "QDir"
#include "sys/sysinfo.h"
#include "errno.h"
#include "time.h"
#include "stdio.h"
#include "QFileDialog"
#include "QProcess"

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private:
    Ui::MainWindow *ui;
    QVector <double> x_cpu,y_cpu,x_mem,y_mem,x_swap,y_swap;
    QDateTime boot_time;
    void cpu_customplot_init(QVector <double> x,QVector <double> y);
    void mem_customplot_init(QVector <double> x,QVector <double> y);
    void swap_customplot_init(QVector <double> x,QVector <double> y);
    void show_info(int index);
private slots:
    void timer_update();
    void _show_info(int index);
    void on_reboot_clicked();
    void on_shutdown_clicked();
    void on_kill_clicked();
    void on_proc_refresh_clicked();
    void on_start_process_clicked();
    void on_proc_button_clicked();
};

#endif // MAINWINDOW_H
