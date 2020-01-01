#include "mainwindow.h"
#include "ui_mainwindow.h"

QTimer *timer;
MainWindow::MainWindow(QWidget *parent) : QMainWindow(parent),
                                          ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    this->setWindowTitle("任务管理器");
    struct sysinfo info;
    sysinfo(&info);
    uint cur_time_t, boot_time_t;
    QDateTime cur_time = QDateTime::currentDateTime();
    cur_time_t = cur_time.toTime_t();
    boot_time_t = cur_time_t - info.uptime;
    boot_time.setTime_t(boot_time_t);
    this->ui->boot_time->setText("系统启动时间:" + boot_time.toString("yyyy-MM-dd hh:mm:ss"));
    timer = new QTimer(this);
    show_info(0);
    QWidget::connect(timer, SIGNAL(timeout()), this, SLOT(timer_update()));
    QWidget::connect(ui->tabWidget, SIGNAL(currentChanged(int)), this, SLOT(_show_info(int)));
    timer->start(1000);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::timer_update()
{
    show_info(0);
}

void MainWindow::_show_info(int index)
{
    if (index != 0)
    {
        show_info(index);
    }
}

void MainWindow::show_info(int index)
{  //Tab显示
    QString str;
    QFile file;
    int pos;
    static int cpu_times = 0;
    static int mem_times = 0;
    static int swap_times = 0;
    int flag1 = 0, flag2 = 0, flag3 = 0, flag4 = 0;
    QDateTime cur_time = QDateTime::currentDateTime();
    QTime run_time;
    run_time.setHMS(0, 0, 0, 0);
    ui->run_time->setText("系统运行时间:" + run_time.addSecs(boot_time.secsTo(cur_time)).toString("hh:mm:ss"));
    QString output;
    output.clear();
    output += cur_time.toString("yyyy-MM-dd hh:mm:ss") + "\t";
    if (index == 0)
    {
        file.setFileName("/proc/meminfo");
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("warning"), tr("Open the meminfo file failed!"), QMessageBox::Yes);
            return;
        }
        QString memused, memfree, memtotal, swapused, swapfree, swaptotal;
        int nmemused, nmemfree, nmemtotal, nswapused, nswapfree, nswaptotal;
        while (1)
        {
            str = file.readLine();
            if ((pos = str.indexOf("MemTotal"), pos != -1) && !flag1)
            {
                memtotal = str.mid(pos + 9, str.length() - 12);
                memtotal.trimmed();
                nmemtotal = memtotal.toInt() / 1024;
                flag1 = 1;
            }
            if ((pos = str.indexOf("MemFree"), pos != -1) && !flag2)
            {
                memfree = str.mid(pos + 8, str.length() - 11);
                memfree.trimmed();
                nmemfree = memfree.toInt() / 1024;
                flag2 = 1;
            }
            if ((pos = str.indexOf("SwapFree"), pos != -1) && !flag3)
            {
                swapfree = str.mid(pos + 9, str.length() - 12);
                swapfree.trimmed();
                nswapfree = swapfree.toInt() / 1024;
                flag3 = 1;
            }
            if ((pos = str.indexOf("SwapTotal"), pos != -1) && !flag4)
            {
                swaptotal = str.mid(pos + 10, str.length() - 13);
                swaptotal.trimmed();
                nswaptotal = swaptotal.toInt() / 1024;
                flag4 = 1;
            }
            if (flag1 && flag2 && flag3 && flag4)
                break;
        }
        nmemused = nmemtotal - nmemfree;
        memused = QString::number(nmemused, 10);
        memtotal = QString::number(nmemtotal, 10);
        memfree = QString::number(nmemfree, 10);

        ui->memused->setText("使用:" + memused + "MB");
        ui->memfree->setText("空闲:" + memfree + "MB");
        ui->memtotal->setText("总和:" + memtotal + "MB");
        ui->mem->setText("内存使用率:" + QString::number(nmemused * 100 / nmemtotal) + "%");
        output += "内存使用率:" + QString::number(nmemused * 100 / nmemtotal) + "%" + "\t";
        if (mem_times < 61)
        {
            x_mem << (double)mem_times;
            y_mem << (double)(nmemused * 100 / nmemtotal);
            mem_times++;
            mem_customplot_init(x_mem, y_mem);
        }
        else
        {
            y_mem.remove(0);
            y_mem << (double)(nmemused * 100 / nmemtotal);
            mem_customplot_init(x_mem, y_mem);
        }
        nswapused = nswaptotal - nswapfree;
        swapused = QString::number(nswapused, 10);
        swaptotal = QString::number(nswaptotal, 10);
        swapfree = QString::number(nswapfree, 10);
        ui->swapused->setText("使用:" + memused + "MB");
        ui->swapfree->setText("空闲:" + memfree + "MB");
        ui->swaptotal->setText("总和:" + memtotal + "MB");
        ui->swap->setText("交换率:" + QString::number(nswapused * 100 / nswaptotal) + "%");
        output += "内存交换率:" + QString::number(nswapused * 100 / nswaptotal) + "%" + "\t";
        if (swap_times < 61)
        {
            x_swap << (double)swap_times;
            y_swap << (double)(nswapused * 100 / nswaptotal);
            swap_times++;
            swap_customplot_init(x_swap, y_swap);
        }
        else
        {
            y_swap.remove(0);
            y_swap << (double)(nswapused * 100 / nswaptotal);
            swap_customplot_init(x_swap, y_swap);
        }
        file.close();

        static int is_start = 1;
        static int flag = 0, uflag = 1;
        static long cpu_info[2][10];
        static long cpu_temp[2][2];
        long cpu_used;
        long cpu_total;
        cpu_temp[1][flag] = 0;
        file.setFileName("/proc/stat");
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("warning"), tr("Open the stat file failed!\n"), QMessageBox::Yes);
            return;
        }
        str = file.readLine();
        for (int i = 0; i < 10; i++)
        {
            cpu_info[flag][i] = str.section(" ", i + 2, i + 2).toLong();
            cpu_temp[1][flag] += cpu_info[flag][i]; //total
            if (i == 3)
            {
                cpu_temp[0][flag] = cpu_info[flag][i]; //free
            }
        }
        file.close();
        cpu_temp[0][flag] = cpu_temp[1][flag] - cpu_temp[0][flag]; //used
        if (is_start)
        {

            is_start--;
            x_cpu << 0;
            y_cpu << 0;
            cpu_times++;
            cpu_customplot_init(x_cpu, y_cpu);
        }
        else
        {
            if (cpu_temp[0][flag] <= cpu_temp[0][uflag])
            {
                ui->cpu->setText("利用率:0%");
                output += "CPU利用率:0%";
                if (cpu_times < 61)
                {
                    x_cpu << 0;
                    y_cpu << 0;
                    cpu_times++;
                    cpu_customplot_init(x_cpu, y_cpu);
                }
                else
                {
                    y_cpu.remove(0);
                    y_cpu << 0;
                    cpu_customplot_init(x_cpu, y_cpu);
                }
            }
            else
            {
                cpu_used = cpu_temp[0][flag] - cpu_temp[0][uflag];
                cpu_total = cpu_temp[1][flag] - cpu_temp[1][uflag];
                ui->cpu->setText("利用率:" + QString::number(cpu_used * 100 / cpu_total) + "%");
                output += "CPU利用率:" + QString::number(cpu_used * 100 / cpu_total) + "%";
                if (cpu_times < 61)
                {
                    x_cpu << (double)cpu_times;
                    y_cpu << (double)(cpu_used * 100 / cpu_total);
                    cpu_times++;
                    cpu_customplot_init(x_cpu, y_cpu);
                }
                else
                {
                    y_cpu.remove(0);
                    y_cpu << (double)(cpu_used * 100 / cpu_total);
                    cpu_customplot_init(x_cpu, y_cpu);
                }
            }
        }
        ui->statusBar->showMessage(output);
        uflag = flag;
        flag = !flag;
    }
    else if (index == 1)
    {
        ui->listWidget_proc->clear();
        QDir qd("/proc");
        QStringList qslist = qd.entryList();
        QString qs = qslist.join("\n");
        QString id_of_pro;
        bool ok;
        int find_start = 3;
        int a, b;
        int nProPid;
        int number_of_sleep = 0, number_of_run = 0, number_of_zombie = 0;
        int totalProNum = 0;
        QString proName;
        QString proState;
        QString proPri;
        QString proMem;
        QListWidgetItem *title = new QListWidgetItem("PID\t" + QString::fromUtf8("名称") + "\t\t\t" +
                                                         QString::fromUtf8("状态") + "\t" +
                                                         QString::fromUtf8("优先级") + "\t" +
                                                         QString::fromUtf8("占用内存"),
                                                     ui->listWidget_proc);
        while (1)
        {
            a = qs.indexOf("\n", find_start);
            b = qs.indexOf("\n", a + 1);
            find_start = b;
            id_of_pro = qs.mid(a + 1, b - a - 1);
            nProPid = id_of_pro.toInt(&ok, 10);
            if (!ok)
            {
                break;
            }

            file.setFileName("/proc/" + id_of_pro + "/stat");
            if (!file.open(QIODevice::ReadOnly))
            {
                QMessageBox::warning(this, tr("warning"), tr("The pid stat file can not open!"), QMessageBox::Yes);
                return;
            }
            str = file.readLine();
            if (str.length() == 0)
            {
                file.close();
                break;
            }
            file.close();
            totalProNum++;
            a = str.indexOf("(");
            b = str.indexOf(")");
            proName = str.mid(a + 1, b - a - 1);
            proName.trimmed();
            proName.resize(22, ' ');
            //qDebug() << proName;
            proState = str.section(" ", 2, 2);
            proState.trimmed();
            proPri = str.section(" ", 17, 17);
            proMem = str.section(" ", 22, 22);
            switch (proState.at(0).toLatin1())
            {
            case 'S':
                number_of_sleep++;
                break;
            case 'R':
                number_of_run++;
                break;
            case 'Z':
                number_of_zombie++;
                break;
            default:
                break;
            }
            QListWidgetItem *item = new QListWidgetItem(id_of_pro + "\t" +
                                                            proName + "\t\t" +
                                                            proState + "\t" +
                                                            proPri + "\t" +
                                                            proMem,
                                                        ui->listWidget_proc);
            ui->proc->setText("进程数:" + QString::number(totalProNum, 10));
            ui->run->setText("运行数:" + QString::number(number_of_run, 10));
            ui->sleep->setText("睡眠数:" + QString::number(number_of_sleep, 10));
            ui->zombie->setText("僵尸数:" + QString::number(number_of_zombie, 10));
        }
    }
    else if (index == 2)
    {
        file.setFileName("/proc/cpuinfo");
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("warning"), tr("Open the cpuinfo file failed!\n"), QMessageBox::Yes);
            return;
        }
        while (1)
        {
            str = file.readLine();
            if (str == NULL)
                break;
            pos = str.indexOf("model name");
            if (pos != -1)
            {
                pos += 13;
                QString *cpu_name = new QString(str.mid(pos, str.length() - 13));
                ui->cpu_name->setText("CPU名称:" + *cpu_name);
            }
            else if (pos = str.indexOf("vendor_id"), pos != -1)
            {
                pos += 12;
                QString *cpu_type = new QString(str.mid(pos, str.length() - 12));
                ui->cpu_type->setText("CPU类型:" + *cpu_type);
            }
            else if (pos = str.indexOf("cpu MHz"), pos != -1)
            {
                pos += 11;
                QString *cpu_frq = new QString(str.mid(pos, str.length() - 11));
                double cpufrq = cpu_frq->toDouble();
                cpu_frq->setNum(cpufrq);
                ui->cpu_frequency->setText("CPU频率:" + *cpu_frq + " HZ");
            }
            else if (pos = str.indexOf("cache size"), pos != -1)
            {
                pos += 13;
                QString *cache_size = new QString(str.mid(pos, str.length() - 16));
                int cachesize = cache_size->toInt();
                cache_size->setNum(cachesize);
                ui->cache_size->setText("Cache大小:" + *cache_size + " KB");
            }
        }
        file.close();
        file.setFileName("/proc/version");
        if (!file.open(QIODevice::ReadOnly))
        {
            QMessageBox::warning(this, tr("warning"), tr("Open the version file failed!\n"), QMessageBox::Yes);
            return;
        }
        str = file.readLine();
        pos = str.indexOf("version");
        QString *os_type = new QString(str.mid(0, pos - 1));
        ui->OS_type->setText("操作系统类型:" + *os_type);
        delete os_type;
        int pos1 = str.indexOf("(");
        QString *os_version = new QString(str.mid(pos + 8, pos1 - pos - 9));
        ui->OS_version->setText("操作系统版本:" + *os_version);
        delete os_version;
        pos = str.indexOf("gcc version");
        pos1 = str.indexOf("#");
        QString *gcc_info = new QString(str.mid(pos + 12, pos1 - pos - 14));
        ui->gcc->setText("GCC编译器:" + *gcc_info);
        delete gcc_info;
        file.close();
    }
    else if (index == 3)
    {
        ui->my_name->setText("姓名：史瑞泽");
        ui->my_xuehao->setText("学号：U201615677");
        ui->my_class->setText("班级：CSIE1601");
    }
}

