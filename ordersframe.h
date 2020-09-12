#ifndef ORDERSFRAME_H
#define ORDERSFRAME_H

#include <QFrame>
#include <QThread>
#include <QAbstractTableModel>
#include <QLineEdit>
#include <QTableView>

#include "OrderData.h"
#include "datathread.h"
#include "tabviewpagerframe.h"


class OrdersModel;

class BookingOrdersModel : public OrdersModel
{
public:
    BookingOrdersModel(QObject * parent = {}) ;
public:
    int columnCount(const QModelIndex &) const override;


    QVariant data(const QModelIndex &index, int role) const override ;

    QVariant headerData(int section, Qt::Orientation orientation, int role) const override ;
};

class OrdersFrame : public QFrame
{
    Q_OBJECT

public:
    explicit OrdersFrame(OrderType type, QString statuses = QString::number(POSDATA::OD_UNKNOWN), QWidget *parent = nullptr);
    ~OrdersFrame() override;
public slots:
    void rowSelected(const QModelIndex & index );

    void toPage(int pageid);

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent *e) override;


    inline void setFilterType(const QString& type) {
        _odType = OrderType(type);
    }
    inline void setFilterStatus(QString statuses) {
        _odStatuses = statuses;
    }

    void updateOrders();

    inline OrdersModel* getModel() {
        return m_model;
    }
private:
    void initHeader();


private:
    OrderType _odType; //订单类型
    QString _odStatuses; //订单状态，支持多个状态，从个状态之间用 | 来分割
    OrdersModel* m_model;

    QTableView* m_ordersTv;
    TabViewPagerFrame* _pager;
};

#endif // ORDERSFRAME_H
