#include <QtCore>
#include <QByteArray>
#include <QDebug>
#include "sockcandata.h"

SockCanData::SockCanData(QObject *parent) :
    QObject(parent)
{
    this->id      = 0;
    this->channel = 0;
    this->ext     = true;
    this->rtr     = false;
    this->len     = 0;
    data = new int8_t[SockCanData::MAX_DATA_LEN];
    if ( !data) {
        return ;
    }
    memset(this->data, 0, sizeof(int8_t) * SockCanData::MAX_DATA_LEN);
    memset(&arbit,     0, sizeof(arbit_header_t));
}

SockCanData::SockCanData(const QByteArray &byte, QObject *parent) :
    QObject(parent)
{
    data = new int8_t[SockCanData::MAX_DATA_LEN];
    if ( !data) {
        return ;
    }

    if ( byte.isEmpty()) {
        this->id      = 0;
        this->channel = 0;
        this->ext     = true;
        this->rtr     = false;
        this->len     = 0;
        memset(this->data, 0, sizeof(int8_t) * SockCanData::MAX_DATA_LEN);
        memset(&arbit,     0, sizeof(arbit_header_t));
    } else {
        uchar *p_data = (uchar *)(byte.data());

        this->id      = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->channel = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->ext     = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->rtr     = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->len     = *p_data++;
        memcpy(this->data, p_data, sizeof(int8_t) * (this->len));

        uint new_id = SockCanData::little_ending_to_big(this->id);
        memcpy(&arbit, &new_id, sizeof(arbit_header_t));

#ifdef __DEBUG_
        qDebug() << "ID: " << debug_id();
        qDebug() << "CHANNEL: " << debug_channel();
        qDebug() << "EXT: " << debug_ext();
        qDebug() << "RTR: " << debug_rtr();
        qDebug() << "LEN: " << debug_len();
        qDebug() << "DATA: " << debug_data();

        qDebug() << "protocol = " << arbit.identify_code;
        qDebug() << "ctrl = " << arbit.ctrl_code;
        qDebug() << "src = " << arbit.user_code.defined.src_nid;
        qDebug() << "dest = " << arbit.user_code.defined.dest_nid;
        qDebug() << "frame cnt = " << arbit.user_code.defined.frame_cnt;
#endif
    }
}

SockCanData::SockCanData(const SockCanData &can)
{
    qDebug() << "copy";
    this->id      = can.can_id();
    this->channel = can.can_channel();

    if ( can.can_ext()) {
        this->ext = 1;
    } else {
        this->ext = 0;
    }

    if ( can.can_rtr()) {
        this->rtr = 1;
    } else {
        this->rtr = 0;
    }

    this->len     = can.can_len();
    uchar *p_data = (uchar *)(can.can_data());

    memset(this->data, 0, sizeof(int8_t) * (SockCanData::MAX_DATA_LEN));
    memcpy(this->data, p_data, sizeof(int8_t) * this->len);

    uint new_id = SockCanData::little_ending_to_big(this->id);
    memcpy(&arbit, &new_id, sizeof(arbit_header_t));
}

SockCanData::~SockCanData()
{
    delete data;
}

void SockCanData::clear()
{
    this->id      = 0;
    this->channel = 0;
    this->ext     = true;
    this->rtr     = false;
    this->len     = 0;
    memset(this->data, 0, sizeof(int8_t) * SockCanData::MAX_DATA_LEN);
    memset(&arbit,     0, sizeof(arbit_header_t));
}

uint SockCanData::can_id(void) const
{
    return (this->id);
}

uint SockCanData::can_channel(void) const
{
    return (this->channel);
}

bool SockCanData::can_ext(void) const
{
    return (this->ext == 1 ? true : false);
}

bool SockCanData::can_rtr(void) const
{
    return (this->rtr == 1 ? true : false);
}

uint8_t SockCanData::can_len(void) const
{
    return (this->len);
}

const int8_t *SockCanData::can_data(void) const
{
    return (this->data);
}

uchar SockCanData::src() const
{
    return (this->arbit.user_code.defined.src_nid);
}

uchar SockCanData::dest() const
{
    return (this->arbit.user_code.defined.dest_nid);
}

uint SockCanData::ctrl() const
{
    return (static_cast<uint>(this->arbit.ctrl_code));
}

uint SockCanData::oid_upper()
{
    return (this->arbit.user_code.dev_oid_hi);
}

