#include <QtGui>
#include <QtCore>
#include <QTextCodec>
#include <QDebug>
#include <QTableWidgetItem>
#include <QRegExpValidator>
#include "maingui.h"
#include "sockcandata.h"
#include "cerr.h"


MainGui::MainGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainGUI)
{
    ui->setupUi(this);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));

    this->init_widget();
    this->init_object();
    this->init_timer();
    this->make_connections();
}

MainGui::~MainGui()
{
    delete ui;
    p_task->terminate();

    delete p_task;
}

bool MainGui::is_need_ack()
{
    return (this->ui->p_btn_start_tx->isChecked());
}

void MainGui::do_update_current_time(void)
{
    this->ui->p_datetime_display->display(QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss.zzz"));
}

void MainGui::do_rx_msg(bool flag)
{
    quint16 port = this->ui->p_spin_rx_port->value();

    this->change_btn_color(flag);
    if ( flag) {
        p_task = new RxTask(port);
        QObject::connect(this, SIGNAL(notify_change_stop_flag()), p_task, SLOT(update_stop_flag()));
        QObject::connect(p_task, SIGNAL(notify_can_frame(QByteArray)), this, SLOT(do_update_rx_msg(QByteArray)));
        p_task->start();
    } else {
        emit notify_change_stop_flag();
        QObject::disconnect(this, SIGNAL(notify_change_stop_flag()), p_task, SLOT(update_stop_flag()));
        QObject::disconnect(this->p_task, SIGNAL(notify_can_frame(QByteArray)), this, SLOT(do_update_rx_msg(QByteArray)));
        p_task->quit();
        p_task->wait(10);

        delete p_task;
    }
}

void MainGui::do_update_rx_msg(const QByteArray &byte)
{
    this->data_handle(byte);
}

void MainGui::on_p_btn_id_convert_clicked()
{
    arbit_header_t arbit;
    uint  identify = this->ui->p_line_id_identify->text().toUInt(0, 16);
    uint  ctrl     = this->ui->p_line_id_ctrl->text().toUInt(0, 16);
    uint  oid;
    uchar src;
    uchar dest;
    uchar frame_cnt;
    bool  flag     = true;

    arbit.reserve = 0x0;

    if ( this->ui->p_btn_id_direction->isChecked()) {
        if ( this->ui->p_line_id_result->text().isEmpty()) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("ת���������Ϊ��"));
            flag = false;
        }

        QStringList result = this->ui->p_line_id_result->text().split(' ');
        QByteArray  byte;
        for (int i = 0; i < result.size(); ++i) {
            byte.append(result.at(i).toUInt(0, 16));
        }

        uchar *p_data       = (uchar *)(byte.data());
        arbit.ctrl_code     = (*p_data)&0x3;
        arbit.identify_code = ((*p_data)>>2) & 0x7;
        memcpy(&arbit.user_code, byte.data() + 1, sizeof(uchar) * 3);

        if ( SockCanData::DEV_IDENTIFY == arbit.identify_code) {
            if ( !SockCanFrame::is_valid_ctrl_code(arbit)) {
                QMessageBox::warning(0,
                                     QObject::tr("����"),
                                     QObject::tr("�豸ʶ��Э����ʶ������ֲ���ȷ��ֻ��Ϊ1��2��3"));
                flag = false;
            } else {
                uint value = arbit.user_code.dev_oid_hi;
                value = ((value&0xFF0000)>>16) | ((value&0x0000FF)<<16) | (value&0x00FF00);
                this->ui->p_line_id_oid->setText(QString("0x%1").arg(value, 0, 16));
            }
        } else if ( SockCanData::DATA_TRANSMISSION == arbit.identify_code) {
            if ( !SockCanFrame::is_valid_ctrl_code(arbit)) {
                QMessageBox::warning(0,
                                     QObject::tr("����"),
                                     QObject::tr("���ݴ���ʶ��Э����ʶ������ֲ���ȷ��ֻ��Ϊ0��1��2"));
                flag = false;
            } else {
                this->ui->p_line_id_dest->setText(QString("0x") + QString("%1").arg(arbit.user_code.defined.dest_nid, 0, 16).toUpper());
                this->ui->p_line_id_src->setText(QString("0x") + QString("%1").arg(arbit.user_code.defined.src_nid, 0, 16).toUpper());
                this->ui->p_line_id_frame_cnt->setText(QString("0x") + QString("%1").arg(arbit.user_code.defined.frame_cnt, 0, 16).toUpper());
            }
        } else {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("������ַ��������豸ʶ���벻��ȷ��bit28-26ֻ��Ϊ0����6"));
            flag = false;
        }

        this->ui->p_line_id_res1->setText(QString("0x") + QObject::tr("0").toUpper());
        this->ui->p_line_id_identify->setText(QString("0x") + QString("%1").arg(arbit.identify_code, 0, 16).toUpper());
        this->ui->p_line_id_ctrl->setText(QString("0x") + QString("%1").arg(arbit.ctrl_code, 0, 16).toUpper());
    } else {
        if ( SockCanData::DEV_IDENTIFY == identify) {
            arbit.identify_code = identify & 0x7;

            if ( this->ui->p_line_id_oid->text().isEmpty()) {
                QMessageBox::warning(0,
                                     QObject::tr("����"),
                                     QObject::tr("�豸Э����Ψһʶ�����24λ����Ϊ��"));
                flag = false;
            } else {
                oid = this->ui->p_line_id_oid->text().toUInt(0, 16);
                oid = ((oid&0xFF0000)>>16) | ((oid&0x0000FF)<<16) | (oid&0x00FF00);
            }

            if ( SockCanFrame::DEV_IDENTIFY_BROAD == ctrl ||
                 SockCanFrame::DEV_IDENTIFY_REQ == ctrl ||
                 SockCanFrame::DEV_IDENTIFY_ALO == ctrl) {
                arbit.ctrl_code            = ctrl;
                arbit.user_code.dev_oid_hi = oid;

                uchar p_data[4];
                memcpy(p_data, &arbit, sizeof(arbit_header_t));
                p_data[0] = ((p_data[0]&0xC0)>>6) | ((p_data[0]&0xE0)<<5) | ((p_data[0]&0x38)>>1);

                QByteArray byte((const char*)(p_data), 4);
                this->ui->p_line_id_result->setText(SockCanData::bytearray_to_hex_str(byte));

            } else {
                QMessageBox::warning(0,
                                     QObject::tr("����"),
                                     QObject::tr("֡���Ϳ�����ֻ��Ϊ1, 2, 3"));
                flag = false;
            }
        } else if ( SockCanData::DATA_TRANSMISSION == identify) {
            if ( this->ui->p_line_id_src->text().isEmpty() ||
                 this->ui->p_line_id_dest->text().isEmpty() ||
                 this->ui->p_line_id_frame_cnt->text().isEmpty()) {
                QMessageBox::warning(0,
                                     QObject::tr("����"),
                                     QObject::tr("���ݴ���Э����Դ��ַ��Ŀ�ĵ�ַ��֡��Ų���Ϊ��"));
                flag = false;
            } else {
                src       = this->ui->p_line_id_src->text().toUInt(0, 16);
                dest      = this->ui->p_line_id_dest->text().toUInt(0, 16);
                frame_cnt = this->ui->p_line_id_frame_cnt->text().toUInt(0, 16);
            }

            if ( SockCanFrame::TX_NO_ACK == ctrl ||
                 SockCanFrame::TX_NEED_ACK == ctrl ||
                 SockCanFrame::RX_ACK == ctrl) {
                arbit.identify_code               = identify & 0x7;
                arbit.ctrl_code                   = ctrl     & 0x3;
                arbit.user_code.defined.src_nid   = src;
                arbit.user_code.defined.dest_nid  = dest;
                arbit.user_code.defined.frame_cnt = frame_cnt;
            } else {
                QMessageBox::warning(0,
                                     QObject::tr("����"),
                                     QObject::tr("֡���Ϳ�����ֻ��Ϊ0, 1, 2, 3"));
                flag = false;
            }
        } else {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("Э��ʶ����ֻ��Ϊ0����6"));
            flag = false;
        }

        if ( flag) {
            uchar p_data[4];
            memcpy(p_data, &arbit, sizeof(arbit));
            p_data[0] = ((p_data[0]&0xC0)>>6) | ((p_data[0]&0xE0)<<5) | ((p_data[0]&0x38)>>1);  /* �����ֽ��ڵı������� */
            QByteArray display_result((const char*)(p_data), 4);
            this->ui->p_line_id_result->setText(SockCanData::bytearray_to_hex_str(display_result));
        } else {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("ת��ʧ��"));
        }
    }
}

