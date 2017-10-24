#ifndef ERRRXTASK_H
#define ERRRXTASK_H

#include <QThread>
#include <QtNetwork/QUdpSocket>
#include <QtCore>

class ErrRxTask : public QThread
{
    Q_OBJECT
public:
    explicit ErrRxTask(QObject *parent = 0);
    explicit ErrRxTask(ushort port, QObject *parent = 0);
    ~ErrRxTask();
    
protected:
    void run(void);
    void stop(void);

signals:
    void notify_err_frame(const QByteArray &byte);
    
public slots:
    void update_stop_flag(void);

private:
    enum PortDefault {DEFAULT_ERR_PORT = 55000};
    QUdpSocket *err_udp_socket;
    ushort      port;
    bool        is_stop;
    QMutex      lock;
};

#endif // ERRRXTASK_H
