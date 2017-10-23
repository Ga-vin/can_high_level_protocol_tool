#ifndef SOCKCANDATA_H
#define SOCKCANDATA_H

#include <QObject>
#include <QDebug>

#pragma pack(1)
typedef struct _arbit_header {
    uint8_t       reserve       : 3;
    uint8_t       identify_code : 3;
    uint8_t       ctrl_code     : 2;

    union {
        uint32_t dev_oid_hi  : 24;
        struct {
            uint8_t dest_nid;
            uint8_t src_nid;
            uint8_t frame_cnt;
        } defined;
    } user_code;
} arbit_header_t, *parbit_header_t;

#pragma pack(1)
typedef struct _can_dev_oid_ {
    uint8_t  reserved;                                          /* 保留                                */
    uint16_t productID     : 12;                                /* 生产方编号                           */
    uint8_t  device_typeID : 4;                                 /* 设备类型号                           */
    uint32_t factoryID     : 24;                                /* 出厂序号                             */
} can_dev_oid_t;

class SockCanData : public QObject
{
    Q_OBJECT
public:
    enum protocol_type_e {
        DEV_IDENTIFY = 0x0,
        DATA_TRANSMISSION = 0x6};
    enum service_type_e {
    INFO_TRANS = 0x1,
    TIME_TRANS = 0x2,
    ON_BOARD   = 0x3,
    FILE_TRANS = 0x4,
    DEV_DRIVER = 0x5,
    TM_TRANS   = 0x6,
    TC_TRANS   = 0x7,
    MEM_DOWN   = 0x8,
    BUS_MANAGE = 0x9};

    explicit SockCanData(QObject *parent = 0);
    explicit SockCanData(const QByteArray &byte, QObject *parent = 0);
    explicit SockCanData(const SockCanData &can_data);
    ~SockCanData();

    void clear(void);

    uint     can_id(void) const;
    uint     can_channel(void) const;
    bool     can_ext(void) const;
    bool     can_rtr(void) const;
    uint8_t  can_len(void) const;
    const int8_t *can_data(void) const;

    uchar    src(void) const;
    uchar    dest(void) const;
    uint     ctrl(void) const;
    uint     oid_upper(void);

    void     to_can_frame(const QByteArray &byte);

    bool     is_device_identify_pro(void) const;
    bool     is_data_trans_pro(void) const;
    uint8_t  curr_frame_cnt(void) const;
    bool     is_first_frame(void) const;
    bool     is_last_frame(void) const;
    bool     is_ind_frame(void) const;
    bool     is_complete(void) const;

    bool     is_dev_identify_broad(void) const;
    bool     is_dev_identify_request(void) const;
    bool     is_dev_identify_allow(void) const;
    
    bool         operator==(const SockCanData &can_data);
    SockCanData &operator=(const SockCanData * can_data);

    static QString bytearray_to_hex_str(QByteArray data)
    {
        QString tmp("");
        QString str = data.toHex();
        for (int i = 0; i < str.length(); i += 2) {
            tmp += str.mid(i, 2) + " ";
        }

        return (tmp.trimmed().toUpper());
    }

    static uint little_ending_to_big(uint value)
    {
        uint new_value;
        uchar *p = (uchar *)&value;

        new_value = (*p<<24) | (*(p+1)<<16) | (*(p+2)<<8) | (*(p+3)<<0);
        return (new_value);
    }

    static arbit_header_t byte_to_arbit(const QByteArray &byte)
    {
        arbit_header_t arbit;
        uchar *p_data       = (uchar *)byte.data();

        arbit.reserve       = ((*p_data)&0xE0) >> 5;
        arbit.identify_code = ((*p_data)&0x1C) >> 2;
        arbit.ctrl_code     = (*p_data)&0x3;

        if ( SockCanData::DEV_IDENTIFY == arbit.identify_code) {
            arbit.user_code.dev_oid_hi = (*(p_data + 1) << 16) | (*(p_data + 2) << 8) | *(p_data + 3);
        } else if ( SockCanData::DATA_TRANSMISSION == arbit.identify_code) {
            arbit.user_code.defined.dest_nid  = *(p_data + 1);
            arbit.user_code.defined.src_nid   = *(p_data + 2);
            arbit.user_code.defined.frame_cnt = *(p_data + 3);
        } else {
            qDebug() << "UN";
            memset(&arbit, 0, sizeof(arbit_header_t));
        }

#ifdef __DEBUG_
        qDebug() << "H:protocol  = " << arbit.identify_code;
        qDebug() << "H:ctrl      = " << arbit.ctrl_code;
        qDebug() << "H:src       = " << arbit.user_code.defined.src_nid;
        qDebug() << "H:dest      = " << arbit.user_code.defined.dest_nid;
        qDebug() << "H:frame cnt = " << arbit.user_code.defined.frame_cnt;
#endif
        return (arbit);
    }

    static QByteArray arbit_to_byte(arbit_header_t arbit)
    {
        QByteArray byte;
        byte.resize(sizeof(arbit_header_t));

        uchar *p_data = (uchar *)byte.data();

        *p_data = (arbit.reserve<<5) | (arbit.identify_code<<2) | (arbit.ctrl_code);

        if ( SockCanData::DEV_IDENTIFY == arbit.identify_code)  {
            *p_data++ = (arbit.user_code.dev_oid_hi&0xFF0000) >> 16;
            *p_data++ = (arbit.user_code.dev_oid_hi&0x00FF00) >> 8;
            *p_data   = (arbit.user_code.dev_oid_hi&0x0000FF) >> 0;
        } else if ( SockCanData::DATA_TRANSMISSION == arbit.identify_code) {
            *p_data++ = arbit.user_code.defined.dest_nid;
            *p_data++ = arbit.user_code.defined.src_nid;
            *p_data   = arbit.user_code.defined.frame_cnt;
        } else {
            qDebug() << "UNN";
            byte.resize(0);
        }

        return (byte);
    }

    QString debug_id(void);
    QString debug_channel(void);
    QString debug_ext(void);
    QString debug_rtr(void);
    QString debug_len(void);
    QString debug_data(void);

signals:
    
public slots:

private:
    enum can_data_e{ MAX_DATA_LEN = 8};
    enum data_seq_e {
        START_SEQ = 0x1,
        END_SEQ   = 0xFF,
        IND_SEQ   = 0x0};
    uint           id;
    uint           channel;
    uint           ext;
    uint           rtr;
    uint8_t        len;
    int8_t        *data;
    arbit_header_t arbit;

private:
    bool is_equal(const int8_t *left, const int8_t *right, uint len);
};

#endif // SOCKCANDATA_H
