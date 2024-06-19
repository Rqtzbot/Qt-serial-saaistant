#ifndef RECEIVE_DATA_H
#define RECEIVE_DATA_H

#include <QObject>
#include <QThread>

class receive_data : public QThread
{
    Q_OBJECT
public:
    explicit receive_data(QObject *parent = nullptr);

signals:
    void sendportdata(QString data);
public slots:
    void receive(QByteArray data,bool checkstate1,bool checkstate2,bool checkstate3,bool checkstate4);

};

#endif // RECEIVE_DATA_H