void MainGui::on_p_btn_head_convert_clicked()
{
    can_msg_header_t header;

    if ( this->ui->p_btn_head_direction->isChecked()) {
        /* Byte array to structure */
        QByteArray byte;
        QString    str = this->ui->p_line_head_result->text();
        if ( str.isEmpty()) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("ת���������Ϊ��"));
            this->ui->p_line_head_result->setFocus();
        }

        QStringList str_list = str.split(" ");
        for (size_t i = 0; i < str_list.size(); ++i) {
            byte.append(str_list.at(i).toUInt(0, 16));
        }

        if ( SockCanFrame::calc_chksum((uchar *)byte.data(), sizeof(can_msg_header_t))) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("�ײ�У�������Ϸ�"));
        }

        uchar *p_data   = (uchar *)byte.data();

        header.version  = ((*p_data)&0xE0) >> 5;
        header.len      = (((*p_data)&0x1F)<<6) | ((*(p_data + 1)&0xFC) >> 2);
        header.reserve1 = *(p_data + 1)&0x3;
        header.tran     = (*(p_data + 2)&0xF0) >> 4;
        if ( !SockCanFrame::is_valid_service(header.tran)) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("������񲻺Ϸ�"));
        }

        header.ctrl     = *(p_data + 2)&0x0F;
        memcpy(&(header.src), p_data + 3, sizeof(uchar) * 5);

        if ( !SockCanFrame::is_valid_net_addr(header.src)) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("Դ��ַ���Ϸ�"));
        }

        if ( !SockCanFrame::is_valid_net_addr(header.dest)) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("Ŀ���ַ���Ϸ�"));
        }

        this->ui->p_line_head_ver->setText(QString("0x") + QString("%1").arg(header.version, 0, 16).toUpper());
        this->ui->p_line_head_len->setText(QString("0x") + QString("%1").arg(header.len, 0, 16).toUpper());
        this->ui->p_line_head_res1->setText(QString("0x") + QString("%1").arg(header.reserve1, 0, 16).toUpper());
        this->ui->p_line_head_trans->setText(QString("0x") + QString("%1").arg(header.tran, 0, 16).toUpper());
        this->ui->p_line_head_ctrl->setText(QString("0x") + QString("%1").arg(header.ctrl, 0, 16).toUpper());
        this->ui->p_line_head_src->setText(QString("0x") + QString("%1").arg(header.src, 0, 16).toUpper());
        this->ui->p_line_head_dest->setText(QString("0x") + QString("%1").arg(header.dest, 0, 16).toUpper());
        this->ui->p_line_head_res2->setText(QString("0x") + QString("%1").arg(header.reserve2, 0, 16).toUpper());
        this->ui->p_line_head_chksum->setText(QString("0x") + QString("%1").arg(header.chk, 0, 16).toUpper());
    } else {
        /* Structure to byte array */
        uint dest = this->ui->p_line_head_dest->text().toUInt(0, 16);
        uint src  = this->ui->p_line_head_src->text().toUInt(0, 16);
        uint serv = this->ui->p_line_head_trans->text().toUInt(0, 16);
        uint len  = this->ui->p_line_head_len->text().toUInt(0, 16);

        if ( !SockCanFrame::is_valid_net_addr(dest)) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("Ŀ���ַ���Ϸ�"));
        }

        if ( !SockCanFrame::is_valid_net_addr(src)) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("Դ��ַ���Ϸ�"));
        }

        if ( !SockCanFrame::is_valid_service(serv)) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("����������Ͳ��Ϸ�"));
        }

        if ( !SockCanFrame::is_valid_msg_len(len)) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("���ݳ��Ȳ��Ϸ������ô���2047"));
        }

        header.version  = this->ui->p_line_head_ver->text().toUInt(0, 16);
        header.len      = len;
        header.reserve1 = this->ui->p_line_head_res1->text().toUInt(0, 16);
        header.tran     = serv;
        header.ctrl     = this->ui->p_line_head_ctrl->text().toUInt(0, 16);
        header.src      = src;
        header.dest     = dest;
        header.reserve2 = this->ui->p_line_head_res2->text().toUInt(0, 16);

        QByteArray byte;
        byte.resize(sizeof(can_msg_header_t));;

        uchar *p_data = (uchar *)byte.data();
        *p_data++     = ((header.version&0x7) << 5) | ((header.len&0x7C0) >> 6);
        *p_data++     = ((header.len&0x3F)<<2) | header.reserve1&0x3;
        *p_data++     = ((header.tran&0xF)<<4) | (header.ctrl&0xF);
        memcpy(p_data, &(header.src), sizeof(can_msg_header_t) - 4);

        header.chk      = SockCanFrame::calc_chksum((uchar *)byte.data(), 7);
        byte[7]         = header.chk;
        this->ui->p_line_head_chksum->setText(QString("0x") + QString("%1").arg(header.chk, 0, 16).toUpper());

        this->ui->p_line_head_result->setText(SockCanData::bytearray_to_hex_str(byte));
    }
}

