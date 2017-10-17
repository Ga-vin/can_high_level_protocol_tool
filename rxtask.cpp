#include <QtCore>
#include "rxtask.h"
#include "sockcandata.h"
#include "sockcanframe.h"

RxTask::RxTask(QObject *parent) :
    QThread(parent),
    is_start(false),
    is_finish(false),
    is_stop(false)
{
    p_udp_socket = new QUdpSocket;
    this->p_udp_socket->bind(QHostAddress::Any, RxTask::DEFAULT_PORT);

    p_can_frame = new SockCanFrame;
}

RxTask::RxTask(quint16 port, QObject *parent) :
    QThread(parent),
    is_start(false),
    is_finish(false),
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
    } else {
        qDebug() << "create udp socket ok";
    }

    int status = this->p_udp_socket->bind(QHostAddress::Any, this->port);
    if ( !status) {
        qDebug() << "bind socket error";
    } else {
        qDebug() << "bind socket ok";
    }

    p_can_frame = new SockCanFrame(this);
    if ( !p_can_frame) {
        qDebug() << "create can socket frame error";
    } else {
        qDebug() << "create can socket frame ok";
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

            data_handler(data);
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

void RxTask::data_handler(const QByteArray &byte)
{
    if ( byte.size() != RxTask::DEFAULT_DATA_SIZE) {
        qDebug() << "data received size is not correct. It is " << byte.size() << " now";
        return ;
    } else {
        qDebug() << "data received size is correct";
    }

#ifndef __DEBUG_
    qDebug() << SockCanData::bytearray_to_hex_str(byte);
#endif __DEBUG_

    p_can_data      = new SockCanData(byte);
    char *p_new_tmp = (char *)(p_can_data->can_data());

#ifdef __DEBUG_
    qDebug() << "ID: " << p_can_data->debug_id();
    qDebug() << "CHANNEL: " << p_can_data->debug_channel();
    qDebug() << "EXT: " << p_can_data->debug_ext();
    qDebug() << "RTR: " << p_can_data->debug_rtr();
    qDebug() << "LEN: " << p_can_data->debug_len();
    qDebug() << "DATA: " << p_can_data->debug_data();
    return ;
#endif __DEBUG_

    if ( this->p_can_data->is_device_identify_pro()) {
        qDebug() << "device identify frame";
        /* Device identify */
        this->is_start  = true;
        this->is_finish = true;
        this->p_can_frame->set_protocol(QString("%1").arg(QObject::tr("设备识别帧")));
        this->p_can_frame->set_service(QString("null"));
        this->p_can_frame->set_time(QTime::currentTime());
        this->p_can_frame->set_type(p_can_data->ctrl());
        this->p_can_frame->set_src(0xF0);

        if ( p_can_data->is_dev_identify_broad()) {
            qDebug() << "broad";
            this->p_can_frame->set_dest(0xFF);
            char by[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            this->p_can_frame->append_oid(QByteArray(by));
        } else if ( p_can_data->is_dev_identify_allow()){
            qDebug() << "allow";
            this->p_can_frame->set_dest(*(p_new_tmp + 3));

            uint oid     = p_can_data->oid_upper();
            p_new_tmp[4] = oid>>16&0xFF;
            p_new_tmp[5] = oid>>8&0xFF;
            p_new_tmp[6] = oid>>0&0xFF;
            QByteArray by(p_new_tmp+4, 3);
            by.append(p_new_tmp, 3);
            this->p_can_frame->append_oid(by);
        } else {
            qDebug() << "unknown";
            this->is_start  = false;
            this->is_finish = false;
        }
    } else if ( this->p_can_data->is_data_trans_pro()) {
        qDebug() << "data transmission";
        /* Data transmission */
        if ( this->p_can_data->is_ind_frame() || this->p_can_data->is_first_frame()) {
            qDebug() << "first or indenpent";
            /* First frame or Indenpendent frame */
            this->p_can_frame->clear();
            this->is_start = true;

            /* 填充帧信息 */
            this->p_can_frame->set_protocol(QString("%1").arg(QObject::tr("数据传输帧")));

            /* 填充源和目的地址 */
            this->p_can_frame->set_src(p_can_data->src());
            this->p_can_frame->set_dest(p_can_data->dest());

            can_msg_header_t header;
            QString          trans_service;
            memcpy(&header, p_new_tmp, sizeof(can_msg_header_t));
            switch ( header.tran) {
            case SockCanData::INFO_TRANS:
                trans_service = QString("%1").arg(QObject::tr("信息传输协议"));
                break;

            case SockCanData::TIME_TRANS:
                trans_service = QString("%1").arg(QObject::tr("时间传输协议"));
                break;

            case SockCanData::ON_BOARD:
                trans_service = QString("%1").arg(QObject::tr("在轨快速测试协议"));
                break;

            case SockCanData::FILE_TRANS:
                trans_service = QString("%1").arg(QObject::tr("文件传输协议"));
                break;

            case SockCanData::DEV_DRIVER:
                trans_service = QString("%1").arg(QObject::tr("设备驱动协议"));
                break;

            case SockCanData::TM_TRANS:
                trans_service = QString("%1").arg(QObject::tr("遥测传输协议"));
                break;

            case SockCanData::TC_TRANS:
                trans_service = QString("%1").arg(QObject::tr("遥控传输协议"));
                break;

            case SockCanData::MEM_DOWN:
                trans_service = QString("%1").arg(QObject::tr("内存下载协议"));
                break;

            case SockCanData::BUS_MANAGE:
                trans_service = QString("%1").arg(QObject::tr("总线管理维护协议"));
                break;
            }

            this->p_can_frame->set_service(trans_service);
            this->p_can_frame->inc_frame_cnt();

            if ( this->p_can_data->is_ind_frame()) {
                qDebug() << "indpendent";
                this->is_finish = true;
            }
        } else if ( (this->is_start) && (this->p_can_data->is_last_frame())) {
            qDebug() << "last";
            /* Last frame */
            this->is_finish         = true;
            this->p_can_frame->inc_len(this->p_can_data->can_len());
            this->p_can_frame->append_data(p_new_tmp);
            this->p_can_frame->inc_frame_cnt();
        } else {
            qDebug() << "middle";
            this->p_can_frame->inc_len(8);
            this->p_can_frame->append_data(p_new_tmp);
            this->p_can_frame->inc_frame_cnt();
        }
    } else {
        qDebug() << "invalid";
        return ;
    }

    if ( this->is_start && this->is_finish) {
        qDebug() << "emit";
        this->is_start  = false;
        this->is_finish = false;

        emit notify_can_frame(this->p_can_frame);
    }

    delete p_can_data;
}

void RxTask::update_stop_flag()
{
    QMutexLocker locker(&this->locker);
    this->is_stop = true;
}
