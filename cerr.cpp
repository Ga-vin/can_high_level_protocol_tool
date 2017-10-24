#include <QtCore>
#include "cerr.h"
#include "sockcandata.h"

CErr::CErr(QObject *parent) :
    QObject(parent)
{
    this->type = 0;
    this->code = 0;
    this->line = 0;
    this->file_name = QString("");
    this->func_name = QString("");
    this->err_desp  = QString("");
}

CErr::CErr(const QByteArray &byte, QObject *parent) :
    QObject(parent)
{
    if ( byte.isEmpty()) {
        this->type      = 0;
        this->code      = 0;
        this->line      = 0;
        this->file_name = QString("");
        this->func_name = QString("");
        this->err_desp  = QString("");
    } else {
        uchar *p_data = (uchar *)byte.data();

        this->type = (*(p_data)<<8) | (*(p_data + 1));
        p_data += CErr::TYPE_OFF;

        this->code = (*(p_data)<<8) | (*(p_data + 1));
        p_data += CErr::CODE_OFF;

        this->file_name = QByteArray((const char *)p_data, CErr::FILE_OFF);
        p_data += CErr::FILE_OFF;

        this->func_name = QByteArray((const char *)p_data, CErr::FUNC_OFF);
        p_data += CErr::FUNC_OFF;

        this->line = (*(p_data)<<24) | (*(p_data + 1)<<16) | (*(p_data + 2)<<8) | (*(p_data + 3));
        p_data += CErr::LINE_OFF;

        this->err_desp = QString(QByteArray((const char *)p_data, CErr::DESP_OFF));
    }
}

CErr::~CErr()
{
}

ushort CErr::get_type_value() const
{
    return (this->type);
}

QString CErr::get_type_str() const
{
    return (QString("0x") + QString("%1").arg(this->type, 0, 16).toUpper());
}

ushort CErr::get_code_value() const
{
    return (this->code);
}

QString CErr::get_code_str() const
{
    return (QString("0x") + QString("%1").arg(this->code, 0, 16).toUpper());
}

QString CErr::get_file_name() const
{
    return (this->file_name);
}

QString CErr::get_func_name() const
{
    return (this->func_name);
}

uint CErr::get_line_value() const
{
    return (this->line);
}

QString CErr::get_line_str() const
{
    return (QString("%1").arg(this->line) + QObject::tr("ÐÐ"));
}

QString CErr::get_err_desp() const
{
    return (this->err_desp);
}

void CErr::set_type(ushort _type)
{
    if ( this->type != _type) {
        this->type = _type;
    }
}

void CErr::set_code(ushort _code)
{
    if ( this->code != _code) {
        this->code = _code;
    }
}

void CErr::set_file_name(const QString &name)
{
    if ( this->file_name != name) {
        this->file_name = name;
    }
}

void CErr::set_func_name(const QString &name)
{
    if ( this->func_name != name) {
        this->func_name = name;
    }
}

void CErr::set_line(uint _line)
{
    if ( this->line != _line) {
        this->line = _line;
    }
}

void CErr::set_err_desp(const QString &desp)
{
    if ( this->err_desp != desp) {
        this->err_desp = desp;
    }
}