void MainGui::on_p_btn_oid_convert_clicked()
{
    can_dev_oid_t oid;

    if ( this->ui->p_btn_oid_direction->isChecked()) {
        /* Byte array to structure */
        QStringList str_list = this->ui->p_line_oid_result->text().split(" ");
        if ( !str_list.size()) {
            QMessageBox::warning(0,
                                 QObject::tr("����"),
                                 QObject::tr("ת���������Ϊ��"));
        }

        QByteArray  byte;
        for (size_t i = 0; i < str_list.size(); ++i) {
            byte.append(str_list.at(i).toInt(0, 16));
        }

        uchar *p_data = (uchar *)byte.data();

        oid.reserved      = *p_data++;
        oid.productID     = (*p_data<<4) | ((*(p_data + 1)&0xF0)>>4);
        oid.device_typeID = *(p_data + 1)&0x0F;
        oid.factoryID     = (*(p_data + 2)<<16) | (*(p_data + 3)<<8) | *(p_data + 4);

        this->ui->p_line_oid_res1->setText(QString("0x") + QString("%1").arg(oid.reserved, 0, 16).toUpper());
        this->ui->p_line_oid_product->setText(QString("0x") + QString("%1").arg(oid.productID, 0, 16).toUpper());
        this->ui->p_line_oid_device->setText(QString("0x") + QString("%1").arg(oid.device_typeID, 0, 16).toUpper());
        this->ui->p_line_oid_factory->setText(QString("0x") + QString("%1").arg(oid.factoryID, 0, 16).toUpper());
    } else {
        /* Structure to byte array */
        oid.reserved      = this->ui->p_line_oid_res1->text().toUInt(0, 16);
        oid.productID     = this->ui->p_line_oid_product->text().toUInt(0, 16);
        oid.device_typeID = this->ui->p_line_oid_device->text().toUInt(0, 16);
        oid.factoryID     = this->ui->p_line_oid_factory->text().toUInt(0, 16);

        QByteArray byte;
        byte.resize(sizeof(can_dev_oid_t));

        uchar *p_data = (uchar *)byte.data();

        *p_data++   = oid.reserved;
        *p_data++   = (oid.productID&0xFF0) >> 4;
        *p_data++   = ((oid.productID&0x00F)<<4) | (oid.device_typeID&0xF);
        *p_data++   = (oid.factoryID&0xFF0000)>>16;
        *p_data++   = (oid.factoryID&0x00FF00)>>8;
        *p_data     = (oid.factoryID&0x0000FF);

        this->ui->p_line_oid_result->setText(SockCanData::bytearray_to_hex_str(byte));
    }
}

