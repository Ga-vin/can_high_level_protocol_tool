#ifndef SOCKCANFRAME_H
#define SOCKCANFRAME_H

#include <QObject>
#include <QTime>

#include "sockcandata.h"

#pragma pack(1)
typedef struct _can_msg_header {
    uint8_t       version  : 3;                         /* 版本号：当前为001B                            */
    uint16_t      len      : 11;                        /* 包长度：长度包含首部加数据                      */
    uint8_t       reserve1 : 2;                         /* 保留  ：全写1B                               */
    uint8_t       tran     : 4;                         /* 传输协议：                                   */
    uint8_t       ctrl     : 4;                         /* 控制域  ：全写1B                              */
    uint8_t       src         ;                         /* 源地址  ：发出节点NID                          */
    uint8_t       dest        ;                         /* 目的地址：接收节点NID                          */
    uint16_t      reserve2    ;                        /* 保留    ：全写1B                              */
    uint8_t       chk         ;                         /* 校验    ：首部所有字节之和低8位为0时通过校验       */
} can_msg_header_t, *pcan_msg_header_t;

class SockCanFrame : public QObject
{
    Q_OBJECT
public:
    explicit SockCanFrame(QObject *parent = 0);
    ~SockCanFrame();

    void    clear(void);
    void    set_time(const QTime &tt);
    void    set_protocol(const QString &pro);
    void    set_service(const QString &ser);
    void    set_src(uchar src);
    void    set_dest(uchar dest);
    void    set_data(const QByteArray &data);
    void    append_data(const QByteArray &data);
    void    append_oid(const QByteArray &data);
    void    inc_len(int len = 1);
    void    inc_frame_cnt(int cnt = 1);
    void    set_type(uint tt);

    QString get_time(void) const;
    QString get_protocol(void) const;
    QString get_service(void) const;
    QString get_src(void) const;
    QString get_dest(void) const;
    QString get_data(void) const;
    QString get_len(void) const;
    QString get_type(void) const;
    QString get_frame_cnt(void) const;
    QByteArray get_oid(void) const;

    bool    is_device_identify_pro(void) const;
    bool    is_data_tramsmission_pro(void) const;

public:
    enum DevCtrlCode {DEV_IDENTIFY_REQ = 0x1, DEV_IDENTIFY_ALO = 0x2, DEV_IDENTIFY_BROAD = 0x3};
    enum DataCtrlCode {TX_NEED_ACK = 0x0, TX_NO_ACK = 0x1, RX_ACK = 0x2};
    enum NetAddr {NET_ADDR_START = 0x01, NET_ADDR_END = 0xEF, NET_MASTER_ADDR = 0xF0, NET_BROADCAST_ADDR = 0xFF};
    enum MsgLen {MAX_MSG_LEN = 2047};

    static bool is_valid_identify_code(const arbit_header_t &arbit)
    {
        if ( SockCanData::DEV_IDENTIFY == arbit.identify_code || SockCanData::DATA_TRANSMISSION == arbit.identify_code) {
            return (true);
        } else {
            return (false);
        }
    }

    static bool is_valid_ctrl_code(const arbit_header_t &arbit)
    {
        if ( (SockCanData::DEV_IDENTIFY == arbit.identify_code) &&
             (SockCanFrame::DEV_IDENTIFY_BROAD == arbit.ctrl_code ||
              SockCanFrame::DEV_IDENTIFY_REQ == arbit.ctrl_code   ||
              SockCanFrame::DEV_IDENTIFY_ALO == arbit.ctrl_code)) {
            return (true);
        } else if ( (SockCanData::DATA_TRANSMISSION == arbit.identify_code) &&
                    (SockCanFrame::TX_NEED_ACK == arbit.ctrl_code ||
                     SockCanFrame::TX_NO_ACK == arbit.ctrl_code ||
                     SockCanFrame::RX_ACK == arbit.ctrl_code)) {
            return (true);
        } else {
            return (false);
        }
    }

    static bool is_valid_net_addr(uchar addr)
    {
        if ( addr >= SockCanFrame::NET_ADDR_START && addr <= SockCanFrame::NET_ADDR_END) {
            return (true);
        } else if ( (SockCanFrame::NET_BROADCAST_ADDR == addr) || (SockCanFrame::NET_MASTER_ADDR == addr)) {
            return (true);
        } else {
            return (false);
        }
    }

    static bool is_valid_service(uchar serv)
    {
        switch ( serv) {
        case SockCanData::INFO_TRANS:
        case SockCanData::TIME_TRANS:
        case SockCanData::ON_BOARD:
        case SockCanData::FILE_TRANS:
        case SockCanData::TC_TRANS:
        case SockCanData::TM_TRANS:
        case SockCanData::DEV_DRIVER:
        case SockCanData::MEM_DOWN:
        case SockCanData::BUS_MANAGE:
            return (true);
        default:
            return (false);
        }
    }

    static bool is_valid_msg_len(uint len)
    {
        if ( len > SockCanFrame::MAX_MSG_LEN) {
            return (false);
        } else {
            return (true);
        }
    }

    static uchar calc_chksum(uchar *p_data, uint len)
    {
        uchar chk = 0;

        for (size_t i = 0; i < len; ++i) {
            chk ^= *(p_data + i);
        }

        return (chk);
    }

signals:
    
public slots:
    
private:
    QTime      recv_time;
    QString    recv_protocol;
    QString    recv_service;
    uchar      recv_src;
    uchar      recv_dest;
    uint32_t   recv_len;
    uint       recv_type;
    QByteArray recv_data;
    uint       frame_cnt;
    QByteArray recv_oid;
};

#endif // SOCKCANFRAME_H
