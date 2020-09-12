#ifndef HOMEFRAME_H
#define HOMEFRAME_H

#include <QFrame>
#include <QLineEdit>

#include "orderpainter.h"
#include "ordersframe.h"
#include "tablesframe.h"
#include "gridlayoutex.h"
#include "posbuttonex.h"

#include "redis-cli/redisclient.h"
using namespace  Redis;


class HomeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit HomeFrame(QWidget *parent = nullptr);
    ~HomeFrame() override;

private:
    void initTablePane();
    void initOrderPane();
    void initNavPane(); //左边的基本导航栏
public:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent*e) override;
    void hide();

    void retranslateUI(); //更新资源
//    void show();
public slots:
    //未完成的订单，正在下单中
    void on_m_allOrderingOrders_clicked();
    //所有的订单
    void on_m_allOrders_clicked();

    void on_m_allPaiedOrders_clicked();
    void on_m_allUnpaiedOrders_clicked();

    void on_m_waideng_clicked();
    void on_m_delevery_clicked();
    void on_m_tangshi_clicked();

    void on_deliveryUnpaiedOrders_clicked();
    void on_deliveryPaiedOrders_clicked();

    void on_pickupPaiedOrders_clicked();
    void on_pickupUnpaiedOrders_clicked();

    void on_m_onlineUnpaiedOrders_clicked();
    void on_m_onlinePaiedOrders_clicked();

    void onKeyDown(const QString&);

    void onBookingUnpaiedOrdersClicked();

    void onCallInLin1Clicked();
    void onCallInLin2Clicked();
public slots:
    void connected();
    void disconnected();
    void onReply(const QString& cmd, Reply value);
    void messageReceived(const QString& channel, const QVariant& payload);


private:
    void showTabFrame();
    void showOrderFrame();
    void showBookingOrderFrame();
private:

    QWidget* m_parentWnd;
    //台桌布局
    TablesFrame* m_tabFrame;
    //订单布局
    OrdersFrame* m_orFrame;
    OrdersFrame* _bookingFrame;

    GridLayoutEx* _leftlayout;
    //电话号
    QLineEdit* _callid;

    RedisClient* _rediscli;
};

#endif // HOMEFRAME_H
