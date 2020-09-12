#ifndef CASHBOXSETFRAME_H
#define CASHBOXSETFRAME_H
/**
  钱箱gpio设置
  */
#include <QFrame>
#include <QLineEdit>
#include <QLabel>
#include <QThread>

#include "vkeyboardex.h"

#include "redis-cli/redisclient.h"
using namespace  Redis;

class CashBoxSetFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CashBoxSetFrame(QWidget *parent = nullptr);
    ~CashBoxSetFrame() override;

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

    inline int getCtrlGPIO() {
        return _ctrlPort;
    }

    inline int getDispGPIO() {
        return _dispPort;
    }

    void setDisplay(int value) {
        _display->setText( QString::number(value) );
    }
private:
    void init();

public slots:
    void focussed(QWidget*, bool);
    void onKeyDown(const QString&);

    void onVKeyDown(const QString&);

public slots:
    void connected();
    void disconnected();
    void messageReceived(const QString& channel, const QVariant& payload);
    void onReply(const QString& cmd, Redis::Reply value);
private:
    QLabel* _label;
    QLineEdit* _display; //显示状态
    LineEditEx* _ctrlID; //gpio export id;
    LineEditEx* _dispID; //gpio export id;
    VKeyboardEx* _vkb; //虚拟键

    VKeyItemButton* _kbOpen; //打开
    VKeyItemButton* _kbSet; //设置
    int _ctrlPort; // 控制端口
    int _dispPort; //显示端口

    RedisClient* _redis;
    RedisClient* _redisCmd;
};

#endif // CASHBOXSETFRAME_H
