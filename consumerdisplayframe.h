#ifndef CONSUMERDISPLAYFRAME_H
#define CONSUMERDISPLAYFRAME_H
/**
  客户显示屏
  显示 购物车信息
  */
#include <QFrame>

#include "shopcartview.h"
#include "OrderData.h"
#include "gridlayoutex.h"

class ConsumerDisplayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ConsumerDisplayFrame(QWidget *parent = nullptr);
    ~ConsumerDisplayFrame() override;

public:
    void init();

    void updateOrder(OrderBean& ob) ;

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

private:
    OrderBean* _ob; //当前订单

    ShopcartView* _shopcart; //购物车

    GridLayoutEx* _layout;
};

#endif // CONSUMERDISPLAYFRAME_H