void SockCanData::to_can_frame(const QByteArray &byte)
{
    if ( byte.isEmpty()) {
        this->id      = 0;
        this->channel = 0;
        this->ext     = true;
        this->rtr     = false;
        this->len     = 0;
        memset(this->data, 0, sizeof(int8_t) * SockCanData::MAX_DATA_LEN);
        memset(&arbit,     0, sizeof(arbit_header_t));
    } else {
        uchar *p_data = (uchar *)(byte.data());

        this->id      = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->channel = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->ext     = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->rtr     = (*p_data<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3)<<0);
        p_data       += 4;
        this->len     = *p_data++;
        memcpy(this->data, p_data, sizeof(char) * this->len);

        uint new_id = SockCanData::little_ending_to_big(this->id);
        memcpy(&arbit,     &new_id, sizeof(arbit_header_t));
    }
}

bool SockCanData::is_device_identify_pro(void) const
{
    return ((this->arbit.identify_code == SockCanData::DEV_IDENTIFY) ? true : false);
}

bool SockCanData::is_data_trans_pro(void) const
{
    return ((this->arbit.identify_code == SockCanData::DATA_TRANSMISSION) ? true : false);
}

uint8_t SockCanData::curr_frame_cnt(void) const
{
    return (this->arbit.user_code.defined.frame_cnt);
}

bool SockCanData::is_first_frame(void) const
{
    return ((this->curr_frame_cnt() == SockCanData::START_SEQ) ? true : false);
}

bool SockCanData::is_last_frame(void) const
{
    return ((this->curr_frame_cnt() == SockCanData::END_SEQ) ? true : false);
}

bool SockCanData::is_ind_frame() const
{
    return ((this->curr_frame_cnt() == SockCanData::IND_SEQ) ? true : false);
}

bool SockCanData::is_complete() const
{
    if ( this->is_ind_frame()) {
        return (true);
    }

    if ( this->is_last_frame()) {
        return (true);
    } else {
        return (false);
    }
}

bool SockCanData::is_dev_identify_broad() const
{
    if ( this->arbit.ctrl_code == 0x3) {
        return (true);
    } else {
        return (false);
    }
}

bool SockCanData::is_dev_identify_request() const
{
    if ( this->arbit.ctrl_code == 0x1) {
        return (true);
    } else {
        return (false);
    }
}

bool SockCanData::is_dev_identify_allow() const
{
    if ( this->arbit.ctrl_code == 0x2) {
        return (true);
    } else {
        return (false);
    }
}

bool SockCanData::operator ==(const SockCanData &can)
{
    if ( (this->id == can.can_id()) &&
         (this->channel == can.can_channel()) &&
         (this->len == can.can_len()) &&
         ((this->ext == 1) && can.can_ext()) &&
         ((this->rtr == 1) && can.can_rtr())) {
        const int8_t *p_data = can.can_data();
        if ( !is_equal(this->data, p_data, this->len)) {
            return (false);
        } else {
            return (true);
        }
    } else {
        return (false);
    }
}

SockCanData &SockCanData::operator =(const SockCanData *p_can)
{
    if ( this == p_can) {
        return (*this);
    }

    if ( this->data) {
        delete [] data;
        data = new int8_t[SockCanData::MAX_DATA_LEN];
    }
    id      = p_can->can_id();
    channel = p_can->can_channel();

    if ( p_can->can_ext()) {
        this->ext = 1;
    } else {
        this->ext = 0;
    }

    if ( p_can->can_rtr()) {
        this->rtr = 1;
    } else {
        this->rtr = 0;
    }
    this->len            = p_can->can_len();
    const int8_t *p_data = p_can->can_data();
    memcpy(this->data, p_data, sizeof(int8_t) * this->len);

    uint new_id = SockCanData::little_ending_to_big(this->id);
    memcpy(&arbit, &new_id, sizeof(arbit_header_t));

    return (*this);
}

QString SockCanData::debug_id()
{
    return (QString("%1").arg(this->id));
}

QString SockCanData::debug_channel()
{
    return (QString("%1").arg(this->channel));
}

QString SockCanData::debug_ext()
{
    return (QString("%1").arg(this->ext));
}

QString SockCanData::debug_rtr()
{
    return (QString("%1").arg(this->rtr));
}

QString SockCanData::debug_len()
{
    return (QString("%1").arg(this->len));
}

QString SockCanData::debug_data()
{
    const int8_t *p_data = this->data;
    QByteArray byte;
    for (int i = 0; i < this->len; ++i) {
        byte.append(*(p_data + i));
    }
    return (SockCanData::bytearray_to_hex_str(byte));
}

bool SockCanData::is_equal(const int8_t *left, const int8_t *right, uint len)
{
    for (uint i = 0; i < len; ++i) {
        if ( *(left + i) != *(right + i)) {
            return (false);
        }
    }

    return (true);
}
