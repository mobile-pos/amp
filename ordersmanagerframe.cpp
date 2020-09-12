#include "ordersmanagerframe.h"
#include <QAbstractTableModel>
#include <QDebug>
#include <QMessageBox>

#include "datathread.h"
#include "orderpainter.h"
#include "TableviewEx/buttondelegate.h"

#include "mainwindow.h"

extern MainWindow *gWnd;

OrdersManagerModel::OrdersManagerModel(QObject *parent) : OrdersModel(parent)
{
}

int OrdersManagerModel::columnCount(const QModelIndex &) const
{
    return 6;
}


QVariant OrdersManagerModel::headerData(int section,
                                        Qt::Orientation orientation,
                                        int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
  switch (section) {
  case 0: return "#";
  case 1: return QObject::tr("订单号");
  case 2: return QObject::tr("总消费");
  case 3: return QObject::tr("状态");
  default: return "";
  }

}
QVariant OrdersManagerModel::data(const QModelIndex &index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
  const auto & order = m_data[index.row()];
  switch (index.column()) {
      case 0: return index.row();
      case 1: return order.sid + "\n" + order.time;
      case 2: return "$" + QString::number(order.amount);
      case 3: return orderStatus(order.status);
      default: return {};
  }
}
//////////////////////////////////////////////////////////////////////////////
OrdersManagerFrame::OrdersManagerFrame(QWidget *parent) : QFrame(parent)
{

    this->setStyleSheet("background-color: #ffffff");
    init();
}
OrdersManagerFrame::~OrdersManagerFrame()
{
}

void OrdersManagerFrame::init()
{
    _odTab = new QTableView(this);
    _orderdetailLayout = new GridLayoutEx(10, this);
    _orderdetailLayout->setRowHeight(20);

    _pager = new TabViewPagerFrame(this);
    this->connect(_pager, SIGNAL(toPage(int)), this, SLOT(toPage(int)));

    _model = new OrdersManagerModel(this);

    _odTab->setSelectionMode(QAbstractItemView::SingleSelection);
    _odTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _odTab->setSelectionBehavior(QAbstractItemView::SelectRows);

    _odTab->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);
    _odTab->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);

    _odTab->setModel(_model);

    ButtonDelegate *_fnish = new ButtonDelegate(QObject::tr("结单"), dynamic_cast<QFrame *>(this));
    ButtonDelegate *_drop = new ButtonDelegate(QObject::tr("废弃"), dynamic_cast<QFrame *>(this));

    this->connect(_fnish, SIGNAL(onClicked(const QModelIndex &)), this, SLOT(onClicked(const QModelIndex &)));
    this->connect(_drop, SIGNAL(onClicked(const QModelIndex &)), this, SLOT(onClicked(const QModelIndex &)));

    _odTab->setItemDelegateForColumn(4, _fnish);
    _odTab->setItemDelegateForColumn(5, _drop);
    this->connect(_odTab, SIGNAL(clicked(const QModelIndex &)), this, SLOT(rowSelected(const QModelIndex &)));
}

void OrdersManagerFrame::initOrderDetail(OrderBean &ob)
{
    _orderdetailLayout->clean();
    QVariantMap order = DataThread::inst().loadOrderByOId(ob.oid);
    //10列
    {
        //
        _orderdetailLayout->addWidget(new QLabel(tr("订单号"), _orderdetailLayout), 3);

        QLineEdit *le = new QLineEdit(_orderdetailLayout);
        le->setObjectName("OrderName");
        le->setEnabled(false);
        le->setText( ob.sid );

        _orderdetailLayout->addWidget(le, 7);

        _orderdetailLayout->addWidget(new QLabel("", _orderdetailLayout), 10);
    }
    {
        //菜品列表
        _orderdetailLayout->addWidget(new QLabel(tr("菜品"), _orderdetailLayout), 4);
        _orderdetailLayout->addWidget(new QLabel(tr("单价"), _orderdetailLayout), 3);
        _orderdetailLayout->addWidget(new QLabel(tr("份量"), _orderdetailLayout), 3);

        double total = 0;

        foreach (QVariant food, order["foods"].toList())
        {
            FoodBean fb;
            QVariantMap fooditem = food.toMap();
            fb.reid = fooditem["reid"].toInt();
            fb.count = fooditem["count"].toInt();
            fb.name = fooditem["name"].toString();
            //        fb.enName = fooditem["enName"].toString();
            fb.unit_price = fooditem["unit_price"].toDouble();
            //        gb.flavor = fooditem["flavor"].toString().toStdString();
            fb.flavor = QByteArray::fromPercentEncoding(fooditem["flavor"].toString().toUtf8());
            fb.cid = fooditem["cid"].toInt();
            fb.rid = fooditem["rid"].toInt();
            fb.discount = fooditem["discount"].toDouble();
            fb.id = fooditem["fid"].toInt();
            fb.oid = fooditem["oid"].toInt();
            fb.icon = fooditem["icon"].toString();

            total += fb.count * fb.unit_price * (1 - fb.discount);

            qDebug() << "flavor: " << fb.flavor;

            _orderdetailLayout->addWidget(new QLabel(fb.name, _orderdetailLayout), 4);
            _orderdetailLayout->addWidget(new QLabel(QString::number(fb.unit_price), _orderdetailLayout), 3);
            _orderdetailLayout->addWidget(new QLabel(QString::number(fb.count), _orderdetailLayout), 3);
            if (fb.discount > 0.0)
            {
                _orderdetailLayout->addWidget(new QLabel("", _orderdetailLayout), 3);
                _orderdetailLayout->addWidget(new QLabel(QString(tr("折扣:%1")).arg(fb.discount), _orderdetailLayout), 7);
            }
        }
        _orderdetailLayout->addWidget(new QLabel("", _orderdetailLayout), 10);

        double tx = DataThread::inst().getRestaurantInfo().value("taxRate").toDouble();
        double total2 = total + total * tx;

        _orderdetailLayout->addWidget(new QLabel(QString(tr("税费: %1")).arg(total * tx), _orderdetailLayout), 10);
        _orderdetailLayout->addWidget(new QLabel(QString(tr("Tips: %1")).arg(order["gratuity"].toString()), _orderdetailLayout), 10);
        _orderdetailLayout->addWidget(new QLabel(QString(tr("计费: %1")).arg(total2), _orderdetailLayout), 10);
    }

    _orderdetailLayout->resetPages();
}

