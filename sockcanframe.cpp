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
    //recv_oid.resize(6);
}

SockCanFrame::SockCanFrame(const SockCanFrame &obj)
{
    this->recv_time = obj.get_time_orig();
    this->recv_protocol = obj.get_protocol();
    this->recv_service  = obj.get_service();
    this->recv_src      = obj.get_src_orig();
    this->recv_dest     = obj.get_dest_orig();
    this->recv_len      = obj.get_len_value();
    this->frame_cnt     = obj.get_frame_cnt_orig();

    this->recv_data     = obj.get_data_orig();
    this->recv_oid      = obj.get_oid();
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
    this->recv_oid.clear();
    this->recv_data.resize(0);
    this->recv_oid.resize(0);
    //this->recv_oid.resize(6);
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

void SockCanFrame::set_len(uint len)
{
    this->recv_len = len;
}

QString SockCanFrame::get_time(void) const
{
    return (this->recv_time.toString("hh:mm:ss.zzz"));
}

QTime SockCanFrame::get_time_orig() const
{
    return (this->recv_time);
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
    return (QString("0x") + QString("%1").arg(this->recv_src, 0, 16).toUpper());
}

uchar SockCanFrame::get_src_orig() const
{
    return (this->recv_src);
}

QString SockCanFrame::get_dest(void) const
{
    return (QString("0x") + QString("%1").arg(this->recv_dest, 0, 16).toUpper());
}

uchar SockCanFrame::get_dest_orig() const
{
    return (this->recv_dest);
}

QString SockCanFrame::get_data(void) const
{
    QString tmp = "";
    uchar   *p_chr  = (uchar *)this->recv_data.data();
    ushort  *p_data = (ushort *)p_chr;
    ushort   chk    = SockCanFrame::calc_chksum_16(p_data, this->recv_data.size()/2);

    if ( 0 != chk ) {
        qDebug() << "data chksum is invalid. " << chk;

        return (tmp);
    }

    QString str = this->recv_data.toHex();

    for (int i = 0; i < (str.length() - 4); i += 2) {
        tmp += str.mid(i, 2) + " ";
    }

    QString ret_str("");

    if ( tmp.mid(0, 5) == QString("3c 11")) {
        ret_str = QObject::tr("上位机请求下位机发送工程参数 ");
    } else if ( tmp.mid(0, 5) == QString("3c 33")) {
        ret_str = QObject::tr("上位机发送到下位机的控制指令 ");
    } else if ( tmp.mid(0, 5) == QString("3c 55")) {
        ret_str = QObject::tr("上位机请求下位机发送备份的关键数据 ");
    } else if ( tmp.mid(0, 5) == QString("1b 00")) {
        ret_str = QObject::tr("时间广播服务");
    }

    ret_str.append(tmp.mid(6).trimmed().toUpper());

    return (ret_str);
}

QByteArray SockCanFrame::get_data_orig() const
{
    return (this->recv_data);
}

QString SockCanFrame::get_len() const
{
    return (QString("0x") + QString("%1").arg(this->recv_len - 8, 0, 16).toUpper());
}

uint SockCanFrame::get_len_value(void) const
{
    return (this->recv_len);
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

uint SockCanFrame::get_type_orig() const
{
    return (this->recv_type);
}

QString SockCanFrame::get_frame_cnt() const
{
    return (QString("%1").arg(this->frame_cnt));
}

uint SockCanFrame::get_frame_cnt_orig() const
{
    return (this->frame_cnt);
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

SockCanFrame& SockCanFrame::operator =(const SockCanFrame *obj)
{
    if ( this == obj) {
        return (*this);
    }

    this->recv_time     = obj->get_time_orig();
    this->recv_protocol = obj->get_protocol();
    this->recv_service  = obj->get_service();
    this->recv_src      = obj->get_src_orig();
    this->recv_dest     = obj->get_dest_orig();
    this->recv_len      = obj->get_len_value();
    this->recv_data     = obj->get_data_orig();
    this->frame_cnt     = obj->get_frame_cnt_orig();
    this->recv_oid      = obj->get_oid();

    return (*this);
}

bool SockCanFrame::operator ==(const SockCanFrame &obj)
{
    if ( (this->recv_time == obj.get_time_orig()) &&
         (this->recv_protocol == obj.get_protocol()) &&
         (this->recv_service  == obj.get_service()) &&
         (this->recv_src == obj.get_src_orig()) &&
         (this->recv_dest == obj.get_dest_orig()) &&
         (this->recv_len == obj.get_len_value()) &&
         (this->recv_data == obj.get_data_orig()) &&
         (this->recv_oid == obj.get_oid()) &&
         (this->frame_cnt == obj.frame_cnt)) {
        return (true);
    } else {
        return (false);
    }
}
