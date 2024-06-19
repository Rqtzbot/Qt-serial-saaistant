#include "mainwindow.h"
#include "./ui_mainwindow.h"

#include <QFileDialog>
#include <QMessageBox>
#include <QSerialPortInfo>
#include <fstream>
#include <iostream>
#include <QDebug>
#include <QString>
#include <QThread>
#include <QTime>
#include <QTimer>
#include <QHBoxLayout>
#include <QMouseEvent>
#include <QBitmap>
#include <QPainter>

#define stopport ":/stopload"
#define loadport ":/load"
#define bmostlittle ":/mostlittle"
#define bmostlarge ":/mostlarge"
#define bclose ":/close"
#define porticon ":/portico"
#define closeswitch ":/closeswitch"
#define onswitch ":/onswitch"
#define rclose ":/hclose"
#define rmostlittle ":/hlittel"
#define rmostlarge ":/hlarge"
#define logo ":/12"
#define wlogo ":/13"
#define root "D:"

//构造函数
MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    port = new QSerialPort(this);
    ui->openport->setIcon(QPixmap(stopport));
    ui->little->setPixmap(QPixmap(bmostlittle).scaled(16,16));
    ui->large->setPixmap(QPixmap(bmostlarge).scaled(16,16));
    ui->close->setPixmap(QPixmap(bclose).scaled(16,16));
    ui->icon->setPixmap(QPixmap(porticon).scaled(25,25));
    ui->switch_2->setPixmap(QPixmap(closeswitch).scaled(25,25));
    ui->openport->setIconSize(QSize(32,32));
    ui->openport->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);
    ui->checknum->setMaximumWidth(50);
    ui->parity_out->setMaximumWidth(60);
    ui->lengthbtn->setMinimumWidth(70);
    ui->checkbtn->setMinimumWidth(100);
    ui->binary->setMaximumWidth(60);
    ui->rclear->setMinimumWidth(100);
    ui->rsave->setMinimumWidth(100);
    ui->udapteport->setMinimumWidth(100);
    ui->senddata->setMinimumHeight(50);
    ui->length->setMaximumWidth(30);
    ui->portname->setMaximumWidth(240);
    ui->label->setMaximumHeight(30);
    ui->stoptimesend->setDisabled(true);
    ui->textEdit->setPlaceholderText("请输入16进制格式，并以空格分开.例如 01 3c");
    ui->rhex->setChecked(Qt::CheckState(2));
    ui->hex->setChecked(Qt::CheckState(2));
    //将控件设置为悬停控件，安装事件过滤器
    sethoverevent();
    //设置无边框
     setWindowFlags(Qt::FramelessWindowHint | Qt::WindowSystemMenuHint);
    //设置窗口标题
    setWindowTitle("串口调试助手");
    //遍历串口
    updateport();
    //设置标准波特率
    auto Bound = QSerialPortInfo::standardBaudRates();
    for(auto val: Bound)
        ui->bound->addItem(QString::number(val),val);
    ui->bound->setCurrentText("115200");
    //实例化子线程对象
    portreceiver = new receive_data(this);
    //连接主子线程信号
    connect(this,&MainWindow::senddataval,portreceiver,&receive_data::receive);
    //接受子线程处理后的数据
    connect(portreceiver,&receive_data::sendportdata,this,[=](QString val){
        //再加一个判断是因为不同模式的数据插入方式不一样insertHtml，insertPlainText
        if(ui->rhex->isChecked())
            ui->textBrowser->insertHtml(val);
        else
         ui->textBrowser->insertPlainText(val);
    });
    //窗口销毁时子线程退出
    connect(this,&MainWindow::destroyed,this,[=](){
        portreceiver->quit();
    });
    //链接串口信号
    connect(port,&QSerialPort::readyRead,this,[=](){
        static int k =0;
        k++;
        if(k==1)
            //启动线程
            portreceiver->start();
        //每次默认只读10个数据
        QByteArray arry = port->read(length);
        //将数据发送至子线程处理
        emit senddataval(arry,ui->timefreq->isChecked(),ui->rhex->isChecked(),ui->rtext->isChecked(),ui->rflie->isChecked());
    });
    //进制选择
    connect(ui->binary,&QComboBox::currentTextChanged,this,[=](QString text)
    {
        if(text == "HEX")
        {
            ui->parity_out->setText(QString("%1").arg(check_num,0,16));
        }
        else if(text == "DEC")
        {
            ui->parity_out->setText(QString("%1").arg(check_num,0,10));
        }
        else if(text == "BIN")
        {
            ui->parity_out->setText(QString("%1").arg(check_num,0,2));
        }

    });
    //校验选择
    connect(ui->parity_check,&QComboBox::currentTextChanged,this,[=](QString text)
    {
        if(text == "异或校验")
        {
            ui->parity_in->setPlaceholderText("");
            ui->binary->setDisabled(false);

        }
        else if(text == "奇校验")
        {
            ui->parity_in->setPlaceholderText("请输入二进制");
            ui->binary->setCurrentText("BIN");
            ui->binary->setDisabled(true);
        }
        else if(text == "偶校验")
        {
            ui->parity_in->setPlaceholderText("请输入二进制");
            ui->binary->setCurrentText("BIN");
            ui->binary->setDisabled(true);
        }
        ui->parity_in->clear();
        ui->parity_out->clear();

    });
    //发送模式与显示模式的信号连接
    connect(ui->hex,&QRadioButton::clicked,this,[=](){
         ui->textEdit->setPlaceholderText("请输入16进制格式，并以空格分开.例如 01 3c");
    });
    connect(ui->text,&QRadioButton::clicked,this,[=](){
        ui->textEdit->setPlaceholderText("");
    });
    connect(ui->rhex,&QRadioButton::clicked,this,[=](){
        ui->rtext->setCheckState(Qt::CheckState(0));
        ui->rflie->setCheckState(Qt::CheckState(0));
    });
    connect(ui->rtext,&QRadioButton::clicked,this,[=](){
        ui->rhex->setCheckState(Qt::CheckState(0));
        ui->rflie->setCheckState(Qt::CheckState(0));
    });
    connect(ui->rflie,&QRadioButton::clicked,this,[=](){
        ui->rhex->setCheckState(Qt::CheckState(0));
        ui->rtext->setCheckState(Qt::CheckState(0));
    });

    //重写窗口关闭，最小化，最大化，标题栏
    connect(this,&MainWindow::hoverval,this,&MainWindow::hoveraction);
    connect(this,&MainWindow::clickval,this,&MainWindow::clickaction);

    //设置时间+定时发送
    QLabel *lab2 = new QLabel(this);
    timer1 = new QTimer(this);
    timer2 = new QTimer(this);
    timer1->start(1000);
    connect(timer2,&QTimer::timeout,this,[=](){
        if(ui->hex->isChecked())
            shex();
        else if(ui->text->isChecked())
            stext();
    });
    connect(timer1, &QTimer::timeout, this, [=]()
    {
        QDateTime curTime = QDateTime::currentDateTime();
        QString str = curTime.toString("当前时间: yyyy-MM-dd hh:mm:ss");
        lab2->setText(str);
        ui->statusbar->addWidget(lab2);
    });
    connect(ui->timesend,&QCheckBox::stateChanged,this,[=](int state){
    {
        if(state==2){
            timer2->start(ui->timevalue->text().toInt());
            ui->stoptimesend->setDisabled(false);
        }
        else if(state==0){
            timer2->stop();
            ui->stoptimesend->setDisabled(true);
        }
    }
    });
    //设置底部状态栏statusbar，超链接+发送次数
    lab4 = new QLabel(this);
    lab4->setPixmap(QPixmap(logo).scaled(61,31));
    ui->statusbar->addWidget(lab4);//logo

    QLabel *lab3 = new QLabel(this);
    lab3->setText("发送次数: ");
    ui->statusbar->addWidget(lab3);
    edit = new QLineEdit(this);
    edit->setStyleSheet("QLineEdit{border-color:#999999;border-radius: 7px; border-style: solid; border-width: 1px; padding: 1px;}");
    edit->setMaximumWidth(30);
    ui->statusbar->addWidget(edit);

    QLabel *lab = new QLabel(this);
    lab->setText(QStringLiteral("<a style='color: red; font: 10pt \"微软雅黑\";text-decoration: none ;font-size:15px' href =https://blog.csdn.net/m0_75192474?type=blog><u>CSDN博客</u>"));
    lab->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(lab);

    QLabel *lab1 = new QLabel(this);
    lab1->setText(QStringLiteral("<a style='color: red; font: 10pt \"微软雅黑\";text-decoration: none ;font-size:15px' href =https://github.com/Rqtzbot><u>GitHup源码</u>"));
    lab1->setOpenExternalLinks(true);
    ui->statusbar->addPermanentWidget(lab1);
}
//析构(细狗)函数
MainWindow::~MainWindow()
{
    delete ui;
}
//向串口发送数据
void MainWindow::senddata()
{
    if(ui->arm->isChecked())
    {
        hextoint();
        arm();
    }
    else if(ui->base->isChecked())
    {    hextoint();
         base();
    }
    else if(ui->hex->isChecked())
        shex();
    else if(ui->text->isChecked())
        stext();
    else if(ui->textEdit->toPlainText() == "")
       QMessageBox::warning(this,"警告","输入不能为空");
    else
       QMessageBox::warning(this,"警告","请选择发送数据格式");

}
//16进制转为10进制
void MainWindow::hextoint()
{
    QStringList data;
    //将数据以空格分开
    data = ui->textEdit->toPlainText().split(" ");
    //将数据转为10进制
    for(int i = 0;i<data.size();i++)
    { 
      bool ok;
      hexdata[i] = data.at(i).toInt(&ok,16);
    }
}
//底盘模式
void MainWindow::base()
{}
//机械臂模式
void MainWindow::arm() 
{
    //机械臂通信协议
    sendbuffer[0] = 0x5a;
    sendbuffer[1] = 0xa5;
    sendbuffer[2] = hexdata[0] & 0x00ff;
    sendbuffer[3] = hexdata[1] & 0x00ff;
    sendbuffer[4] = hexdata[2] & 0x00ff;
    sendbuffer[5] = hexdata[3] & 0x00ff;
    sendbuffer[6] = hexdata[4] & 0x00ff;
    sendbuffer[7] = 0x00;
    sendbuffer[8] = 0x0d;
    sendbuffer[9] = 0x0a;

    QByteArray buffer;
    //异或校验校验位
    unsigned char checknum = 0;
    for(int i=2;i<10;i++)
    {
        checknum^=sendbuffer[i];
        //16进制显示
//        qDebug()<<QString("%1").arg(sendbuffer[i],0,16);/
    }
    sendbuffer[10] = checknum;
    //添加到Qbytearry中
    for(int k=0;k<11;k++)
    {
        buffer.append(sendbuffer[k]);
    }
    ui->checknum->setText(QString("%1").arg(checknum,0,16));
    //将数据写入串口
    port->write(buffer,11);
}
//16进制模式
void MainWindow::shex()
{
    auto data = ui->textEdit->toPlainText() + (ui->newrow->isChecked() ? "\r\n":"");
    port->write(data.toLocal8Bit());
}
//文本模式
void MainWindow::stext()
{
    auto data = ui->textEdit->toPlainText() + (ui->newrow->isChecked() ? "\r\n":"");
    port->write(data.toLocal8Bit());
}
//将控件设置为悬停控件，将控件安装事件过滤器
void MainWindow::sethoverevent()
{
    ui->close->setAttribute(Qt::WA_Hover,true);
    ui->close->installEventFilter(this);
    ui->little->setAttribute(Qt::WA_Hover,true);
    ui->little->installEventFilter(this);
    ui->large->setAttribute(Qt::WA_Hover,true);
    ui->large->installEventFilter(this);
    ui->switch_2->installEventFilter(this);
}
//设置按钮style样式
QString MainWindow::setbtnstyle(QString val)
{
    if(val=="after")
        return "QPushButton{\
                font: 9pt \"微软雅黑\";\
                color: rgb(255,255,255);\
                background-color: rgb(60, 60, 60);\
                border-color:#c3c3c3;\
                border-radius: 14px;\
                border-style: solid;\
                border-width: 1px;\
                padding: 4px;\
            }\
            QPushButton::hover {\
            background-color: rgb(150, 150, 150);\
            border-color: rgb(0,120,212);\
            }";
    else if(val=="before")
    {
        return "QPushButton{\
                font: 9pt \"微软雅黑\";\
                color: rgb(0,0,0);\
                background-color: rgb(255, 255, 255);\
                border-color:#c3c3c3;\
                border-radius: 14px;\
                border-style: solid;\
                border-width: 1px;\
                padding: 4px;\
            }\
            QPushButton::hover {\
            background-color: rgb(224,238,249);\
            border-color: rgb(0,120,212);\
            }";
    }
}
//遍历串口
void MainWindow::updateport()
{
    foreach(const QSerialPortInfo& info,QSerialPortInfo::availablePorts())
    {
        ui->portname->addItem(info.portName()+":"+info.description(),info.portName());
    }
}
//打开串口
void MainWindow::on_openport_clicked()
{
    if(!portflag)
    {
        port->setPortName(ui->portname->currentData().toString());
        port->setBaudRate(ui->bound->currentText().toInt());
        port->setDataBits(QSerialPort::DataBits(ui->databit->currentText().toInt()));
        port->setStopBits(QSerialPort::StopBits(ui->stopbit->currentText().toInt()));
        switch(ui->partbit->currentIndex())
        {
        case 0:
            port->setParity(QSerialPort::NoParity);
            break;
        case 1:
            port->setParity(QSerialPort::EvenParity);
            break;
        case 2:
            port->setParity(QSerialPort::OddParity);
            break;
        default:
            port->setParity(QSerialPort::NoParity);
            break;
        }
        port->setFlowControl(QSerialPort::NoFlowControl);
        port->open(QIODevice::ReadWrite);
        if(port->isOpen())
        {
            QMessageBox::information(this,"提示","串口打开成功！");
            ui->openport->setText("关闭串口");
            ui->openport->setIcon(QIcon(loadport));
            ui->bound->setDisabled(true);
            ui->partbit->setDisabled(true);
            ui->stopbit->setDisabled(true);
            ui->databit->setDisabled(true);

            portflag = true;

        }
        else {
            QMessageBox::warning(this,"警告","串口打开失败！"+port->errorString());
        }
    }
    else{
        portflag = false;
        port->close();
        ui->openport->setText("打开串口");
        ui->openport->setIcon(QIcon(stopport));
        ui->bound->setDisabled(false);
        ui->partbit->setDisabled(false);
        ui->stopbit->setDisabled(false);
        ui->databit->setDisabled(false);
    }

}
//更新串口
void MainWindow::on_udapteport_clicked()
{
    updateport();
}
//清除接收区
void MainWindow::on_rclear_clicked()
{
    ui->textBrowser->clear();
}
//发送数据
void MainWindow::on_senddata_clicked()
{
    if(port->isOpen())
    {
        senddata();
        send_count++;
        edit->setText(QString::number(send_count));
    }
    else
        QMessageBox::warning(this,"警告","串口未打开");

}
//清除发送区
void MainWindow::on_sclear_clicked()
{
    ui->textEdit->clear();
}
//保存发送区数据到文件
void MainWindow::on_rsave_clicked()
{
    QStringList filename = QFileDialog::getOpenFileNames(this,tr("选择文件"),root,tr("*txt"));
    QString names;
    for(int i=0;i<filename.size();i++)
    {
        names += filename.at(i);
    }
    std::ofstream flie(names.toStdString(),std::ios::app);
    flie << ui->textBrowser->toPlainText().toStdString();
    QMessageBox::information(this,"提示","成功写入"+names+"文件中");
}
//校验
void MainWindow::on_checkbtn_clicked()
{
    //异或校验
    if(ui->parity_in->text()!="" && ui->parity_check->currentIndex()==0 )
    {
        QStringList data;
        data = ui->parity_in->text().split(" ");
        unsigned char intdata[20];
        check_num = 0;
        for(int k=0;k<data.size();k++)
        {
            bool ok;
            intdata[k]=data.at(k).toInt(&ok,16);
        }

        for (int i = 0;i<data.size();i++) {
            check_num^=intdata[i];
        }
        if(ui->binary->currentIndex()==0)
           ui->parity_out->setText(QString("%1").arg(check_num,0,16));
        else if(ui->binary->currentIndex()==1)
            ui->parity_out->setText(QString("%1").arg(check_num,0,10));
        else if(ui->binary->currentIndex()==2)
            ui->parity_out->setText(QString("%1").arg(check_num,0,2));
    }
    //奇校验
    else if(ui->parity_in->text()!="" && ui->parity_check->currentIndex()==1)
    {

        QString bin = ui->parity_in->text();
        int numcount=0;
        for(QString bit:bin)
        {
            if(bit == "1")
                numcount++;
        }
        if((numcount+1)%2==0)
        {
            QString aimbin = bin+"0";
            ui->parity_out->setText(aimbin);
            numcount=0;
        }

        else
        {
            QString aimbin2 = bin+"1";
            ui->parity_out->setText(aimbin2);
            numcount=0;
        }

    }
    //偶校验
    else if(ui->parity_in->text()!="" && ui->parity_check->currentIndex()==2)
    {
        QString bin = ui->parity_in->text();
        int numcount=0;
        for(QString bit:bin)
        {
            if(bit == "1")
                numcount++;
        }
        if((numcount)%2==0)
        {
            QString aimbin = bin+"0";
            ui->parity_out->setText(aimbin);
            numcount=0;
        }

        else
        {
            QString aimbin2 = bin+"1";
            ui->parity_out->setText(aimbin2);
            numcount=0;
        }
    }
    else
        QMessageBox::warning(this,"警告","输入为空！");

}
//长度设置
void MainWindow::on_lengthbtn_clicked()
{
    length = ui->length->text().toInt();
}
//鼠标悬停后槽函数
void MainWindow::hoveraction(QObject *obj, QString name)
{
    if(name=="close") {
        ui->close->setPixmap(QPixmap(rclose).scaled(16,16));
    }
    else if (name=="little") {
        ui->little->setPixmap(QPixmap(rmostlittle).scaled(16,16));
    }
    else if (name=="large") {
        ui->large->setPixmap(QPixmap(rmostlarge).scaled(16,16));
    }

    else if (name=="mouseout") {
        ui->little->setPixmap(QPixmap(bmostlittle).scaled(16,16));
        ui->large->setPixmap(QPixmap(bmostlarge).scaled(16,16));
        ui->close->setPixmap(QPixmap(bclose).scaled(16,16));
    }
}
//鼠标点击无click信号控件(QLabel)的槽函数
void MainWindow::clickaction(QObject *obj, QString name)
{
    if(name=="close") {
       //关闭前查看资源是否关闭
        if(timer1->isActive())
            timer1->stop();
        if(timer2->isActive())
            timer2->stop();
        if(port->isOpen())
            port->close();
         this->close();
    }
    else if (name=="little") {
            this->showMinimized();
    }
    else if (name=="large") {
        static int num = 0;
        num++;
        qDebug()<<num;
        if((num+1)%2==0)
           this->showFullScreen();
        else
            this->showNormal();
    }
    else if (name=="switch_2") {
        if((theammode_count+1)%2==0)
        {
            ui->switch_2->setPixmap(QPixmap(onswitch).scaled(25,25));
            this->setStyleSheet("background-color: rgb(60, 60, 60); color:rgb(255, 255, 255);");
            ui->textEdit->setStyleSheet("QTextEdit{background-color: rgb(60, 60, 60);font: 10pt \"微软雅黑\"; border-color:#c3c3c3; border-radius: 18px;border-style: solid;border-width: 1px; padding: 2px; }");
            ui->textBrowser->setStyleSheet("QTextEdit{background-color: rgb(60, 60, 60);font: 10pt \"微软雅黑\"; border-color:#c3c3c3; border-radius: 18px;border-style: solid;border-width: 1px; padding: 2px; }");
            ui->rclear->setStyleSheet(setbtnstyle("after"));
            ui->rsave->setStyleSheet(setbtnstyle("after"));
            ui->udapteport->setStyleSheet(setbtnstyle("after"));
            ui->senddata->setStyleSheet(setbtnstyle("after"));
            ui->checkbtn->setStyleSheet(setbtnstyle("after"));
            ui->sclear->setStyleSheet(setbtnstyle("after"));
            ui->lengthbtn->setStyleSheet(setbtnstyle("after"));
            ui->stoptimesend->setStyleSheet(setbtnstyle("after"));
            ui->sendflie->setStyleSheet(setbtnstyle("after"));
            ui->selflie->setStyleSheet(setbtnstyle("after"));
            ui->openport->setStyleSheet("QToolButton{\
                                        font: 75 12pt \"微软雅黑\";\
                                        color: rgb(255, 255, 255);\
                                        background-color:rgb(60, 60, 60);\
                                        border-color:#a5a5a5;\
                                        border-radius: 15px;\
                                        border-style: solid;\
                                        border-width: 1px;\
                                        padding: 3px;\
                                    }\
                                    QToolButton::hover {\
                                    background-color: rgb(150, 150, 150);\
                                    border-color: rgb(0,120,212);\
                                    }");
            lab4->setPixmap(QPixmap(wlogo).scaled(61,31));

        }

        else {
            ui->switch_2->setPixmap(QPixmap(closeswitch).scaled(25,25));
            this->setStyleSheet("background-color:; color:;");
            ui->textEdit->setStyleSheet("QTextEdit{background-color: rgb(255, 255, 255);font: 10pt \"微软雅黑\"; border-color:#c3c3c3; border-radius: 18px;border-style: solid;border-width: 1px; padding: 2px; }");
            ui->textBrowser->setStyleSheet("QTextEdit{background-color: rgb(255, 255, 255);font: 10pt \"微软雅黑\"; border-color:#c3c3c3; border-radius: 18px;border-style: solid;border-width: 1px; padding: 2px; }");
            ui->rclear->setStyleSheet(setbtnstyle("before"));
            ui->rsave->setStyleSheet(setbtnstyle("before"));
            ui->udapteport->setStyleSheet(setbtnstyle("before"));
            ui->senddata->setStyleSheet(setbtnstyle("before"));
            ui->checkbtn->setStyleSheet(setbtnstyle("before"));
            ui->sclear->setStyleSheet(setbtnstyle("before"));
            ui->stoptimesend->setStyleSheet(setbtnstyle("before"));
            ui->lengthbtn->setStyleSheet(setbtnstyle("before"));
            ui->sendflie->setStyleSheet(setbtnstyle("before"));
            ui->selflie->setStyleSheet(setbtnstyle("before"));
            ui->openport->setStyleSheet("QToolButton{\
                                        font: 75 12pt \"微软雅黑\";\
                                        color: #ff2b2b;\
                                        background-color:#ffffff;\
                                        border-color:#a5a5a5;\
                                        border-radius: 15px;\
                                        border-style: solid;\
                                        border-width: 1px;\
                                        padding: 3px;\
                                    }\
                                    QToolButton::hover {\
                                    background-color:rgb(224,238,249);\
                                    border-color: rgb(0,120,212);\
                                    }");
            lab4->setPixmap(QPixmap(logo).scaled(61,31));
        }
    }

}
//重写虚函数，控制无边框窗口移动
void MainWindow::mouseMoveEvent(QMouseEvent *e)
{
    if(mouseflag)
    {
        QPoint dis = e->globalPos()-mousepose;
        this->move(windowpose+dis);
    }
}
//重写虚函数，捕捉鼠标按压信号
void MainWindow::mousePressEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
    {
        mouseflag = true;
        mousepose = e->globalPos();
        windowpose = this->geometry().topLeft();
    }
}
//重写虚函数，捕捉鼠标松开信号
void MainWindow::mouseReleaseEvent(QMouseEvent *e)
{
    if(e->button() == Qt::LeftButton)
        mouseflag = false;
}
//重写事件过滤器，获取悬浮，点击信号
bool MainWindow::eventFilter(QObject *obj, QEvent *e)
{
    if(e->type() == QEvent::HoverEnter)
    {
        emit hoverval(obj,obj->objectName());
        return  true;
    }
    else if(e->type() == QEvent::HoverLeave)
    {
        emit hoverval(obj,"mouseout");
        return true;
    }
    else if(e->type() == QEvent::MouseButtonPress)
    {
        theammode_count++;
        emit clickval(obj,obj->objectName());
        return true;
    }
    else
    {
        return  QObject::eventFilter(obj, e);
    }

}
//重写paintEvent事件，保持圆角
void MainWindow::paintEvent(QPaintEvent *e)
{
    QBitmap bmp(this->size());
    bmp.fill();
    QPainter p(&bmp);
    p.setPen(Qt::NoPen);
    p.setBrush(Qt::black);
    p.drawRoundedRect(bmp.rect(),20,20);//像素为20的圆角
    setMask(bmp);
}
//停止定时发送
void MainWindow::on_stoptimesend_clicked()
{
    timer2->stop();
    ui->timesend->setCheckState(Qt::CheckState(0));
    ui->stoptimesend->setDisabled(true);
}
//选择文件
void MainWindow::on_selflie_clicked()
{
   QStringList filename = QFileDialog::getOpenFileNames(this,tr("选择文件"),root,"tr(*txt)");
   QString names;
   for(int i=0;i<filename.size();i++)
   {
       names += filename.at(i);
   }
   ui->fliepath->setText(names);
}
//发送文件
void MainWindow::on_sendflie_clicked()
{
    QFile file(ui->fliepath->text());
    if(!file.open(QIODevice::ReadOnly))
    {
        QMessageBox::warning(this,"警告","文件打开失败"+file.errorString());
    }
    else {
        port->write(file.readAll());
    }
}
//16进制发送时，判断是否符合格式要求
void MainWindow::on_textEdit_textChanged()
{
    if(ui->hex->isChecked())
    {
        QString data = ui->textEdit->toPlainText();
        for (int i =0;i<data.size();i++) {
            if(!((data.at(i)>='0'&& data.at(i)<='9') || (data.at(i)==' ') || (data.at(i)>='a' && data.at(i)<='f') || (data.at(i)>='A' && data.at(i)<="F")))
                QMessageBox::warning(this,"警告","格式错误，请输入0-9，a-f,A-F");
        }
    }
}
