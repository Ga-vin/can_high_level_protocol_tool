#include "sockcanframe.h"
#include <QtCore>

SockCanFrame::SockCanFrame(QObject *parent) :
    QObject(parent),
    recv_time(QTime()),
    recv_protocol(QString("null")),
    recv_service(QString("null")),
    recv_src(0xFF),
    recv_dest(0x00),
    recv_len(0x0),
    frame_cnt(0)
{
    recv_data.clear();
}

SockCanFrame::~SockCanFrame()
{

}

void SockCanFrame::clear(void)
{
    this->recv_protocol = QString("null");
    this->recv_service  = QString("null");
    this->recv_src      = 0x0;
    this->recv_dest     = 0xFF;
    this->recv_len      = 0;
    this->frame_cnt     = 0;
    this->recv_data.clear();
}

void SockCanFrame::set_time(const QTime &tt)
{
    this->recv_time = tt;
}

void SockCanFrame::set_protocol(const QString &pro)
{
    if ( !pro.isEmpty()) {
        this->recv_protocol = pro;
    }
}

void SockCanFrame::set_service(const QString &ser)
{
    if ( !ser.isEmpty()) {
        this->recv_service = ser;
    }
}

void SockCanFrame::set_src(uchar src)
{
    this->recv_src = src;
}

void SockCanFrame::set_dest(uchar dest)
{
    this->recv_dest = dest;
}

void SockCanFrame::set_data(const QByteArray &data)
{
    this->recv_data = data;
}

void SockCanFrame::append_data(const QByteArray &data)
{
    this->recv_data.append(data);
}

void SockCanFrame::append_oid(const QByteArray &data)
{
    this->recv_oid.append(data);
}

void SockCanFrame::inc_len(int len)
{
    this->recv_len += len;
}

void SockCanFrame::inc_frame_cnt(int cnt)
{
    this->frame_cnt += cnt;
}

void SockCanFrame::set_type(uint tt)
{
    this->recv_type = tt;
}

QString SockCanFrame::get_time(void) const
{
    return (this->recv_time.toString("hh:mm:ss.zzz"));
}

QString SockCanFrame::get_protocol(void) const
{
    return (this->recv_protocol);
}

QString SockCanFrame::get_service(void) const
{
    return (this->recv_service);
}

QString SockCanFrame::get_src(void) const
{
    return (QString("%1").arg(this->recv_src));
}

QString SockCanFrame::get_dest(void) const
{
    return (QString("%1").arg(this->recv_dest));
}

QString SockCanFrame::get_data(void) const
{
    QString tmp = "";
    QString str = this->recv_data.toHex();

    for (int i = 0; i < str.length(); i += 2) {
        tmp += str.mid(i, 2) + " ";
    }

    return (tmp.trimmed().toUpper());
}

QString SockCanFrame::get_len() const
{
    return (QString("%1").arg(this->recv_len));
}

QString SockCanFrame::get_type() const
{
    QString str;

    if ( this->is_data_tramsmission_pro()) {
        switch ( this->recv_type) {
        case SockCanFrame::TX_NEED_ACK:
            str = QString("%1").arg(QObject::tr("发送需要回复"));
            break;

        case SockCanFrame::TX_NO_ACK:
            str = QString("%1").arg(QObject::tr("发送不需回复"));
            break;

        case SockCanFrame::RX_ACK:
            str = QString("%1").arg(QObject::tr("接收确认帧"));
            break;
        }
    } else if ( this->is_device_identify_pro()) {
        switch ( this->recv_type) {
        case SockCanFrame::DEV_IDENTIFY_REQ:
            str = QString("%1").arg(QObject::tr("设备识别申请帧"));
            break;

        case SockCanFrame::DEV_IDENTIFY_ALO:
            str = QString("%1").arg(QObject::tr("设备识别允许帧"));
            break;

        case SockCanFrame::DEV_IDENTIFY_BROAD:
            str = QString("%1").arg(QObject::tr("设备识别广播帧"));
            break;
        }
    }

    return (str);
}

QString SockCanFrame::get_frame_cnt() const
{
    return (QString("%1").arg(this->frame_cnt));
}

QByteArray SockCanFrame::get_oid() const
{
    return (this->recv_oid);
}

bool SockCanFrame::is_device_identify_pro(void) const
{
    if ( QString("%1").arg(QObject::tr("设备识别帧")) == this->recv_protocol) {
        return (true);
    } else {
        return (false);
    }
}

bool SockCanFrame::is_data_tramsmission_pro(void) const
{
    if ( QString("%1").arg(QObject::tr("数据传输帧")) == this->recv_protocol) {
        return (true);
    } else {
        return (false);
    }
}
