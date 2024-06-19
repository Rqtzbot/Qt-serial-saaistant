#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QLineEdit>
#include <QObject>
#include <QMainWindow>
#include <QSerialPort>
#include <QLabel>
#include "receive_data.h"
QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
    void updateport();
    void senddata();
    void hextoint();
    void base();
    void arm();
    void shex();
    void stext();
    void sethoverevent();
    QString setbtnstyle(QString val);

private slots:
    void on_openport_clicked();

    void on_udapteport_clicked();

    void on_rclear_clicked();

    void on_senddata_clicked();

    void on_sclear_clicked();

    void on_rsave_clicked();

    void on_checkbtn_clicked();

    void on_lengthbtn_clicked();

    void hoveraction(QObject *obj, QString name);

    void clickaction(QObject *obj, QString name);

    void on_stoptimesend_clicked();

    void on_selflie_clicked();

    void on_sendflie_clicked();

    void on_textEdit_textChanged();

signals:
    void hoverval(QObject *obj, QString name);
    void clickval(QObject *obj, QString name);
    void senddataval(QByteArray data,bool checkstate1,bool checkstate2,bool checkstate3,bool checkstate4);
protected:
      void mouseMoveEvent(QMouseEvent *e) override;
      void mousePressEvent(QMouseEvent *e) override;
      void mouseReleaseEvent(QMouseEvent *e) override;
      bool eventFilter(QObject*obj,QEvent *e) override;
      void paintEvent(QPaintEvent* e) override;

private:
    Ui::MainWindow *ui;
    QLabel *lab4;
    QTimer* timer1;
    QTimer* timer2;
    QPoint mousepose;
    QPoint windowpose;
    bool mouseflag;
    QSerialPort *port;
    bool portflag = false;
    unsigned char sendbuffer[100];
    unsigned char hexdata[11];
    unsigned char check_num = 0;
    int length = 10;
    int send_count = 0;
    int theammode_count = 0;
    QLineEdit *edit;
    receive_data *portreceiver;


};
#endif // MAINWINDOW_H
