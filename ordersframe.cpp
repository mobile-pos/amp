#include "ordersframe.h"

#include "orderpainter.h"
#include "datathread.h"
#include <QDebug>

#include "homeframe.h"

#include "mainwindow.h"

extern MainWindow* gWnd;



BookingOrdersModel::BookingOrdersModel(QObject * parent) : OrdersModel(parent) {

}
int BookingOrdersModel::columnCount(const QModelIndex &) const  { return 6; }


QVariant BookingOrdersModel::data(const QModelIndex &index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
  const auto & order = m_data[index.row()];
  switch (index.column()) {
      case 0: return index.row();
      case 1: return order.sid;
      case 2: return order.time;
      case 3: return order.cookingTime;
      case 4: return "$" + QString::number(order.amount);
      case 5: return orderStatus(order.status);
      default: return {};
  }
}

QVariant BookingOrdersModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
  switch (section) {
  case 0: return "#";
  case 1: return QObject::tr("订单号");
  case 2: return QObject::tr("下单时间");
  case 3: return QObject::tr("预约时间");
  case 4: return QObject::tr("总消费");
  case 5: return QObject::tr("状态");
  default: return {};
  }

}

///////////////////////////////////////////////////////////
/// \brief OrdersFrame::OrdersFrame
/// \param type
/// \param status
/// \param parent
///
OrdersFrame::OrdersFrame(OrderType type, QString statuses, QWidget *parent) :
    QFrame(parent),
    _odType(type),
    _odStatuses(statuses)
{
    qDebug() << "OrdersFrame::init";

    m_ordersTv = new QTableView(this);

    if(_odStatuses.startsWith( QString::number(POSDATA::OD_BOOKING_SHARE) )
       || _odStatuses.startsWith( QString::number(POSDATA::OD_BOOKING_TAKEUP )))
    {
        m_model = new BookingOrdersModel(this);
    } else {
        m_model = new OrdersModel(this);
    }

    _pager = new TabViewPagerFrame(this);
    this->connect(_pager, SIGNAL(toPage(int)), this, SLOT(toPage(int)));

    initHeader();

    qDebug() << "OrdersFrame::init end";
}

OrdersFrame::~OrdersFrame()
{
}


void OrdersFrame::initHeader() {

    m_ordersTv->setSelectionMode(QAbstractItemView::SingleSelection);
    m_ordersTv->setEditTriggers(QAbstractItemView::NoEditTriggers);
    m_ordersTv->setSelectionBehavior(QAbstractItemView::SelectRows);

    m_ordersTv->setModel(m_model);

    this->connect(m_ordersTv, SIGNAL(clicked(const QModelIndex & )), this, SLOT(rowSelected(const QModelIndex &)));

}
void OrdersFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();

    m_ordersTv->setGeometry(0, 0, rect.width(), rect.height() - 10);
    _pager->setGeometry(0, rect.height() - 40, rect.width(), 35);
}

void OrdersFrame::showEvent(QShowEvent *e) {
    QFrame::showEvent(e);

    QRect rect = this->frameGeometry();

    m_ordersTv->setColumnWidth(0, 40);

    int colcount = m_model->columnCount(QModelIndex());
    int colWidth = rect.width() / ( colcount - 1) - 10;

    for(int i = 1; i < colcount; i++) {
        m_ordersTv->setColumnWidth(i, colWidth);
    }

    updateOrders();

}

void OrdersFrame::updateOrders() {
    m_model->clean();
    m_ordersTv->update();

    int total = 0;
    QList<QVariant> orders = DataThread::inst().loadOrders(total, _odType.curType(), _odStatuses, _pager->curPage());

    foreach (QVariant order, orders) {
        QVariantMap item = order.toMap();
        int oid = item["id"].toInt();
        QString sid = item["sid"].toString();
        QString time = item["time"].toString();
        QString cookingTime = item["cookingTime"].toString();
        QString orderid = item["orderid"].toString();
        QString count = item["count"].toString();
        double amount = item["amount"].toDouble();
        int status = item["status"].toInt();

        OrderBean ob(oid, sid, time, amount, orderid, status);
        ob.cookingTime = cookingTime;

        m_model->append(ob);
    }

    int rows = m_model->rowCount(QModelIndex());
    for(int i = 0; i< rows; i++) {
        m_ordersTv->setRowHeight(i, 45);
    }

    _pager->setPager(total);
}

void OrdersFrame::rowSelected(const QModelIndex & index ) {
    qDebug() << m_model->rowCount(index);

    OrderBean ob = m_model->order(index);

    QVariantMap order = DataThread::inst().loadOrderByOId(ob.oid);
    OrderBean _ob2(order);
    gWnd->openMenuFrame(_ob2);
}

void OrdersFrame::toPage(int pageId) {
    qDebug() << "当前页码为：" << pageId;
    updateOrders();
}
