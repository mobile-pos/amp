#include "appordersframe.h"
#include "orderpainter.h"

#include <QDebug>
#include <QLabel>
#include <QMessageBox>

#include "TableviewEx/buttondelegate.h"
#include "datathread.h"

#include "mainwindow.h"

extern MainWindow* gWnd;


/////////////////////////////////////////////////////////

class OrderContact {
public:
    OrderContact() {
        _status = -1;
        _oid = 0;
    }
public:
    QString _sid;
    QString _contact;
    QString _time;
    QString _addr;

    int _status;
    int _oid;
};

class AppOrdersModel : public QAbstractTableModel {

   QList<OrderContact> m_data;

public:
   AppOrdersModel(QObject * parent = {}) : QAbstractTableModel{parent} {}

   int rowCount(const QModelIndex &) const override { return m_data.count(); }
   int columnCount(const QModelIndex &) const override { return 5; }


   QVariant data(const QModelIndex &index, int role) const override {
      if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
      const auto & order = m_data[index.row()];
      switch (index.column()) {
          case 0: return index.row();
          case 1: {
                  QString type = tr("线上");
                  if(order._sid.startsWith("TC-")) {
                       type = tr("堂吃");
                  }
                  return type + ":" + order._sid.mid(3) + "\n" + order._addr;
              }
          case 2: return QObject::tr("预览");
          case 3: return QObject::tr("接收");
          case 4: return QObject::tr("拒绝");
          default: return {};
      }
   }

   QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
      if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
      switch (section) {
      case 0: return "#";
      case 1: return QObject::tr("联系方式");
      case 2: return "";
      case 3: return "";
      case 4: return "";
      default: return {};
      }

   }
   int append(const OrderContact & order) {
      //判断是否重复
      bool exist = false;
      foreach(OrderContact oc, m_data ) {
          if(oc._oid == order._oid) {
              exist = true;
          }
      }
      if(false == exist) {
            beginInsertRows({}, m_data.count(), m_data.count());
            m_data.append(order);
            endInsertRows();

            return m_data.length();
      }

      return 0;
   }

   void clean() {
       beginResetModel();
       m_data.erase(m_data.begin(), m_data.end());
       endResetModel();
   }

   OrderContact order(const QModelIndex& index) {
       if( m_data.length() > index.row() ) {
            return m_data[index.row()];
       }
       return OrderContact();
   }

   void remove(const QModelIndex& index) {
       if( m_data.length() > index.row() ) {
            m_data.removeAt(index.row());
       }
   }
};



//////////////////////////////////////////////////////////////////
/// \brief AppOrdersDataThread::run
///
AppOrdersDataThread::AppOrdersDataThread(AppOrdersFrame* appFrm)
    :_appFrm(appFrm),
      _isRunning(false)
{

}
AppOrdersDataThread::~AppOrdersDataThread(){
    _appFrm = nullptr;
}

void AppOrdersDataThread::run() {
    _isRunning = true;

    while(_isRunning) {
        if( DataThread::inst().isLoggedIn() &&
                _appFrm->getModel()->rowCount(QModelIndex()) < 5 &&
                false == _appFrm->isHidden()) {
            //需要加载数据
            qDebug() << " load orders from cloud web server";
            _appFrm->updateOrders();
        }

        msleep(5000);
    }
    qDebug() << " app orders data thread is end now";

//    msleep(100);
}
//////////////////////////////////////////////////////////////////
/// \brief AppOrdersFrame::AppOrdersFrame
/// \param parent
///
AppOrdersFrame::AppOrdersFrame(QWidget *parent) :
    QFrame(parent),
    _dataThread(nullptr)
{

    this->setStyleSheet("background-color: #ffffff");

    _odTab = new QTableView(this);

    _model = new AppOrdersModel(this);

    _odTab->setSelectionMode(QAbstractItemView::SingleSelection);
    _odTab->setEditTriggers(QAbstractItemView::NoEditTriggers);
//    _odTab->setSelectionBehavior(QAbstractItemView::SelectRows);

    _odTab->setModel(_model);


    init();
}

AppOrdersFrame::~AppOrdersFrame()
{
    _dataThread->stop();

    if(nullptr != _model) {
        delete _model;
        _model = nullptr;
    }
}

void AppOrdersFrame::init() {
    qDebug() << "AppOrdersFrame::init";

    if(nullptr == _dataThread) {
        _dataThread = new AppOrdersDataThread(this);
        _dataThread->start();
    }

    qDebug() << "AppOrdersFrame::init end";

    retranslateUI();
}