void MainGui::on_p_btn_id_clear_clicked()
{
    this->ui->p_line_id_res1->clear();
    this->ui->p_line_id_identify->clear();
    this->ui->p_line_id_ctrl->clear();
    this->ui->p_line_id_oid->clear();
    this->ui->p_line_id_src->clear();
    this->ui->p_line_id_dest->clear();
    this->ui->p_line_id_frame_cnt->clear();
    this->ui->p_line_id_result->clear();
}

void MainGui::on_p_btn_head_clear_clicked()
{
    this->ui->p_line_head_ver->clear();
    this->ui->p_line_head_len->clear();
    this->ui->p_line_head_res1->clear();
    this->ui->p_line_head_trans->clear();
    this->ui->p_line_head_ctrl->clear();
    this->ui->p_line_head_src->clear();
    this->ui->p_line_head_dest->clear();
    this->ui->p_line_head_res2->clear();
    this->ui->p_line_head_chksum->clear();
    this->ui->p_line_head_result->clear();
}

void MainGui::on_p_btn_oid_clear_clicked()
{
    this->ui->p_line_oid_res1->clear();
    this->ui->p_line_oid_factory->clear();
    this->ui->p_line_oid_product->clear();
    this->ui->p_line_oid_device->clear();
    this->ui->p_line_oid_result->clear();
}

void MainGui::on_p_btn_id_direction_toggled(bool flag)
{
    if ( flag) {
        this->ui->p_btn_id_direction->setStyleSheet("background-image: url(:/images/up_blue_empty.png);");
    } else {
        this->ui->p_btn_id_direction->setStyleSheet("background-image: url(:/images/down_blue_empty.png);");
    }
}

void MainGui::on_p_btn_head_direction_toggled(bool flag)
{
    if ( flag) {
        this->ui->p_btn_head_direction->setStyleSheet("background-image: url(:/images/up_blue_empty.png);");
    } else {
        this->ui->p_btn_head_direction->setStyleSheet("background-image: url(:/images/down_blue_empty.png);");
    }
}

void MainGui::on_p_btn_oid_direction_toggled(bool flag)
{
    if ( flag) {
        this->ui->p_btn_oid_direction->setStyleSheet("background-image: url(:/images/up_blue_empty.png);");
    } else {
        this->ui->p_btn_oid_direction->setStyleSheet("background-image: url(:/images/down_blue_empty.png);");
    }
}

void MainGui::on_p_btn_clear_rx_clicked()
{
    this->ui->p_tab_rx->setRowCount(0);
    this->ui->p_tab_rx_dev->setRowCount(0);
}

void MainGui::do_update_err_rx_msg(const QByteArray &byte)
{
    if ( byte.size() <= 0) {
        qDebug() << "error rx msg invalid. ";

        return ;
    }

    CErr cerr_msg(byte);

    int row_cnt = this->ui->p_tab_rx_err->rowCount();
    this->ui->p_tab_rx_err->insertRow(row_cnt);

    /* ������ | ������ | �ļ��� | ������ | ���� | �������� */
    this->ui->p_tab_rx_err->setItem(row_cnt, 0, new QTableWidgetItem(QString("%1").arg(cerr_msg.get_code_str())));
    this->ui->p_tab_rx_err->setItem(row_cnt, 1, new QTableWidgetItem(QString("%1").arg(cerr_msg.get_type_str())));
    this->ui->p_tab_rx_err->setItem(row_cnt, 2, new QTableWidgetItem(QString("%1").arg(cerr_msg.get_file_name())));
    this->ui->p_tab_rx_err->setItem(row_cnt, 3, new QTableWidgetItem(QString("%1").arg(cerr_msg.get_func_name())));
    this->ui->p_tab_rx_err->setItem(row_cnt, 4, new QTableWidgetItem(QString("%1").arg(cerr_msg.get_line_str())));
    this->ui->p_tab_rx_err->setItem(row_cnt, 5, new QTableWidgetItem(QString("%1").arg(cerr_msg.get_err_desp())));
}

