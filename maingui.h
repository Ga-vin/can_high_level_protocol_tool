#ifndef MAINGUI_H
#define MAINGUI_H

#include <QWidget>
#include "ui_maingui.h"
#include "sockcanframe.h"
#include "rxtask.h"

class QTimer;

class MainGui : public QWidget
{
    Q_OBJECT
public:
    explicit MainGui(QWidget *parent = 0);
    ~MainGui();
    
signals:
    void notify_change_stop_flag(void);
    
public slots:
    void do_update_current_time(void);
    void do_rx_msg(bool flag);
//    void do_update_rx_msg(const SockCanFrame *frame);
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
    
private:
    Ui::MainGUI *ui;
    QTimer      *timer_500ms;

    enum {TIMER_500MS = 500};
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

private:
    RxTask *p_task;
};

#endif // MAINGUI_H
