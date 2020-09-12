#include "OrderData.h"
#include <QJsonDocument>

#include "datathread.h"

QString OrderType::TC = "TC";
QString OrderType::A = "A*";
QString OrderType::Q = "Q*";
QString OrderType::C = "C*";
QString OrderType::D = "*D";
QString OrderType::P = "*P";
QString OrderType::ALL = "AL";



/////////////////////////////////
/// \brief FoodBean::FoodBean
///
FoodBean::FoodBean() {
    this->id = 0;
    this->reid = -1;
    this->oid = 0;
    this->count = 0;
    this->unit_price = 0.0;
    this->discount = 0.0;

    this->cid = 0;
    this->rid = 0;
    this->icon = "";
}
FoodBean::~FoodBean() {

}
QVariantMap FoodBean::toMap() {
    QVariantMap map;
    map.insert("oid", this->oid);
    map.insert("fid", this->id);
    map.insert("count", this->count);
    map.insert("unit_price", this->unit_price);
    map.insert("discount", this->discount);
    map.insert("cid", this->cid);
    map.insert("rid", this->rid);
    map.insert("food_name", this->name);
    map.insert("flavor", this->flavor);

    return map;
}
OrderBean::OrderBean() {
    this->oid = 0;
    this->sid = "";
    this->time = "";
    this->amount = 0;
    this->orderid = "";
    this->status = 0;
    this->gratuity = 0.0;

    this->discount = 0.0;
    this->payType = 0;
    this->isSubOrder = false;
    this->parentOid = 0;
    this->creditType = "";
    this->addr = "";
    this->mark = "";

    this->name = DataThread::inst().getRestaurantInfo().value("name").toString();
    this->taxRate = DataThread::inst().getRestaurantInfo().value("taxRate").toDouble();
}

OrderBean::OrderBean(int oid,
                     const QString& sid,
                     const QString& time,
                     double amount,
                     const QString& orderid,
                     int status ) {
    this->oid = oid;
    this->sid = sid;
    this->time = time;
    this->amount = amount;
    this->orderid = orderid;
    this->status = status;
    this->gratuity = 0.0;
    this->discount = 0.0;
    this->payType = 0;
    this->isSubOrder = false;
    this->parentOid = 0;
    this->creditType = "";
    this->addr = "";
    this->mark = "";

    this->name = DataThread::inst().getRestaurantInfo().value("name").toString();
    this->taxRate = DataThread::inst().getRestaurantInfo().value("taxRate").toDouble();
}

OrderBean::OrderBean(const QVariantMap& order) {
    if(0 == order.count("id") ) throw QString("not found id");
    if(0 == order.count("sid") ) throw QString("not found sid");
    if(0 == order.count("time") ) throw QString("not found time");
    if(0 == order.count("orderid") ) throw QString("not found orderId");

    this->oid = order["id"].toInt();
    this->sid = order["sid"].toString();
    this->time = order["time"].toString();
    this->orderid = order["orderId"].toString();
    this->status = order["status"].toInt();
    this->gratuity = order["gratuity"].toDouble(); //0 == order.count("gratuity") ? 0 : order["gratuity"].toDouble();

    this->name = DataThread::inst().getRestaurantInfo().value("name").toString();
    this->taxRate = DataThread::inst().getRestaurantInfo().value("taxRate").toDouble();

    this->payType = order["payType"].toInt();
    this->isSubOrder = false;
    this->parentOid = 0;
    this->creditType = order["creditType"].toString();
    this->mark = order["mark"].toString();
    this->addr = "";
    if(order.count("addr") > 0) {
        this->addr = order["addr"].toString();
    }


    this->total = 0;
    this->amount = 0.0;

    this->discount = order["discount"].toDouble();
//    this->total = order["totalPrice"].toDouble();
//    this->amount = order["amount"].toDouble();

    this->cookingTime = order["cookingTime"].toString();
    this->finishTime = order["finishTime"].toString();

    foreach (QVariant food, order["foods"].toList()) {
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

        this->total += fb.count * fb.unit_price * (1 - fb.discount);

        qDebug() << "flavor: " << fb.flavor;
        this->foods.append(fb);
    }

    foreach( QVariant suboid, order["subOids"].toList()) {

        QVariantMap sub = suboid.toMap();
        this->subOids.append( sub["id"].toInt());
    }

//    this->amount = this->total * (1 - this->discount) + this->total * this->taxRate + this->gratuity;
}
OrderBean::~OrderBean() {

}