void MainGui::init_widget()
{
    /* Title */
    this->setWindowTitle(QObject::tr("CAN�߲�Э����֤��� -- by Gavin.Bai"));

    /* Icon */
    this->setWindowIcon(QIcon(":/images/main.ico"));

    /* LCD */
    this->ui->p_datetime_display->setNumDigits(23);
    this->ui->p_datetime_display->display(QDateTime::currentDateTime().toString("yyyy:MM:dd hh:mm:ss.zzz"));

    /* Table */
    QFont font = this->ui->p_tab_rx->horizontalHeader()->font();
    font.setBold(true);
    this->ui->p_tab_rx->horizontalHeader()->setFont(font);
    this->ui->p_tab_rx_dev->horizontalHeader()->setFont(font);
    this->ui->p_tab_rx->horizontalHeader()->setStretchLastSection(true);
    this->ui->p_tab_rx->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->p_tab_rx_dev->setEditTriggers(QAbstractItemView::NoEditTriggers);
    this->ui->p_tab_rx_err->horizontalHeader()->setFont(font);
    this->ui->p_tab_rx_err->horizontalHeader()->setStretchLastSection(true);

    /* ƽ�����ÿ�� */
    this->ui->p_tab_rx_dev->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    /* Port range */
    this->ui->p_spin_rx_port->setRange(1000, 999999);
    this->ui->p_spin_rx_port->setValue(55009);

    /* Add prefix */
    QRegExp exp("[0]{1}|[6]{1}");
    this->ui->p_line_id_identify->setValidator(new QRegExpValidator(exp, this));
    this->ui->p_line_id_identify->setWhatsThis(QObject::tr("ֻ������0����6"));

    this->ui->p_line_id_ctrl->setValidator(new QRegExpValidator(QRegExp("[0-3]{1}"), this));
    this->ui->p_line_id_ctrl->setWhatsThis(QObject::tr("ֻ������0��1��2��3"));

    this->ui->p_line_head_chksum->setReadOnly(true);

    QRegExp id_exp("([0-9|A-F]{2}\\s){3}[0-9|A-F]{2}");
    this->ui->p_line_id_result->setValidator(new QRegExpValidator(id_exp, this));

    id_exp.setPattern("([0-9|A-F]{2}\\s){7}[0-9|A-F]{2}");
    this->ui->p_line_head_result->setValidator(new QRegExpValidator(id_exp, this));

    id_exp.setPattern("([0-9|A-F]{2}\\s){5}[0-9|A-F]{2}");
    this->ui->p_line_oid_result->setValidator(new QRegExpValidator(id_exp, this));

    this->ui->p_spin_recv_err_port->setValue(55000);
    this->ui->p_spin_tx_port->setRange(1000, 99999);
    this->ui->p_spin_tx_port->setValue(55008);

    this->ui->p_line_tx_ip->setText("192.168.7.34");
}

void MainGui::init_object()
{
    this->psock_can_frame = new SockCanFrame;
    this->is_start        = false;
    this->is_finish       = false;

    this->ack_data_sendp  = new QUdpSocket;

    this->prj_ack_tx_cnt  = 0;
    this->bak_ack_tx_cnt  = 0;
    this->tm_ack_tx_cnt   = 0;
    this->mem_ack_tx_cnt  = 0;
    this->bus_man_tx_cnt  = 0;
}

void MainGui::init_timer()
{
    timer_500ms = new QTimer;

    QObject::connect(this->timer_500ms, SIGNAL(timeout()), this, SLOT(do_update_current_time()));
    this->timer_500ms->setInterval(MainGui::TIMER_500MS);
    this->timer_500ms->start();
}

void MainGui::make_connections()
{
    QObject::connect(this->ui->p_btn_start_rx, SIGNAL(toggled(bool)), this, SLOT(do_rx_msg(bool)));
    QObject::connect(this, SIGNAL(notify_ack_data(int)), this, SLOT(do_send_back_ack_data(int)));
}

void MainGui::change_btn_color(bool flag)
{
    if ( flag) {
        this->ui->p_btn_start_rx->setStyleSheet("background-color:green");
    } else {
        this->ui->p_btn_start_rx->setStyleSheet("background-color:red");
    }
}

