#ifndef ORDERPAYFRAME_H
#define ORDERPAYFRAME_H

#include <QFrame>
#include <QLineEdit>
#include <QListWidget>

#include "gridlayoutex.h"
#include "OrderData.h"
#include "gratuityframe.h"
#include "qrcodepayframe.h"

#include "redis-cli/redisclient.h"
#include "redis-cli/command.h"
using namespace  Redis;

class OrderPayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit OrderPayFrame(OrderBean* ob, QWidget *parent = nullptr);
    ~OrderPayFrame() override;
public:
    void resizeEvent(QResizeEvent* e) override;
    void showEvent(QShowEvent*) override;

    void init();

    void hideEvent(QHideEvent *event) override;
public:
    void updateOrder(OrderBean* ob);

    void newWeiXinQRCodePay();
    void newAliQRCodePay();
signals:
    void onFinishPay();
public slots:
    void onKeyDown(const QString&);

    void on_tips_clicked();
    void onGratuityDone(QList<GratuityItem>& tips);

    void onlinePayFinished(int oid, int type);
public slots:
    void connected();
    void disconnected();
    void onReply(const QString& cmd, Redis::Reply value);
private:
    GridLayoutEx* _layout;
    OrderBean* _ob;

    QLineEdit* _amount;

    QLineEdit* _paied; //实收

    GratuityFrame* _tipFrm; //小费
    QListWidget* _tipSummary; //小费概述

    //用于开钱箱
    RedisClient* _redis;
    //二维码支付显示
    QRCodePayFrame* _qrcFrm;
};

#endif // ORDERPAYFRAME_H
