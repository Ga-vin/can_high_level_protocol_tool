#ifndef MAINGUI_H
#define MAINGUI_H

#include <QWidget>
#include "ui_maingui.h"
#include "sockcanframe.h"
#include "rxtask.h"
#include "errrxtask.h"

#include <QtNetwork/QUdpSocket>

class QTimer;

class MainGui : public QWidget
{
    Q_OBJECT
public:
    explicit MainGui(QWidget *parent = 0);
    ~MainGui();

    bool is_need_ack(void);
    
signals:
    void notify_change_stop_flag(void);
    void notify_err_stop_flag(void);
    void notify_ack_data(int index);
    void notify_ack_data(int index, uchar src, uchar dest);
    
public slots:
    void do_update_current_time(void);
    void do_rx_msg(bool flag);
    void do_update_rx_msg(const QByteArray &byte);

    void on_p_btn_id_convert_clicked(void);
    void on_p_btn_head_convert_clicked(void);
    void on_p_btn_oid_convert_clicked(void);

    void on_p_btn_id_clear_clicked(void);
    void on_p_btn_head_clear_clicked(void);
    void on_p_btn_oid_clear_clicked(void);

    void on_p_btn_id_direction_toggled(bool);
    void on_p_btn_head_direction_toggled(bool);
    void on_p_btn_oid_direction_toggled(bool);

    void on_p_btn_clear_rx_clicked(void);

    void do_update_err_rx_msg(const QByteArray &byte);
    void on_p_btn_start_recv_err_toggled(bool);
    void on_p_btn_clear_recv_err_clicked(void);

    void do_send_back_ack_data(int index, uchar src, uchar dest);

    void on_p_btn_start_tx_toggled(bool);
    
private:
    Ui::MainGUI *ui;
    QTimer      *timer_500ms;

    enum {TIMER_500MS = 500,
          PRJ_ACK     = 0x1,
          BAK_ACK     = 0x2,
          TM_ACK      = 0x3,
          MEM_ACK     = 0x4,
          BUS_MAN     = 0x5};
    SockCanFrame *psock_can_frame;
    bool          is_start;
    bool          is_finish;
    QTime         start_time;
    uint          byte_cnt;

private:
    void init_widget(void);
    void init_object(void);
    void init_timer(void);
    void make_connections(void);
    void change_btn_color(bool flag);
    void data_handle(const QByteArray &byte);
    void update_data_tables(void);
    void send_ack_data(const QString &host_ip, ushort host_port, const QByteArray &byte, arbit_header_t arbit, can_msg_header_t header);

private:
    RxTask    *p_task;
    ErrRxTask *p_err_task;

    QUdpSocket *ack_data_sendp;

    uint        prj_ack_tx_cnt;
    uint        bak_ack_tx_cnt;
    uint        tm_ack_tx_cnt;
    uint        mem_ack_tx_cnt;
    uint        bus_man_tx_cnt;
};

#endif // MAINGUI_H