void MainWindow::on_reboot_clicked()
{   //系统调用重启
    system("reboot");
}

void MainWindow::on_shutdown_clicked()
{   //系统调用关机
    system("shutdown -h now");
}

void MainWindow::on_kill_clicked()
{   //关闭进程
    QListWidgetItem *item = ui->listWidget_proc->currentItem();
    QString proc = item->text();
    proc = proc.section("\t", 0, 0);
    system("kill " + proc.toLatin1());
    QMessageBox::warning(this, tr("kill"), tr("killed!"), QMessageBox::Yes);
    show_info(1);
}

void MainWindow::on_proc_refresh_clicked()
{   //进程刷新
    show_info(1);
}

/*Tab折线图初始化*/
void MainWindow::cpu_customplot_init(QVector<double> x, QVector<double> y)
{         //CPU利用率
    ui->cpu_customplot->addGraph();
    ui->cpu_customplot->graph(0)->setPen(QPen(QColor(70, 0, 70)));
    ui->cpu_customplot->graph(0)->setBrush(QBrush(QColor(70, 0, 70, 50)));
    ui->cpu_customplot->graph(0)->setData(x, y);
    ui->cpu_customplot->xAxis->setLabel("time");
    ui->cpu_customplot->xAxis->setLabelPadding(0);
    ui->cpu_customplot->xAxis->setTickLabelPadding(0);
    ui->cpu_customplot->yAxis->setLabel("cpu%");
    ui->cpu_customplot->xAxis->setRange(60, 0);
    ui->cpu_customplot->yAxis->setRange(0, 100);
    ui->cpu_customplot->xAxis->setTickLengthIn(0);
    ui->cpu_customplot->xAxis->setSubTicks(false);
    ui->cpu_customplot->yAxis->setTickLengthIn(0);
    ui->cpu_customplot->yAxis->setSubTicks(false);
    ui->cpu_customplot->replot();
}
void MainWindow::mem_customplot_init(QVector<double> x, QVector<double> y)
{          //内存利用率
    ui->mem_customplot->addGraph();
    ui->mem_customplot->graph(0)->setPen(QPen(QColor(70, 0, 70)));
    ui->mem_customplot->graph(0)->setBrush(QBrush(QColor(70, 0, 70, 50)));
    ui->mem_customplot->graph(0)->setData(x, y);
    ui->mem_customplot->xAxis->setLabel("time");
    ui->mem_customplot->yAxis->setLabel("memory%");
    ui->mem_customplot->xAxis->setRange(60, 0);
    ui->mem_customplot->yAxis->setRange(0, 100);
    ui->mem_customplot->xAxis->setTickLengthIn(0);
    ui->mem_customplot->xAxis->setSubTicks(false);
    ui->mem_customplot->yAxis->setTickLengthIn(0);
    ui->mem_customplot->yAxis->setSubTicks(false);
    ui->mem_customplot->replot();
}
void MainWindow::swap_customplot_init(QVector<double> x, QVector<double> y)
{         //交换
    ui->swap_customplot->addGraph();
    ui->swap_customplot->graph(0)->setPen(QPen(QColor(70, 0, 70)));
    ui->swap_customplot->graph(0)->setBrush(QBrush(QColor(70, 0, 70, 50)));
    ui->swap_customplot->graph(0)->setData(x, y);
    ui->swap_customplot->xAxis->setLabel("time");
    ui->swap_customplot->yAxis->setLabel("swap%");
    ui->swap_customplot->xAxis->setRange(60, 0);
    ui->swap_customplot->yAxis->setRange(0, 100);
    ui->swap_customplot->xAxis->setTickLengthIn(0);
    ui->swap_customplot->xAxis->setSubTicks(false);
    ui->swap_customplot->yAxis->setTickLengthIn(0);
    ui->swap_customplot->yAxis->setSubTicks(false);
    ui->swap_customplot->replot();
}