void AppOrdersFrame::retranslateUI() {

    ButtonDelegate* _view = new ButtonDelegate(tr("预览"), dynamic_cast<QFrame*>(this));
    ButtonDelegate* _acc = new ButtonDelegate(tr("接收"), dynamic_cast<QFrame*>(this));
    ButtonDelegate* _rej = new ButtonDelegate(tr("拒绝"), dynamic_cast<QFrame*>(this));

    this->connect(_view, SIGNAL(onClicked(const QModelIndex&)), this, SLOT(onClicked(const QModelIndex&)));
    this->connect(_acc, SIGNAL(onClicked(const QModelIndex&)), this, SLOT(onClicked(const QModelIndex&)));
    this->connect(_rej, SIGNAL(onClicked(const QModelIndex&)), this, SLOT(onClicked(const QModelIndex&)));

    _odTab->setItemDelegateForColumn(2, _view);
    _odTab->setItemDelegateForColumn(3, _acc);
    _odTab->setItemDelegateForColumn(4, _rej);

//    this->connect(_odTab, SIGNAL(clicked(const QModelIndex &)), this, SLOT(onClicked(const QModelIndex &)));
}

void AppOrdersFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();

    //不会多次调用，内存消耗不大
    POSLabelEx* label = new POSLabelEx( QObject::tr("自助点餐申请"), this);
    label->setGeometry(0, 0, rect.width(), 30);
    _odTab->setGeometry(0, 30, rect.width(), rect.height() -  30);

    this->_odTab->setColumnWidth(0, 20);
    this->_odTab->setColumnWidth(2, 80);
    this->_odTab->setColumnWidth(3, 80);
    this->_odTab->setColumnWidth(4, 80);

    int w = 20 + 80 * 3;

    this->_odTab->setColumnWidth(1, _odTab->geometry().width() - w - 50);

}
void AppOrdersFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);
}

void AppOrdersFrame::onClicked(const QModelIndex &index) {

    qDebug() << " clicked : column: " << index.column() << ", row: " << index.row();

    OrderContact oc = _model->order(index);
    if(-1 == oc._status) {
        return;
    }

    //追加到本地数据库
    QVariantMap order = DataThread::inst().getOrderbyOidOnline(oc._oid);  //loadOrderByOId(oc._oid, DataThread::inst().cloudServer());
    if(order.empty()) {
        QMessageBox::warning(this, "出错", "加载云端订单失败");
        return;
    }
    //存入本地数据库
    OrderBean ob(order);
    ob.status = POSDATA::OD_RECVREQ;

    int code = -1;
    if(index.column() == _model->columnCount(index) - 2) //access
    {
        code = DataThread::inst().auditOrder(oc._oid, POSDATA::OD_ACCESS, DataThread::inst().cloudServer());

    } else if(index.column() == _model->columnCount(index) - 1 ){ // reject
        code = DataThread::inst().auditOrder(oc._oid, POSDATA::OD_REJECTOD, DataThread::inst().cloudServer());

    } else { //view
        //预览
        gWnd->openMenuFrame(ob);
        return;
    }

    if(code != 0) {
        QMessageBox::warning(this, "出错", "提交操作失败, 请稍后重试");
        return;
    }

    if(index.column() == _model->columnCount(index) - 2) //access
    {
        ob.status = POSDATA::OD_SUBMIT;

        ob.total = 0.0;
        for(int i = 0; i < ob.foods.length(); i++) {
            ob.foods[i].reid = -1;
            ob.total += ob.foods[i].unit_price * ob.foods[i].count;
        }
        ob.amount = ob.total * (1 + ob.taxRate);

        order = DataThread::inst().newOrder(ob.toMap());
        if(order.empty()) {
            QMessageBox::warning(this, "出错", "订单存入本地失败，联系客服");
            return;
        }

        try {
            //打印订单
            OrderPainter::Print(ob, 1, DataThread::inst().getCashierPrinter());
        } catch (...) {
            QMessageBox::warning(this, "出错", "打印订单失败,请检查打印机状态或打印机配置");
        }
    }

    if( index.column() >= _model->columnCount(index) - 2) {
        //清空本view
        _model->remove(index);
        _model->layoutChanged();
    }
}
void AppOrdersFrame::updateOrders() {
    QList<QVariant> orders = DataThread::inst().loadOnlineOrders();
    if(orders.empty()) {
        return;
    }

    foreach (QVariant order, orders) {
        QVariantMap item = order.toMap();
        OrderContact oc;
        oc._oid = item["id"].toInt();
        oc._sid = item["sid"].toString();
        oc._time = item["time"].toString();
        oc._status = item["status"].toInt();
        oc._addr = item["addr"].toString();

        _model->append(oc);
    }

    int rows = _model->rowCount(QModelIndex());
    for (int i = 0; i < rows; i++) {
        _odTab->setRowHeight(i, 50);
    }

    _odTab->updateGeometry();
    _odTab->showMaximized();
    _odTab->update();
}


void AppOrdersFrame::rowSelected(const QModelIndex &index)
{
//    index.row()
//    QMessageBox::warning(nullptr, "tr",  QString::number(index.column()));
}
