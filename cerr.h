#ifndef CERR_H
#define CERR_H

#include <QObject>

class CErr : public QObject
{
    Q_OBJECT
public:
    explicit CErr(QObject *parent = 0);
    explicit CErr(const QByteArray &byte, QObject *parent = 0);
    ~CErr();
    
    ushort  get_type_value(void) const;
    QString get_type_str(void) const;
    ushort  get_code_value(void) const;
    QString get_code_str(void) const;
    QString get_file_name(void) const;
    QString get_func_name(void) const;
    uint    get_line_value(void) const;
    QString get_line_str(void) const;
    QString get_err_desp(void) const;

    void    set_type(ushort _type);
    void    set_code(ushort _code);
    void    set_file_name(const QString &name);
    void    set_func_name(const QString &name);
    void    set_line(uint _line);
    void    set_err_desp(const QString &desp);

signals:
    
public slots:
    
private:
    enum OffSet {TYPE_OFF = 0x2, CODE_OFF = 0x2, FILE_OFF = 0x1E,
                FUNC_OFF  = 0x1E, LINE_OFF = 0x4, DESP_OFF = 0x40};
    ushort  type;
    ushort  code;
    QString file_name;
    QString func_name;
    uint    line;
    QString err_desp;
};

#endif // CERR_H
