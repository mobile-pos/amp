#ifndef LAUNCHFRAME_H
#define LAUNCHFRAME_H

#include <QFrame>

#include "gridlayoutex.h"
#include "redis-cli/redisclient.h"

class LaunchFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LaunchFrame(QWidget *parent = nullptr);
    ~LaunchFrame();
public:
    void switchMainFrame();

    void init();

public:
    void showEvent(QShowEvent* e);
    void resizeEvent(QResizeEvent* e);
public slots:
    void connected();
    void disconnected();
    void onReply(const QString& cmd, Reply value);

private:
    bool loadTables();
    bool loadFlavors();
private:
    RedisClient *_rediscli;
    QLabel* _loading;
};

#endif // LAUNCHFRAME_H