void MainGui::data_handle(const QByteArray &byte)
{
    if ( byte.size() != RxTask::DEFAULT_DATA_SIZE) {
        qDebug() << "data received size is not correct. It is " << byte.size() << " now";
        return ;
    }

    SockCanData sock_can_data(byte);
    char *p_new_tmp = (char *)(sock_can_data.can_data());

    if ( sock_can_data.is_device_identify_pro()) {
        qDebug() << "device identify frame";
        /* �豸ʶ��֡ */
        this->psock_can_frame->clear();
        this->is_start  = true;
        this->is_finish = true;
        this->psock_can_frame->set_protocol(QString("%1").arg(QObject::tr("�豸ʶ��֡")));
        this->psock_can_frame->set_service(QString("null"));
        this->psock_can_frame->set_time(QTime::currentTime());
        this->psock_can_frame->set_type(sock_can_data.ctrl());
        this->psock_can_frame->set_src(0xF0);

        if ( sock_can_data.is_dev_identify_broad()) {
            qDebug() << "broad";
            this->psock_can_frame->set_dest(0xFF);
            uchar *p_data  = (uchar *)sock_can_data.can_data();
            uint   tmp_oid = sock_can_data.oid_upper();

            QByteArray tmp_array;
            tmp_array.append((uchar)((tmp_oid&0xFF0000) >> 16));
            tmp_array.append((uchar)((tmp_oid&0x00FF00) >> 8));
            tmp_array.append((uchar)((tmp_oid&0x0000FF) >> 0));
            tmp_array.append(*p_data);
            tmp_array.append(*(p_data + 1));
            tmp_array.append(*(p_data + 2));
            this->psock_can_frame->append_oid(tmp_array);
        } else if ( sock_can_data.is_dev_identify_allow()){
            qDebug() << "allow";
            this->psock_can_frame->set_dest(*(p_new_tmp + 3));

            uint oid      = sock_can_data.oid_upper();
            uchar *p_data = (uchar *)sock_can_data.can_data();

            QByteArray tmp_array;
            tmp_array.append((uchar)((oid&0xFF0000) >> 16));
            tmp_array.append((uchar)((oid&0x00FF00) >>  8));
            tmp_array.append((uchar)((oid&0x0000FF) >>  0));
            tmp_array.append(*p_data);
            tmp_array.append(*(p_data + 1));
            tmp_array.append(*(p_data + 2));
            this->psock_can_frame->append_oid(tmp_array);
        } else {
            qDebug() << "unknown";
            this->is_start  = false;
            this->is_finish = false;
        }
    } else if ( sock_can_data.is_data_trans_pro()) {
        /* ���ݴ���֡ */
        if ( sock_can_data.is_ind_frame() || sock_can_data.is_first_frame()) {
            if ( SockCanFrame::calc_chksum((uchar *)p_new_tmp, 8)) {
                qDebug() << "first frame chksum invalid";
                return ;
            }

            /* ��¼��ʼʱ�� */
            this->start_time = QTime::currentTime();

            /* ��1֡���߶���֡ */
            this->psock_can_frame->clear();
            this->byte_cnt = 0;
            this->is_start = true;
            this->psock_can_frame->set_time(this->start_time);
            this->psock_can_frame->set_type(sock_can_data.ctrl());

            /* ���֡��Ϣ */
            this->psock_can_frame->set_protocol(QString("%1").arg(QObject::tr("���ݴ���֡")));

            /* ���Դ��Ŀ�ĵ�ַ */
            this->psock_can_frame->set_src(sock_can_data.src());
            this->psock_can_frame->set_dest(sock_can_data.dest());

            QByteArray byte_tmp((const char*)p_new_tmp, 8);

            can_msg_header_t header = SockCanFrame::byte_to_header(byte_tmp);
            QString          trans_service;
            switch ( header.tran) {
            case SockCanData::INFO_TRANS:
                trans_service = QString("%1/%2").arg(QObject::tr("��Ϣ�������")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::TIME_TRANS:
                trans_service = QString("%1/%2").arg(QObject::tr("ʱ�䴫�����")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::ON_BOARD:
                trans_service = QString("%1/%2").arg(QObject::tr("�ڹ���ٲ��Է���")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::FILE_TRANS:
                trans_service = QString("%1/%2").arg(QObject::tr("�ļ��������")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::DEV_DRIVER:
                trans_service = QString("%1/%2").arg(QObject::tr("�豸��������")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::TM_TRANS:
                /* ��Ҫң��ش� */
                if ( this->is_need_ack()) {
                    emit notify_ack_data(MainGui::TM_ACK);
                }
                trans_service = QString("%1/%2").arg(QObject::tr("ң�⴫�����")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::TC_TRANS:
                trans_service = QString("%1/%2").arg(QObject::tr("ң�ش������")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::MEM_DOWN:
                /* ��Ҫ�ڴ���ж�ش� */
                if ( this->is_need_ack()) {
                    emit notify_ack_data(MainGui::MEM_ACK);
                }
                trans_service = QString("%1/%2").arg(QObject::tr("�ڴ����ط���")).arg(this->psock_can_frame->get_type());
                break;

            case SockCanData::BUS_MANAGE:
                /* ��Ҫ�ظ����߹��� */
                if ( this->is_need_ack()) {
                    emit notify_ack_data(MainGui::BUS_MAN);
                }
                trans_service = QString("%1/%2").arg(QObject::tr("���߹���ά������")).arg(this->psock_can_frame->get_type());
                break;

            default:
                qDebug() << "service is invalid";
                return ;
            }

            this->psock_can_frame->set_service(trans_service);
            this->psock_can_frame->inc_frame_cnt();
            this->psock_can_frame->set_len(header.len);

            if ( sock_can_data.is_ind_frame()) {
                this->is_finish = true;
            }
            this->byte_cnt += 8;
        } else if ( (this->is_start) && (sock_can_data.is_last_frame())) {
            /* Last frame */
            if ( this->is_need_ack()) {
                if ( ((uchar)(*p_new_tmp) == 0x3C) && ((uchar)(*(p_new_tmp + 1)) == 0x11)) {
                    emit notify_ack_data(MainGui::PRJ_ACK);
                } else if ( ((uchar)(*p_new_tmp) == 0x3C) && ((uchar)(*(p_new_tmp + 1)) == 0x55)) {
                    emit notify_ack_data(MainGui::BAK_ACK);
                }
            }

            uint byte_left          = this->psock_can_frame->get_len_value() - this->byte_cnt;
            this->is_finish         = true;
            this->psock_can_frame->append_data(QByteArray(p_new_tmp, byte_left));
            this->psock_can_frame->set_time(this->start_time);
        } else {
            if ( !this->is_start) {
                qDebug() << "invalid frame";
                return ;
            }

            this->psock_can_frame->append_data(QByteArray(p_new_tmp, 8));
            this->byte_cnt += 8;
        }
    } else {
        qDebug() << "invalid";
        return ;
    }

    if ( this->is_start && this->is_finish) {
        this->byte_cnt  = 0;
        this->is_start  = false;
        this->is_finish = false;

        this->update_data_tables();
    }
}

void MainGui::update_data_tables()
{
    if ( this->psock_can_frame->is_data_tramsmission_pro()) {
        int cnt = this->ui->p_tab_rx->rowCount();
        this->ui->p_tab_rx->insertRow(cnt);

        this->ui->p_tab_rx->setItem(cnt, 0, new QTableWidgetItem(this->psock_can_frame->get_time()));
        this->ui->p_tab_rx->setItem(cnt, 1, new QTableWidgetItem(this->psock_can_frame->get_protocol()));
        this->ui->p_tab_rx->setItem(cnt, 2, new QTableWidgetItem(this->psock_can_frame->get_service()));
        this->ui->p_tab_rx->setItem(cnt, 3, new QTableWidgetItem(this->psock_can_frame->get_src()));
        this->ui->p_tab_rx->setItem(cnt, 4, new QTableWidgetItem(this->psock_can_frame->get_dest()));
        this->ui->p_tab_rx->setItem(cnt, 5, new QTableWidgetItem(QString("0x") + QString("%1").arg(this->psock_can_frame->get_len_value() - 8, 0, 16).toUpper()));
        this->ui->p_tab_rx->setItem(cnt, 6, new QTableWidgetItem(QString("%1").arg(this->psock_can_frame->get_data())));
    } else if ( this->psock_can_frame->is_device_identify_pro()) {
        int curr_row_cnt = this->ui->p_tab_rx_dev->rowCount();
        this->ui->p_tab_rx_dev->insertRow(curr_row_cnt);

        QString tmp("");
        QString str = this->psock_can_frame->get_oid().toHex();
        for (int i = 0; i < str.length(); i += 2) {
            tmp += str.mid(i, 2) + " ";
        }

        this->ui->p_tab_rx_dev->setItem(curr_row_cnt, 0, new QTableWidgetItem(this->psock_can_frame->get_time()));
        this->ui->p_tab_rx_dev->setItem(curr_row_cnt, 1, new QTableWidgetItem(this->psock_can_frame->get_type()));
        this->ui->p_tab_rx_dev->setItem(curr_row_cnt, 2, new QTableWidgetItem(this->psock_can_frame->get_src()));
        this->ui->p_tab_rx_dev->setItem(curr_row_cnt, 3, new QTableWidgetItem(tmp.trimmed().toUpper()));
        this->ui->p_tab_rx_dev->setItem(curr_row_cnt, 4, new QTableWidgetItem(this->psock_can_frame->get_dest()));
    }
}

void MainGui::send_ack_data(const QString &host_ip, ushort host_port, const QByteArray &byte)
{
    if ( host_ip.isEmpty()) {
        QMessageBox::warning(0,
                             QObject::tr("����"),
                             QObject::tr("Ŀ��IP��ַ��Ϊ��"));
        return ;
    }

    if ( host_port < 1000) {
        QMessageBox::warning(0,
                             QObject::tr("����"),
                             QObject::tr("Ŀ��˿ڲ���С��1000"));
        return ;
    }

    if ( byte.isEmpty()) {
        QMessageBox::warning(0,
                             QObject::tr("����"),
                             QObject::tr("�������ݲ���Ϊ��"));
        return ;
    }

    if ( !(this->ack_data_sendp)) {
        this->ack_data_sendp = new QUdpSocket;
    }

    QHostAddress host;
    host.setAddress(host_ip);
    this->ack_data_sendp->writeDatagram(byte, host, host_port);
}

void MainGui::on_p_btn_start_recv_err_toggled(bool flag)
{
    if ( flag) {
        /* Start receive error information */
        this->ui->p_btn_start_recv_err->setStyleSheet("background-color:green;");

        this->p_err_task = new ErrRxTask(this->ui->p_spin_recv_err_port->value());
        QObject::connect(this->p_err_task, SIGNAL(notify_err_frame(QByteArray)), this, SLOT(do_update_err_rx_msg(QByteArray)));
        QObject::connect(this, SIGNAL(notify_err_stop_flag()), this->p_err_task,  SLOT(update_stop_flag()));
        this->p_err_task->start();
    } else {
        this->ui->p_btn_start_recv_err->setStyleSheet("background-color:red;");

        emit notify_err_stop_flag();
        QObject::disconnect(this->p_err_task, SIGNAL(notify_err_frame(QByteArray)), this, SLOT(do_update_err_rx_msg(QByteArray)));
        QObject::disconnect(this, SIGNAL(notify_err_stop_flag()), this->p_err_task, SLOT(update_stop_flag()));

        this->p_err_task->quit();
        this->p_err_task->wait(10);

        delete this->p_err_task;
    }
}

void MainGui::on_p_btn_clear_recv_err_clicked()
{
    this->ui->p_tab_rx_err->setRowCount(0);
}

void MainGui::do_send_back_ack_data(int index)
{
    QByteArray  byte;
    QStringList byte_list;

    switch (index) {

    case MainGui::PRJ_ACK:
        /* ���̲����ش� */
        byte_list = this->ui->p_text_prj_para_ack->toPlainText().split(" ");
        for (size_t i = 0; i < byte_list.size(); ++i) {
            byte.append(byte_list.at(i).toInt(0, 16));
        }
        this->prj_ack_tx_cnt++;
        this->ui->p_label_prj_cnt->setText(QString("%1").arg(this->prj_ack_tx_cnt));
        break;

    case MainGui::BAK_ACK:
        /* ���ݲ����ش� */
        byte_list = this->ui->p_text_bak_para_ack->toPlainText().split(" ");
        for (size_t i = 0; i < byte_list.size(); ++i) {
            byte.append(byte_list.at(i).toInt(0, 16));
        }
        this->bak_ack_tx_cnt++;
        this->ui->p_label_bak_cnt->setText(QString("%1").arg(this->bak_ack_tx_cnt));
        break;

    case MainGui::TM_ACK:
        /* ң������ش� */
        byte_list = this->ui->p_text_tm_para_ack->toPlainText().split(" ");
        for (size_t i = 0; i < byte_list.size(); ++i) {
            byte.append(byte_list.at(i).toInt(0, 16));
        }
        this->tm_ack_tx_cnt++;
        this->ui->p_label_tm_cnt->setText(QString("%1").arg(this->tm_ack_tx_cnt));
        break;

    case MainGui::MEM_ACK:
        /* �ڴ���ж�ش� */
        byte_list = this->ui->p_text_mem_down_ack->toPlainText().split(" ");
        for (size_t i = 0; i < byte_list.size(); ++i) {
            byte.append(byte_list.at(i).toInt(0, 16));
        }
        this->mem_ack_tx_cnt++;
        this->ui->p_label_mem_cnt->setText(QString("%1").arg(this->mem_ack_tx_cnt));
        break;

    case MainGui::BUS_MAN:
        /* ���߹���ά�� */
        qDebug() << "bus manage.";
        this->bus_man_tx_cnt++;
        break;

    default:
        QMessageBox::warning(0,
                             QObject::tr("����"),
                             QObject::tr("�ش���������ȷ"));
    }

    this->send_ack_data(this->ui->p_line_tx_ip->text(), this->ui->p_spin_tx_port->value(), byte);
}

void MainGui::on_p_btn_start_tx_toggled(bool flag)
{
    if ( flag) {
        /* �ش� */
        this->ui->p_btn_start_tx->setStyleSheet("background-color:green;");
        this->ui->p_btn_start_tx->setText(QObject::tr("���ڻش�..."));
    } else {
        /* ����Ҫ�ش� */
        this->ui->p_btn_start_tx->setStyleSheet("background-color:red;");
        this->ui->p_btn_start_tx->setText(QObject::tr("ֹͣ�ش�"));
    }
}
