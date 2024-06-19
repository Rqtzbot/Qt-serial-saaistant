#include "receive_data.h"
#include <QTime>
receive_data::receive_data(QObject *parent) : QThread(parent)
{

}


void receive_data::receive(QByteArray data,bool checkstate1,bool checkstate2,bool checkstate3,bool checkstate4)
{
    if(checkstate2){
        QString hexdata = QString("<font color= 'red' >%1</font>").arg(checkstate1 ? QDateTime::currentDateTime().toString("[hh:mm:ss] :"):"")+QString::fromLocal8Bit(data.toHex(' ').toUpper());
        emit sendportdata(hexdata);
    }
    else if(checkstate3){
        QString plaindata =(checkstate1 ? QDateTime::currentDateTime().toString("[hh:mm:ss] :"):"")+QString::fromLocal8Bit(data);
        emit sendportdata(plaindata);
    }
    else if(checkstate4){
        QString fliedata = QString::fromUtf8(data);
        emit sendportdata(fliedata);
    }
}
