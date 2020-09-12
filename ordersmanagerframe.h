#ifndef ORDERSMANAGERFRAME_H
#define ORDERSMANAGERFRAME_H

/**
  订单管理，即可以删除，修改订单状态
  */

#include <QFrame>
#include <QTableView>

#include "vkeyboardex.h"
#include "tabviewpagerframe.h"
#include "OrderData.h"
#include "gridlayoutex.h"

class OrdersModel;

class OrdersManagerModel : public OrdersModel
{
public:
    OrdersManagerModel(QObject *parent = {});

public:
    int columnCount(const QModelIndex &) const override;

    QVariant headerData(int section,
                        Qt::Orientation orientation,
                        int role) const override;
    QVariant data(const QModelIndex &index, int role) const override ;
    //    QVariant headerData(int section, Qt::Orientation orientation, int role) const override ;
};

class OrdersManagerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit OrdersManagerFrame(QWidget *parent = nullptr);
    ~OrdersManagerFrame();

public:
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *) override;

    void updateOrders(int pageId);

private:
    void init();

    void initOrderDetail(OrderBean &ob);
public slots:
    void toPage(int pageid);
    void onClicked(const QModelIndex &index);
    void rowSelected(const QModelIndex &index);

private:
    QTableView *_odTab;
    OrdersManagerModel *_model;

    TabViewPagerFrame *_pager;

    GridLayoutEx *_orderdetailLayout;
};

#endif // ORDERSMANAGERFRAME_H
