#include <QtGui>
#include <QtCore>
#include <QTextCodec>
#include <QDebug>
#include <QTableWidgetItem>
#include <QRegExpValidator>
#include "maingui.h"
#include "sockcandata.h"


MainGui::MainGui(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::MainGUI)
{
    ui->setupUi(this);
    QTextCodec::setCodecForTr(QTextCodec::codecForName("gb18030"));

    this->init_widget();
    this->init_timer();
    this->make_connections();
}

MainGui::~MainGui()
{
    delete ui;
    p_task->terminate();

    delete p_task;
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
        p_task->start();
    } else {
        emit notify_change_stop_flag();
        QObject::disconnect(this, SIGNAL(notify_change_stop_flag()), p_task, SLOT(update_stop_flag()));
        p_task->quit();
        p_task->wait(10);

        delete p_task;
    }
}

void MainGui::do_update_rx_msg(const SockCanFrame *frame)
{
    qDebug() << "receive data";
    int cnt = this->ui->p_tab_rx->rowCount();
    this->ui->p_tab_rx->insertRow(cnt);

    QTableWidgetItem *p_item_0 = new QTableWidgetItem;
    QTableWidgetItem *p_item_1 = new QTableWidgetItem;
    QTableWidgetItem *p_item_2 = new QTableWidgetItem;
    QTableWidgetItem *p_item_3 = new QTableWidgetItem;
    QTableWidgetItem *p_item_4 = new QTableWidgetItem;
    QTableWidgetItem *p_item_5 = new QTableWidgetItem;
    QTableWidgetItem *p_item_6 = new QTableWidgetItem;

    if ( frame->is_data_tramsmission_pro()) {
        p_item_0->setText(frame->get_time());
        p_item_1->setText(frame->get_protocol());
        p_item_2->setText(frame->get_service());
        p_item_3->setText(frame->get_src());
        p_item_4->setText(frame->get_dest());
        p_item_5->setText(QString("%1").arg(frame->get_len()));
        p_item_6->setText(QString("%1").arg(frame->get_data()));

        this->ui->p_tab_rx->setItem(cnt, 0, p_item_0);
        this->ui->p_tab_rx->setItem(cnt, 1, p_item_1);
        this->ui->p_tab_rx->setItem(cnt, 2, p_item_2);
        this->ui->p_tab_rx->setItem(cnt, 3, p_item_3);
        this->ui->p_tab_rx->setItem(cnt, 4, p_item_4);
        this->ui->p_tab_rx->setItem(cnt, 5, p_item_5);
        this->ui->p_tab_rx->setItem(cnt, 6, p_item_6);
    } else if ( frame->is_device_identify_pro()) {
        QString tmp("");
        QString str = frame->get_oid().toHex();
        for (int i = 0; i < str.length(); i += 2) {
            tmp += str.mid(i, 2) + " ";
        }

        p_item_0->setText(frame->get_time());
        p_item_1->setText(frame->get_type());
        p_item_2->setText(frame->get_src());
        p_item_3->setText(tmp.trimmed().toUpper());
        p_item_4->setText(frame->get_dest());

        this->ui->p_tab_rx_dev->setItem(cnt, 0, p_item_0);
        this->ui->p_tab_rx_dev->setItem(cnt, 1, p_item_1);
        this->ui->p_tab_rx_dev->setItem(cnt, 2, p_item_2);
        this->ui->p_tab_rx_dev->setItem(cnt, 3, p_item_3);
        this->ui->p_tab_rx_dev->setItem(cnt, 4, p_item_4);
    }
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
                                 QObject::tr("警告"),
                                 QObject::tr("转换结果不能为空"));
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
                                     QObject::tr("警告"),
                                     QObject::tr("设备识别协议中识别控制字不正确，只能为1、2、3"));
                flag = false;
            } else {
                uint value = arbit.user_code.dev_oid_hi;
                value = ((value&0xFF0000)>>16) | ((value&0x0000FF)<<16) | (value&0x00FF00);
                this->ui->p_line_id_oid->setText(QString("0x%1").arg(value, 0, 16));
            }
        } else if ( SockCanData::DATA_TRANSMISSION == arbit.identify_code) {
            if ( !SockCanFrame::is_valid_ctrl_code(arbit)) {
                QMessageBox::warning(0,
                                     QObject::tr("警告"),
                                     QObject::tr("数据传输识别协议中识别控制字不正确，只能为0、1、2"));
                flag = false;
            } else {
                this->ui->p_line_id_dest->setText(QString("0x") + QString("%1").arg(arbit.user_code.defined.dest_nid, 0, 16).toUpper());
                this->ui->p_line_id_src->setText(QString("0x") + QString("%1").arg(arbit.user_code.defined.src_nid, 0, 16).toUpper());
                this->ui->p_line_id_frame_cnt->setText(QString("0x") + QString("%1").arg(arbit.user_code.defined.frame_cnt, 0, 16).toUpper());
            }
        } else {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("输入的字符序列中设备识别码不正确，bit28-26只能为0或者6"));
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
                                     QObject::tr("警告"),
                                     QObject::tr("设备协议中唯一识别码高24位不能为空"));
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
                                     QObject::tr("警告"),
                                     QObject::tr("帧类型控制码只能为1, 2, 3"));
                flag = false;
            }
        } else if ( SockCanData::DATA_TRANSMISSION == identify) {
            if ( this->ui->p_line_id_src->text().isEmpty() ||
                 this->ui->p_line_id_dest->text().isEmpty() ||
                 this->ui->p_line_id_frame_cnt->text().isEmpty()) {
                QMessageBox::warning(0,
                                     QObject::tr("警告"),
                                     QObject::tr("数据传输协议中源地址、目的地址、帧序号不能为空"));
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
                                     QObject::tr("警告"),
                                     QObject::tr("帧类型控制码只能为0, 1, 2, 3"));
                flag = false;
            }
        } else {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("协议识别码只能为0或者6"));
            flag = false;
        }

        if ( flag) {
            uchar p_data[4];
            memcpy(p_data, &arbit, sizeof(arbit));
            p_data[0] = ((p_data[0]&0xC0)>>6) | ((p_data[0]&0xE0)<<5) | ((p_data[0]&0x38)>>1);  /* 调整字节内的比特序列 */
            QByteArray display_result((const char*)(p_data), 4);
            this->ui->p_line_id_result->setText(SockCanData::bytearray_to_hex_str(display_result));
        } else {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("转换失败"));
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
                                 QObject::tr("警告"),
                                 QObject::tr("转换结果不能为空"));
            this->ui->p_line_head_result->setFocus();
        }

        QStringList str_list = str.split(" ");
        for (size_t i = 0; i < str_list.size(); ++i) {
            byte.append(str_list.at(i).toUInt(0, 16));
        }

        if ( SockCanFrame::calc_chksum((uchar *)byte.data(), sizeof(can_msg_header_t))) {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("首部校验结果不合法"));
        }

        uchar *p_data   = (uchar *)byte.data();

        header.version  = ((*p_data)&0xE0) >> 5;
        header.len      = (((*p_data)&0x1F)<<6) | ((*(p_data + 1)&0xFC) >> 2);
        header.reserve1 = *(p_data + 1)&0x3;
        header.tran     = (*(p_data + 2)&0xF0) >> 4;
        if ( !SockCanFrame::is_valid_service(header.tran)) {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("传输服务不合法"));
        }

        header.ctrl     = *(p_data + 2)&0x0F;
        memcpy(&(header.src), p_data + 3, sizeof(uchar) * 5);

        if ( !SockCanFrame::is_valid_net_addr(header.src)) {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("源地址不合法"));
        }

        if ( !SockCanFrame::is_valid_net_addr(header.dest)) {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("目标地址不合法"));
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
                                 QObject::tr("警告"),
                                 QObject::tr("目标地址不合法"));
        }

        if ( !SockCanFrame::is_valid_net_addr(src)) {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("源地址不合法"));
        }

        if ( !SockCanFrame::is_valid_service(serv)) {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("传输服务类型不合法"));
        }

        if ( !SockCanFrame::is_valid_msg_len(len)) {
            QMessageBox::warning(0,
                                 QObject::tr("警告"),
                                 QObject::tr("数据长度不合法，不得大于2047"));
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
                                 QObject::tr("警告"),
                                 QObject::tr("转换结果不能为空"));
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

