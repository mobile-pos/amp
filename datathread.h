#ifndef DATATHREAD_H
#define DATATHREAD_H

#include <unistd.h>

#include <QThread>
#include <QList>
#include "OrderData.h"


#define MPOS_CLOUD 1

extern QVariantMap gRestaurant;
extern QVariantMap gFlavors;
extern QList<QVariant> gTables;
extern int gRID;

extern QString _cashierPrinter; //前台打印机
extern QList<QString> _kitchPrinter; //厨房打印机

#include "redis-cli/redisclient.h"

//#define REDISSERVER "cnshop.aiwaiter.net"
extern QString REDISSERVER; // "127.0.0.1"
extern int REDISPORT; // 6379
extern QString REDISAUTH; //
//采用云上服务器
extern QString LOCALWEBSERVER; // "https://cnshop.aiwaiter.net:443"

extern QString WORKDIR;

#define DATAPATH QString("%1/conf").arg(WORKDIR)

#define MENUICONPATH QString("%1/menu-icons").arg(WORKDIR)


class DataThread : public QThread
{
public:
    DataThread();

public:
    static DataThread& inst();
public:
    void setLanguage(const QString& lang) {
        _lang = lang;
    }
    QString getLanguage();
    //前台打印机，即默认打印
    QString getCashierPrinter();
    //厨房打印机,如果设置了多个打印机
    QList<QString> getKitchPrinter();

    void setLogin(const QString& acc, QVariant info);
    QMap<QString, QVariant> getUser();

    void logout();
    //是否已登录过
    inline bool isLoggedIn() {
        return _users.keys().length() > 0;
    }
    //店家管理是否登录了
    bool isAdminLoggedin();

    inline QString activeUserName() {
        return _activeUser;
    }
    inline QString restaurantName() {
        return gRestaurant["name"].toString();
    }

    //获取当前用户信息
    inline QMap<QString, QVariant> activeUserInfo() {
        return m_curUserInfo;
    }

    //获取菜品的图片的相对地址
    QString getMenuIconPath();

public:
    QVariantMap curlGet(const QString& url) ;
    QVariantMap curlPost(const QString& url, const QString param);

    QMap<QString, QVariant> getRestaurantInfo() { return gRestaurant;}

    //配置信息
    QString cloudServer();
    QString cloudServerUrl();
public:
    //系统登录
    QVariantMap login(const QString& acc, const QString& md5);
    //获取订单列表
    QList<QVariant> loadOrders(int& total,
                               const QString& type = "all",
                               QString statuses = "2",
                               int curpage = 0,
                               int days = 0);
    //根据 order id，获取当前订单信息
    QVariantMap loadOrderByOId(int orderId, const QString& server=LOCALWEBSERVER);
    //根据 sid 获取当前订单信息，只支持未支付，
    QVariantMap loadUnpayOrderBySid(const QString& sid, int status = POSDATA::OD_ACCESS, const QString& server=LOCALWEBSERVER);
    QVariantMap loadUnpaiedOrderBytname(const QString& tabname, int status = POSDATA::OD_ACCESS, const QString& server=LOCALWEBSERVER);

    QVariantMap getOrderbyOidOnline(int orderId);
    //获取每个订单的菜品详情
    QList<QVariant> loadReceiptsByOrderId(int oid);

    QVariantMap loadCategories();
    QList<QVariant> loadMenuItemsByCategoryId(int cid);

    //搜索菜品
    QList<QVariant> search4Foods(const QString& condition);

    // 查询已售罄的菜品
    QList<QVariant> loadSoldoutMenus();
    int updateMenuSoldout(int fid, int maxcount);
    //更新菜价，查询特价菜
    QList<QVariant> loadSpecialMenus();
    int updateMenuPrice(int fid, double oldprice, double newprice);
    //更新菜价 是否是店长推荐类
    QList<QVariant> loadRecommendationMenus();
    int updateMenuRecommendation(int fid, bool isRecommendation);


    QVariantMap loadMenuByfoodId(int fid);
    //加载桌号信息，包括桌号名称，桌号状态，是否占用，占用时间，时长
    QList<QVariant> loadTables();
    int setTableStatus(const int& tabId, bool satDown = false);

    //新订单
    QVariantMap newOrder(const QVariantMap& order);

    //删除折单子项
    int deleteSubOrder(int orderId, int subOrderId);

    //更新订单信息，主要是包括小费，支付方式
    int updateOrderInfo(const QVariantMap& order);

    //更新订单状态
    int updateOrderStatusbysid(const QString& sid, int srStatus, int dstStatus);
    int updateOrderStatusbytname(const QString& tabname, int srStatus, int dstStatus);
    //拆单后的子订单及库，并返回子订单列表
    ///
    /// \brief newSubOrder
    /// \param oid 主订单id
    /// \param count 分单的数量
    /// \return  分单后的所有子订单id列表
    ///
    QList<QVariant> newSubOrder(int oid, int count);
    QList<QVariant> loadSubOrders(int oid);
    //新增购物车商品
    QVariantMap updateOrderReceipts(const QVariantMap& order);
    //加载口味配置参数
    QVariantMap loadFlavors();
    //删除已下单中的菜品
    int removeReceiptfromOrder(int oid, int receitid);
    //向购物车填加菜品
    int addReceiptToOrder(int oid, const QVariantMap& receipt);
    //线上应用及审核
    QList<QVariant> loadOnlineOrders();
    int auditOrder(int oid, int status = 2, const QString& server=LOCALWEBSERVER); //默认是接收, status = 10为拒绝

    //获取排队号
    QVariantMap getQueueNumber();
    //结单
    int finishOrder(const QVariantMap& order);

    //将桌信息更新到云上
    int updateTables2Cloud();
    //将打印机同步到云上
    int updatePrinter2Cloud();

    //获取报告数据
    QVariantMap loadReportSalesTrend(int days);
    QList<QVariant> loadReportSalesIncome(int days);
    QVariantMap loadReportTopFoodsSaled(int days);


    //消费者
    QList<QVariant> loadAddressByphone(const QString& phone);
    //保存消费者地址
    int saveConsumerAddress(const QString& phone, const QString& addr);

    // 支付接口，1，生成支付二维码
    QVariantMap newPayOrder4LocalOrder(int orderId, QString sid, double amount);
    QVariantMap getPayStatus4LocalOrder(int orderId, QString payOrderId);
public:
    // utils
    static QByteArray map2json(QVariantMap map);
public:
    void stop();

    void checkLocalOrder();

public:
    // utils tools
    static int CallScript(const QString& cmd, QString& out, QString& err, int timeout = 5);
public:
    //读取店家信息
    QVariantMap loadRestInfo();
    //读取打印机信息
    QVariantMap loadPrinterInfo();

//    QVariantMap api(const QString& api);
private:
    void run();
private:
    bool m_isRunning;


private:
    QMap<QString, QVariant> m_curUserInfo; //店家信息

    QMap<QString, QVariant> _users; //登录的用户信息
    QString _activeUser;//当前活跃用户

    QString _lang; //当前语言


};

#endif // DATATHREAD_H
