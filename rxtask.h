#ifndef RXTASK_H
#define RXTASK_H

#include <QThread>
#include <QtNetwork/QUdpSocket>
#include <QTime>
#include <QtCore>

class SockCanData;

class RxTask : public QThread
{
    Q_OBJECT
public:
    enum port_default_e {DEFAULT_PORT = 55009};
    enum data_size_default {DEFAULT_DATA_SIZE = 25};
    explicit RxTask(QObject *parent = 0);
    RxTask(quint16 port, QObject *parent = 0);
    ~RxTask();
    
protected:
    void run(void);
    void stop(void);

signals:
    void notify_can_frame(const QByteArray &byte);
    
public slots:
    void update_stop_flag(void);
    
private:
    QUdpSocket   *p_udp_socket;
    quint16       port;
    QTime         start_time;

    bool          is_stop;

    QMutex        locker;
};

#endif // RXTASK_H
