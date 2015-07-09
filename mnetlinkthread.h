#ifndef MNETLINKTHREAD_H
#define MNETLINKTHREAD_H

#include <QThread>

class mNetLinkThread : public QThread
{
    Q_OBJECT
public:
    explicit mNetLinkThread(QObject *parent = 0);

signals:

public slots:

};

#endif // MNETLINKTHREAD_H
