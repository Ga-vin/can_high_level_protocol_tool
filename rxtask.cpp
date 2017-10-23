#include <QtCore>
#include "rxtask.h"
#include "sockcandata.h"
#include "sockcanframe.h"

RxTask::RxTask(QObject *parent) :
    QThread(parent),
    is_stop(false)
{
    p_udp_socket = new QUdpSocket;
    this->p_udp_socket->bind(QHostAddress::Any, RxTask::DEFAULT_PORT);
}

RxTask::RxTask(quint16 port, QObject *parent) :
    QThread(parent),
    is_stop(false)
{
    if ( port < 1000) {
        return ;
    } else {
        this->port = port;
    }

    p_udp_socket = new QUdpSocket;
    if ( !p_udp_socket) {
        qDebug() << "create udp socket error";
    }

    int status = this->p_udp_socket->bind(QHostAddress::Any, this->port);
    if ( !status) {
        qDebug() << "bind socket error";
    }
}

RxTask::~RxTask()
{
    this->p_udp_socket->disconnect();
    this->p_udp_socket->close();
    delete this->p_udp_socket;
}

void RxTask::run(void)
{
    while ( true) {
        if ( this->p_udp_socket->hasPendingDatagrams()) {
            uint         data_len = this->p_udp_socket->pendingDatagramSize();
            QByteArray   data;
            QHostAddress send;

            data.resize(data_len);
            this->p_udp_socket->readDatagram(data.data(), data_len, &send, &this->port);
            this->start_time = QTime::currentTime();

            emit notify_can_frame(data);
        }

        QMutexLocker locker(&this->locker);
        if ( this->is_stop) {
            break;
        }
    }
}

void RxTask::stop()
{
    //QMutexLocker locker(&this->locker);
    this->is_stop = true;
}

void RxTask::update_stop_flag()
{
    QMutexLocker locker(&this->locker);
    this->is_stop = true;
}