void OrdersManagerFrame::resizeEvent(QResizeEvent *e)
{
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();

    _odTab->setColumnWidth(0, 20);
    _odTab->setColumnWidth(1, 150);
    _odTab->setColumnWidth(2, 100);
    _odTab->setColumnWidth(3, 60);
    _odTab->setColumnWidth(4, 80);
    _odTab->setColumnWidth(5, 80);

    _odTab->setGeometry(0, 0, 20 + 150 + 100 + 60 + 80 * 2 + 20, rect.height() - 45);
    _pager->setGeometry(0, rect.height() - 40, rect.width(), 35);

    int w = rect.width() - _odTab->geometry().width();
    int x = _odTab->geometry().width() + 20;
    if (w < 200)
    {
        w = 200;
        x = rect.width() - w;
    }
    _orderdetailLayout->setGeometry(x, 0, w, rect.height() - 80);

    qDebug() << "[resize event] orders manager frame size: " << rect;
}
void OrdersManagerFrame::showEvent(QShowEvent *e)
{
    QFrame::showEvent(e);

    QRect rect = this->geometry();
    qDebug() << "[show event] orders manager frame size: " << rect;

    updateOrders(0);
}

void OrdersManagerFrame::toPage(int pageId)
{
    qDebug() << "当前页码为：" << pageId;

    updateOrders(_pager->curPage());
}

void OrdersManagerFrame::updateOrders(int pageId)
{
    //只取 7 天内数据

    _model->clean();
    _odTab->update();

    int total = 0;
    QList<QVariant> orders = DataThread::inst().loadOrders(total,
                                                           OrderType::ALL, QString("%1").arg(POSDATA::OD_ALLSTATUS),
                                                           pageId,
                                                           7);

    foreach (QVariant order, orders)
    {
        QVariantMap item = order.toMap();
        int oid = item["id"].toInt();
        QString sid = item["sid"].toString();
        QString time = item["time"].toString();
        QString orderid = item["orderid"].toString();
        QString count = item["count"].toString();
        double amount = item["amount"].toDouble();
        int status = item["status"].toInt();

        OrderBean ob(oid, sid, time, amount, orderid, status);

        _model->append(ob);
    }

    int rows = _model->rowCount(QModelIndex());
    for (int i = 0; i < rows; i++)
    {
        _odTab->setRowHeight(i, 45);
    }

    _pager->setPager(total);
}

void OrdersManagerFrame::onClicked(const QModelIndex &index)
{

    OrderBean ob = _model->order(index);

    int code = -1;
    if (index.column() == 4)
    { // 结单
        code = DataThread::inst().auditOrder(ob.oid, POSDATA::OD_FINISHORDER);
    }
    else if (index.column() == 5)
    { //废弃当前订单
        code = DataThread::inst().auditOrder(ob.oid, POSDATA::OD_REJECTOD);
    }
    else
    {
        QMessageBox::warning(nullptr, tr("出错"), tr("未知点击事件"));
        return;
    }

    if (0 != code)
    {
        QMessageBox::warning(nullptr, tr("出错"), tr("更新当前订单状态失败"));
        return;
    }
    updateOrders(_pager->curPage());

    gWnd->recordPOSEvent("setting:orders", QString("order id: %1, opteration: %2")
                                      .arg(ob.oid)
                                      .arg(index.column() == 4 ? "Finish": "Reject"));

}

void OrdersManagerFrame::rowSelected(const QModelIndex &index)
{
    if(index.row() == -1) {
        return;
    }

    OrderBean ob = _model->order(index);
    initOrderDetail(ob);
}