void MainWindow::on_start_process_clicked()//打开进程
{
    QString filePath;
    filePath += QFileDialog::getOpenFileName(this, tr("Open File"), "/home", "*");
    qDebug() << filePath;
    if (filePath != "")
    {
        QProcess* qp;
        qp = new QProcess;
        qp->start(filePath, QIODevice::ReadWrite);
        //system(filePath.toLocal8Bit().data());
    }
}


void MainWindow::on_proc_button_clicked()
{
    QString qstrname = ui->procname_line->text();
    DIR *dp;
    bool tag = 0;
    struct dirent *entry;
    struct stat statbuf;
    if( (dp = opendir("/proc") ) ==NULL)
    {
        printf("failing to opendir\n");
        return;
    }
    int pid,ppid,dynice,i,memused;
    char proc[30],procinfo[200],state[3];
    char path[30];
    while( entry=readdir(dp) )
    {
        lstat(entry->d_name,&statbuf);

        int i,len=strlen(entry->d_name);
        for(i=0;i<len;++i)
        {
            if(entry->d_name[i]>'9' || entry->d_name[i]<'0')
                break;
        }
        if(i<len || ! S_ISDIR(statbuf.st_mode)) continue;

        //打开/proc/pid/stat
        sprintf(path,"/proc/%s/stat",entry->d_name);
        FILE *fp = fopen(path,"r");
        fgets(procinfo,199,fp);
        fclose(fp);
        sscanf(procinfo,"%d%s%s%d%d%d%d%d%d%d%d%d%d%d%d%d%d%d",
         &pid,proc,state,&ppid,&i,&i,&i,&i,&i,&i,&i,&i,&i,&i,&i,&i,&i,&dynice);

        QString Proc;
        for(int i=1;i<strlen(proc)-1;++i)
        {
            Proc += QString(proc[i]);
        }
        if(Proc==qstrname)
        {
            //打开/proc/pid/statm
            sprintf(path,"/proc/%s/statm",entry->d_name);
            fp = fopen(path,"r");
            fgets(procinfo,50,fp);
            fclose(fp);
            sscanf(procinfo,"%d%d",&i,&memused);

            QString st;
            switch (state[0]) {
            case 'S':
                st = "睡眠";
                break;
            case 'R':
                st = "运行";
                break;
            case 'D':
                st = "Disk Sleep";
                break;
            case 'T':
                st = "Tracing Stop";
                break;
            case 'Z':
                st = "Zombie";
                break;
            case 'X':
                st = "Dead";
                break;
            default:
                st = "Stopped";
                break;
            }
            QString text;
            text = "进程号："+QString::number(pid)+"\n父进程："+QString::number(ppid)+"\n状态："+st+"\n优先级："+
                    QString::number(dynice)+"\n内存占用："+QString::number(memused);
            QMessageBox::about(this,"进程信息",text);
            ui->procname_line->clear();
            tag = 1;
            break;
        }
    }
    if(tag==0)
        QMessageBox::about(this,"提示","未找到该进程。");
    closedir(dp);
}
