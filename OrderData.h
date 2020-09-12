#ifndef ORDERDATA_H
#define ORDERDATA_H

#include <QAbstractTableModel>
#include <QDebug>

namespace POSDATA {

enum OrderStatus {
    OD_UNKNOWN = -1, //未知
    OD_APPSUMIT = 0, //线上提交，需要申请
    OD_RECVREQ = 1, //店内收到请求
    OD_ACCESS = 2, //接收订单，店内操作时，直接这一步
    OD_BOOKING_SHARE = 3, //预约订单, 不占位
    OD_BOOKING_TAKEUP =  4, //预约订单，已占位
    OD_SUBMIT = 6, //确定下单
    OD_COOKING   = 8, //已下厨
    OD_REJECTOD = 10, //拒绝订单

    OD_ODPAIED   = 15, //订单已付
    OD_FINISHORDER = 16, //结算完成

    OD_ODUNPAIED = 17, //未支付类 // ! ( 32 - 15 )
    OD_UNCOOKING = 24, //未下厨   // ! ( 32 - 8 )
    OD_ALLSTATUS = 32 //所有状态  // 32 - 32 = 0  即 STATUS != 0

};

enum SatDownStatus {
    TAB_UNKNOWN = -1, //
    TAB_UNSITDOWN = 0,
    TAB_SATDOWN = 1,
    TAB_TEMP_TAKEUP = 2,
    TAB_BOOKING_SHARE = 5,  //预约占用，但在预约时间没到前，可以让其他人使用
    TAB_BOOKING_TAKEUP = 6, //预约占用，不让人使用

};

enum PAYTYPE {
    CASH = 0,
    CREDIT = 1, //信用卡支付
    WEIPAY = 2, // 微信支付
    ALIPAY = 3 // 支付宝 线上支付
};

} //END POSDATA

class OrderType{
public:
    OrderType(const QString& curType) : _curType(curType) {}
public:
    //点餐类型
    static QString TC;
    static QString A;
    static QString Q;
    static QString C;
    //收货方式
    static QString D;
    static QString P;

    static QString ALL;

public:
    QString curType() {
        return _curType;
    }
private:
    QString _curType;
};


class FoodBean {
public:
    FoodBean();
    ~FoodBean();

public:
    QVariantMap toMap();
public:
    int id; //商品id，静态id
    QString name; //商品名
//    QString enName; //商品英文名, 在接口中直接转换, 本地看到的是已切换后的语言
    int unit; //商品计量单位
    int count;  //数量
    double unit_price; //单价
    QString flavor; //口味项
    int oid; // order id;
    int reid; //购物车中的动态 id
    double discount; //单项菜品 打折

    int cid; //food catergory
    int rid ; //restaurant id
    QString code; // food code;
    int maxCount; // 当前总量

    //菜品样图路径
    QString icon;

    QString cnName;
    QString enName;
};

class OrderBean {
public:
    OrderBean(const QVariantMap& order);
    OrderBean(int oid, const QString& sid, const QString& time, double total, const QString& orderid, int status);
//    OrderBean(const OrderBean& ob);
    OrderBean();

    ~OrderBean();
public:
    int oid; //订单索引号
    QString sid;    //桌号，或是排队号
    QString time;   //提交时间
    QString orderid; //订单号
    QString name; //餐馆名

    QList<FoodBean> foods; //商品列表

    double gratuity; //小费
    double taxRate; //税率

    double total; //实物总价
    double amount; //应付总价： total * (1 + taxRate) * (1 - discount) + gratuity
    //
    int status; //订单状态 订单状态, 0: 排队,1: 接收,16:结单
    int payType;  // 0 现金，1 信用卡，2 移动支付
    QString creditType; //支付方式：visa, master, cash ...

    double discount; //整单打折
    QString mark;

    //排队数据，不入库
    int queueNumber; //前面排队数量

    bool isSubOrder; //当前订单是否为拆单后的订单
    int parentOid; //拆单后的父订单号，父订单为主要订单

    //同时也临时保存拆单后的子订单id号列表
    QList<int> subOids;

    //订单的送餐地址
    QString addr;

    //预约下厨时间
    QString cookingTime;
    QString finishTime; // payTime
    //服务员
    QString waiter;

    int takeType; // 取餐方式 1 deliver, 2 pick up

    QString creditID; //信用卡 ID
    QString creditValidity; //
public:
    inline QString getTabId() { return sid;}
    inline QString getTime() {return time;}
    inline QString getOrderID() { return orderid;}
    inline QList<FoodBean> getGoods() {return this->foods;}

    QJsonDocument toJsonDoc();
    QVariantMap toMap();
};

class OrdersModel : public QAbstractTableModel {
protected:
   QList<OrderBean> m_data;

protected:
   QString orderStatus(int status) const ;
public:
   OrdersModel(QObject * parent = {}) ;

   int rowCount(const QModelIndex &) const override ;
   int columnCount(const QModelIndex &) const override;


   QVariant data(const QModelIndex &index, int role) const override ;

   QVariant headerData(int section, Qt::Orientation orientation, int role) const override ;

   void append(const OrderBean & order) ;

   void clean() ;
   OrderBean order(const QModelIndex& index) ;
};



#endif // ORDERDATA_H
