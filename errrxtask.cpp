#include "errrxtask.h"
#include <QMutex>

ErrRxTask::ErrRxTask(QObject *parent) :
    QThread(parent),
    is_stop(false)
{
    this->port           = ErrRxTask::DEFAULT_ERR_PORT;
    this->err_udp_socket = new QUdpSocket;
    this->err_udp_socket->bind(QHostAddress::Any, this->port);
}

ErrRxTask::ErrRxTask(ushort port, QObject *parent) :
    QThread(parent),
    is_stop(false)
{
    this->port           = port;
    this->err_udp_socket = new QUdpSocket;
    this->err_udp_socket->bind(QHostAddress::Any, this->port);
}

ErrRxTask::~ErrRxTask()
{
    if ( this->err_udp_socket) {
        this->err_udp_socket->disconnect();
        this->err_udp_socket->close();
        delete this->err_udp_socket;
    }
}

void ErrRxTask::run()
{
    while ( true) {
        if ( this->err_udp_socket->hasPendingDatagrams()) {
            QHostAddress send;
            QByteArray   byte;

            byte.resize(this->err_udp_socket->pendingDatagramSize());
            this->err_udp_socket->readDatagram(byte.data(), byte.size(), &send, &(this->port));

            emit notify_err_frame(byte);
        }

        QMutexLocker locker(&this->lock);
        if ( this->is_stop) {
            break;
        }
    }
}

void ErrRxTask::stop()
{
    this->is_stop = true;
}

void ErrRxTask::update_stop_flag()
{
    QMutexLocker locker(&this->lock);
    this->is_stop = true;
}