QJsonDocument OrderBean::toJsonDoc() {
    return QJsonDocument::fromVariant(toMap());
}
QVariantMap OrderBean::toMap() {
    QVariantMap map;
    map.insert("oid", this->oid);
    map.insert("sid", this->sid);
    map.insert("time", this->time);
    map.insert("orderid", this->orderid);
    map.insert("status", this->status);
    map.insert("gratuity", this->gratuity);
    map.insert("totalPrice", this->total);
    map.insert("discount", this->discount);
    map.insert("taxRate", this->taxRate);
    map.insert("amount", this->amount);
    map.insert("payType", this->payType);
    map.insert("creditType", this->creditType);
    map.insert("isSubOrder", this->isSubOrder);
    map.insert("parentOid", this->parentOid);
    map.insert("addr", this->addr);
    map.insert("cookingTime", this->cookingTime);
    map.insert("finishTime", this->finishTime);
    map.insert("mark", this->mark);

    map.insert("foods", QVariantList());

    QVariantList _l;
    foreach (FoodBean food, this->foods) {
        QVariantMap _m;
        _m.insert("name", food.name);
        _m.insert("count", food.count);
        _m.insert("price", food.unit_price);
        _m.insert("flavor", food.flavor.toUtf8().toPercentEncoding());
        _m.insert("id", food.id);
        _m.insert("reid", food.reid);
        _m.insert("rid" , food.rid);
        _m.insert("cid", food.cid);
        _m.insert("discount", food.discount);

        _l.push_back(_m);
    }
    map.insert("foods", _l);

    return map;
}




////////////////////////////////////////////////////////
/// \brief OrdersModel::orderStatus
/// \param status
/// \return
///


QString OrdersModel::orderStatus(int status) const {
   switch( status ) {
       case POSDATA::OD_APPSUMIT: return QObject::tr("排队");
       case POSDATA::OD_RECVREQ: return QObject::tr("网络下单中");
       case POSDATA::OD_ACCESS: return QObject::tr("本地下单中");
       case POSDATA::OD_SUBMIT: return QObject::tr("已下单");
       case POSDATA::OD_COOKING: return QObject::tr("已下厨");
       case POSDATA::OD_REJECTOD: return QObject::tr("已取消");
       case POSDATA::OD_BOOKING_SHARE: return QObject::tr("已预约，未占位");
       case POSDATA::OD_BOOKING_TAKEUP: return QObject::tr("已预约，已占位");
       case POSDATA::OD_ODPAIED: return QObject::tr("已结算");
       case POSDATA::OD_FINISHORDER: return QObject::tr("完成");
       default: {
           qDebug() << "订单未知状态";
           return QObject::tr("未知状态");
       }
   }
}

OrdersModel::OrdersModel(QObject * parent) : QAbstractTableModel{parent} {}

int OrdersModel::rowCount(const QModelIndex &) const  { return m_data.count(); }
int OrdersModel::columnCount(const QModelIndex &) const  { return 5; }


QVariant OrdersModel::data(const QModelIndex &index, int role) const {
  if (role != Qt::DisplayRole && role != Qt::EditRole) return {};

  const auto & order = m_data[index.row()];
  switch (index.column()) {
      case 0: return index.row();
      case 1: return order.sid;
      case 2: return order.time;
      case 3: return "$" + QString::number(order.amount);
      case 4: return orderStatus(order.status);
      default: return {};
  }
}

QVariant OrdersModel::headerData(int section, Qt::Orientation orientation, int role) const {
  if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
  switch (section) {
  case 0: return "#";
  case 1: return QObject::tr("订单号");
  case 2: return QObject::tr("下单时间");
  case 3: return QObject::tr("总消费");
  case 4: return QObject::tr("状态");
  default: return {};
  }

}
void OrdersModel::append(const OrderBean & order) {
  beginInsertRows({}, m_data.count(), m_data.count());
  m_data.append(order);
  endInsertRows();
}

void OrdersModel::clean() {
   beginResetModel();
   m_data.erase(m_data.begin(), m_data.end());
   endResetModel();
}

OrderBean OrdersModel::order(const QModelIndex& index) {
   return m_data[index.row()];
}