void MainGui::init_widget()
{
    /* Title */
    this->setWindowTitle(QObject::tr("CAN高层协议验证软件 -- by Gavin.Bai"));

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

    /* 平均表格每行 */
    this->ui->p_tab_rx_dev->horizontalHeader()->setResizeMode(QHeaderView::Stretch);

    /* Port range */
    this->ui->p_spin_rx_port->setRange(1000, 999999);
    this->ui->p_spin_rx_port->setValue(55009);

    /* Add prefix */
    QRegExp exp("[0]{1}|[6]{1}");
    this->ui->p_line_id_identify->setValidator(new QRegExpValidator(exp, this));
    this->ui->p_line_id_identify->setWhatsThis(QObject::tr("只能输入0或者6"));

    this->ui->p_line_id_ctrl->setValidator(new QRegExpValidator(QRegExp("[0-3]{1}"), this));
    this->ui->p_line_id_ctrl->setWhatsThis(QObject::tr("只能输入0、1、2、3"));

    this->ui->p_line_head_chksum->setReadOnly(true);
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
}

void MainGui::change_btn_color(bool flag)
{
    if ( flag) {
        this->ui->p_btn_start_rx->setStyleSheet("background-color:green");
    } else {
        this->ui->p_btn_start_rx->setStyleSheet("background-color:red");
    }
}
