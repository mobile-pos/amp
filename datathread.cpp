#include "datathread.h"
#include <iostream>
#include <sstream>

#include <QUrl>
#include <QDebug>
#include <QProcess>
#include <QJsonDocument>
#include <QMessageBox>
#include <QDateTime>


#include "orderpainter.h"
#include <curl/curl.h>


//#define LOCALWEBSERVER "http://127.0.0.1:8080"
//#define LOCALWEBSERVER "http://127.0.0.1:8080"
//#define CLOUDWEBSERVER "http://120.24.66.234:8080"
//#define CLOUDWEBSERVER "http://67.198.232.11:8080"

QString WORKDIR = "/opt/pos";

//店家初始化信息
QVariantMap gRestaurant;
QVariantMap gFlavors;
QList<QVariant> gTables;
int gRID = 0; //当前店家ID

QString _cashierPrinter; //前台打印机
QList<QString> _kitchPrinter; //厨房打印机

QString REDISSERVER ; //缓存地址，可以是本地，也可以是云端
int REDISPORT = 6379; // redis port
QString REDISAUTH = "pos-us#sz@2019"; // redis auth;

QString LOCALWEBSERVER; //本地服务地址，可以是本地，也可以是云端

// yum install gridsite-clients
// urlencode ...

DataThread::DataThread()
{
    _lang = "zh_CN";
}

DataThread& DataThread::inst() {
    static DataThread dt;
    static bool binit = false;
    if( false == binit) {

        curl_global_init(CURL_GLOBAL_ALL);

        dt.loadRestInfo();

        binit = true;
    }
    return dt;
}

QString DataThread::getLanguage() {
    return _lang;
}
QString DataThread::getCashierPrinter() {
    return _cashierPrinter;
}
QList<QString> DataThread::getKitchPrinter() {
    return _kitchPrinter;
}

QString DataThread::cloudServer() {
    return gRestaurant["cloudIP"].toString();
}

QString DataThread::cloudServerUrl() {
    if( gRestaurant.find("cloudIP") == gRestaurant.end()) {
        //第一次认证是 使用 国内服
//        gRestaurant["cloudIP"] = "cnshop.aiwaiter.net";
        return "http://cnshop.aiwaiter.net";
    }
#ifdef Q_OS_ANDROID
    return QString("http://%1").arg(gRestaurant["cloudIP"].toString());
#else
    return QString("https://%1").arg(gRestaurant["cloudIP"].toString());
#endif
}

void DataThread::stop() {
    m_isRunning = false;
}


QVariantMap DataThread::login(const QString& account, const QString& md5) {

    QString url = cloudServerUrl();
    url += QString("/index.php/pos/projs.oauth/login?account=")
                + account
                + "&md5=" + md5;

    qDebug() << "login url: " << url;
    QVariantMap user = this->curlGet( url );

    if(user.count("code") == 0) {
        //
        QVariantMap ret ;
        ret["code"] = 100;
        ret["msg"] = "no reponse";

        return ret;
    }
    else if(user["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << user["code"].toInt() << ", msg: " << user["msg"].toString();

        QVariantMap ret ;
        ret["code"] = user["code"];
        ret["msg"] = user["msg"];

        return ret;
    }

    qDebug() << "login result: " << user;

    QList<QVariant> data = user["data"].toList();
    if(data.length() == 0) {
        return QVariantMap();
    }
    //把token放到data下
    QVariantMap uinfo = data.at(0).toMap();
    uinfo.insert("token", user["token"].toString());

    if(gRestaurant.value("account").toString() == account) {
        gRestaurant["token"] = user["token"].toString();

        //更新redis
        QProcess::execute(QString("redis-cli hset res:info token %1").arg(
                              user["token"].toString()));
    }
    return uinfo;
}
void DataThread::setLogin(const QString& acc, QVariant info) {
    _users.insert(acc, info);

    _activeUser = acc;

    this->m_curUserInfo = info.toMap();
    this->m_curUserInfo.insert("rid", this->m_curUserInfo.value("id"));
}
bool DataThread::isAdminLoggedin() {
    if( false == isLoggedIn() || "" == _activeUser) {
        return false;
    }

    QVariantMap item = _users[_activeUser].toMap();

    return item["role"].toInt() == 30;
}

QMap<QString, QVariant> DataThread::getUser() {
    QMap<QString, QVariant> admin;

    foreach (QString acc, _users.keys()) {
        if( _users[acc].toMap()["role"] == 30 ) {
            admin.insert(acc, _users[acc] );
            return admin;
        }
    }
    return admin;
}
void DataThread::logout() {
    if(_activeUser == "") {
        return;
    }

    _users.remove(_activeUser);

    if( true == isLoggedIn() ) {
        _activeUser = _users.keys().at(0);
    } else {
        _activeUser = "";
    }

}

QString DataThread::getMenuIconPath() {
    return MENUICONPATH;
}

int DataThread::auditOrder(int oid, int status, const QString& server) {

    QString uurl = server;
    if(server == LOCALWEBSERVER) {
        uurl += QString("/index.php/local/api.orders/updateOrderByRestaurant?token=");
    } else {
        uurl += QString("/index.php/pos/api.orders/updateOrderByRestaurant?token=");
    }
    uurl += this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&oid=" + QString::number(oid)
            + "&status=" + QString::number(status);

    QVariantMap order = this->curlGet( uurl );

    if(order.count("code") == 0) {
        //
        return -1;
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return -2;
    }

    return order["code"].toInt();
}
QList<QVariant>  DataThread::loadOnlineOrders() {
    do {
        // @TODO test
        break;

        try {
            QString url = cloudServerUrl();
            url += "/index.php/pos/api.orders/getHistOrdersOnline?token="
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
//                    + "&type=" + "A*"
                    + "&status=" + QString::number(0)
                    + "&curpage=" + QString::number(0)
                    + "&pagesize=5";

            QVariantMap order = this->curlGet( url);
            qDebug() << "curl req: " << url;
            qDebug() << "curl res: " << order;

            if(order.count("code") == 0) {
                //
                break;
            }
            else if(order["code"].toInt() != 0) {
                //异常返回值，需要提示前端吗
                qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
                break;
            }

            return order["data"].toList();

        } catch(QString& err) {

            qDebug() << "catch error: " << err;

        } catch( ... ) {
            qDebug() << "catch unknown error";
        }
    }while(false);

    return QList<QVariant>();
}

QList<QVariant> DataThread::loadSoldoutMenus() {

    QString url = LOCALWEBSERVER + QString("/index.php/local/api.goods4svr/loadSoldoutMenus?token=")
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&language=" + this->getLanguage();

    QVariantMap menus = this->curlGet( url);
    if(menus.count("code") == 0) {
        //
        return QList<QVariant>();
    }
    else if(menus["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << menus["code"].toInt() << ", msg: " << menus["msg"].toString();
        return QList<QVariant>();
    }

    return menus["data"].toList();
}
int DataThread::updateMenuSoldout(int fid, int maxcount) {
    QString url = LOCALWEBSERVER + QString("/index.php/local/api.goods4svr/modifyMenuSoldout?token=")
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&fid=" + QString::number(fid)
            + "&maxcount=" + QString::number(maxcount);

    QVariantMap ret = this->curlGet( url);
    if(ret.count("code") == 0) {
        //
        return -2;
    }

    return ret["code"].toInt() ;

}
int DataThread::updateMenuPrice(int fid, double oldprice, double newprice) {
    QString url = LOCALWEBSERVER + QString("/index.php/local/api.goods4svr/modifyMenuPrice?token=")
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&fid=" + QString::number(fid)
            + "&oldprice=" + QString::number(oldprice)
            + "&price=" + QString::number(newprice);

    QVariantMap ret = this->curlGet( url);
    if(ret.count("code") == 0) {
        //
        return -2;
    }

    return ret["code"].toInt() ;

}
QList<QVariant> DataThread::loadSpecialMenus() {

    QString url = LOCALWEBSERVER + QString("/index.php/local/api.goods4svr/loadSpecialMenus?token=")
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&language=" + this->getLanguage();

    QVariantMap menus = this->curlGet( url);
    if(menus.count("code") == 0) {
        //
        return QList<QVariant>();
    }
    else if(menus["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << menus["code"].toInt() << ", msg: " << menus["msg"].toString();
        return QList<QVariant>();
    }

    return menus["data"].toList();
}

QList<QVariant> DataThread::loadRecommendationMenus() {
    QString url = LOCALWEBSERVER + QString("/index.php/local/api.goods4svr/loadRecommendationMenus?token=")
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&language=" + this->getLanguage();

    QVariantMap menus = this->curlGet( url);
    if(menus.count("code") == 0) {
        //
        return QList<QVariant>();
    }
    else if(menus["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << menus["code"].toInt() << ", msg: " << menus["msg"].toString();
        return QList<QVariant>();
    }

    return menus["data"].toList();
}
int DataThread::updateMenuRecommendation(int fid, bool isRecommendation) {
    QString url = LOCALWEBSERVER + QString("/index.php/local/api.goods4svr/updateMenuRecommendation?token=")
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&language=" + this->getLanguage()
            + "&fid=" + QString::number(fid)
            + "&recommendation=" + QString::number(isRecommendation);

    QVariantMap ret = this->curlGet( url);
    if(ret.count("code") == 0) {
        //
        return -1;
    }

    return ret["code"].toInt();
}


QVariantMap DataThread::loadMenuByfoodId(int fid) {
    QString url = LOCALWEBSERVER + QString("/index.php/local/api.goods4svr/loadMenubyFoodId?token=")
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&language=" + this->getLanguage()
            + "&fid=" + QString::number(fid);

    QVariantMap item = this->curlGet( url);
    if(item.count("code") == 0) {
        //
        return QVariantMap();
    }
    else if(item["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << item["code"].toInt() << ", msg: " << item["msg"].toString();
        return QVariantMap();
    }

    return item["data"].toMap();
}

void DataThread::checkLocalOrder() {
    do {

        //把订单打印到前台页面上显示
        // @TODO test
        break;

        try {

            QString url = LOCALWEBSERVER + QString("/index.php/local/api.orders/getOrder?token=") + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&language=" + this->getLanguage();

            QVariantMap order = this->curlGet( url);
//            qDebug() << "curl req: " << url;
//            qDebug() << "curl res: " << order;

            if(order.count("code") == 0) {
                //
                break;
            }
            else if(order["code"].toInt() != 0) {
                //异常返回值，需要提示前端吗
                qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
                break;
            }

            QVariantMap data = order["data"].toMap();

            QString uurl = LOCALWEBSERVER + QString("/index.php/local/api.orders/updateOrderByRestaurant?token=")
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&oid=" + data["id"].toString()
                    + "&status=2";

            QVariantMap ret = this->curlGet( uurl );

            if(ret.count("code") == 0) {
                //
                break;
            }
            else if(ret["code"].toInt() != 0) {
                //异常返回值，需要提示前端吗
                qDebug() << "update order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
                break;
            }


            OrderBean rb(order["data"].toMap());
            OrderPainter::Print(rb, 1, this->getCashierPrinter());

        } catch(QString& err) {

            qDebug() << "catch error: " << err;

        } catch( ... ) {
            qDebug() << "catch unknown error";
        }
    }while(false);
}
void DataThread::run() {

    if( gRestaurant.count("rid") == 0
            || gRestaurant.count("uuid") == 0) {

        qDebug() << "not load restaurant information, such as rid, name, uuid, token, taxRate ..";
        sleep(1.0);
//        return;
    }


    m_isRunning = true;

    while(m_isRunning) {
        //每隔一段时间同数据数据到本地

        msleep(5000);

        if(this->isLoggedIn()) {
            // 店内的pad订单，则直接打印，不需要
            this->checkLocalOrder();
        }
    }
}



size_t write_data(void *ptr, size_t size, size_t nmemb, void *stream)
{
    std::string data((const char*) ptr, (size_t) size * nmemb);

    *((std::stringstream*) stream) << data << std::endl;

    return size * nmemb;
}
/**
 * @brief 执行 HTTP GET 请求
 * @param url  请求的 URL
 * @param code 请求返回的状态码的指针
 * @param headers 请求头
 * @return 请求执行成功时返回响应的字符串，失败则返回空字符串，请求是否执行成功可以通过 code 进行判断
 */
QVariantMap DataThread::curlGet(const QString& url) {
    std::string response;

    CURL* curl = curl_easy_init();

    std::stringstream out;
    curl_easy_setopt(curl,CURLOPT_URL,url.toLatin1().data());
//    curl_easy_setopt(curl,CURLOPT_TRANSFERTEXT,1);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curl,CURLOPT_TIMEOUT_MS, 5000);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    CURLcode ret = curl_easy_perform(curl);

     curl_easy_cleanup(curl);
     if (ret != CURLE_OK) {
         fprintf(stderr, "curl_easy_perform() failed:url: %s,  %s\n", url.toStdString().c_str(), curl_easy_strerror(ret));
         return QVariantMap();
     }

     qDebug() << " curl " << url << ", res: " << out.str().c_str() ;
     QString qstr(out.str().c_str());

     QJsonDocument jdoc = QJsonDocument::fromJson( qstr.toLocal8Bit());
     if(jdoc.isNull() ) {

         return QVariantMap();
     }
    return jdoc.toVariant().toMap();
}

QVariantMap DataThread::curlPost(const QString& url, const QString param) {
    std::string response;

    std::string arr = param.toStdString();

    CURL* curl = curl_easy_init();
    curl_easy_setopt(curl, CURLOPT_POST, 1); //作用POST

    std::stringstream out;
    curl_easy_setopt(curl,CURLOPT_URL,url.toLatin1().data());
//    curl_easy_setopt(curl,CURLOPT_TRANSFERTEXT,1);
    curl_easy_setopt(curl,CURLOPT_VERBOSE,1);
    curl_easy_setopt(curl,CURLOPT_TIMEOUT_MS, 5000);
    curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, write_data);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, &out);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0);
    curl_easy_setopt(curl, CURLOPT_POSTFIELDS, arr.data());    // 指定post内容，也可以是arr.c_str()
    curl_easy_setopt(curl, CURLOPT_POSTFIELDSIZE, arr.size());
    CURLcode ret = curl_easy_perform(curl);

     curl_easy_cleanup(curl);
     if (ret != CURLE_OK) {
         fprintf(stderr, "curl_easy_perform() failed: %s\n", curl_easy_strerror(ret));
         return QVariantMap();
     }

     qDebug() << " curl " << url << ", res: " << out.str().c_str() ;
     QString qstr(out.str().c_str());

     QJsonDocument jdoc = QJsonDocument::fromJson( qstr.toLocal8Bit());
     if(jdoc.isNull() ) {

         return QVariantMap();
     }
    return jdoc.toVariant().toMap();
}


int DataThread::CallScript(const QString& cmd, QString& out, QString& err, int timeout) {

    QProcess process;
    process.start(cmd, QIODevice::ReadWrite);
    if(false == process.waitForStarted(timeout * 1000)) {
        return -1;
    }

    process.waitForFinished(timeout * 1000);

    // Continue reading the data until EOF reached
    out = process.readAllStandardOutput();
    err = process.readAllStandardError();

    return process.exitCode();
}

QVariantMap DataThread::loadRestInfo() {
    //通过控制台输出获取数据 redis-cli
//    QProcess *proc = new QProcess();
//    proc->start(REDISBIN + " hgetall res:info", QIODevice::ReadWrite);
//    // Wait for it to start
//    proc->waitForFinished(5 * 1000);
//    // Continue reading the data until EOF reached
//    QString out = proc->readAllStandardOutput();
//    QString err = proc->readAllStandardError();
//    qDebug() << "redis res:info: " << out;
//    qDebug() << "redis err: " << err;

//    QString out, err;
////    if(-1 == CallScript(REDISBIN + " hgetall res:info", out, err)) {
////        QMessageBox::warning(nullptr, "错误", "Redis 启动失败: " + err);
////        return QVariantMap();
////    }

//    QStringList arr = out.split(QChar('\n'));
//    if(arr.length() < 10 ) {
//        QMessageBox::warning(nullptr, "错误", "店家数据格式非法: " + err);
//        qDebug() << "需要有 rid,name,uuid,token,taxRate";
//        return QVariantMap();
//    }

//    gRestaurant.insert("rid", arr[ arr.indexOf("rid") + 1 ]);
//    gRestaurant.insert("id", arr[ arr.indexOf("rid") + 1 ]);
//    gRestaurant.insert("account", arr[ arr.indexOf("account") + 1 ]);
//    gRestaurant.insert("name", arr[ arr.indexOf("name") + 1 ]);
//    gRestaurant.insert("uuid", arr[ arr.indexOf("uuid") + 1]);
//    gRestaurant.insert("token", arr[ arr.indexOf("token") + 1]);
//    gRestaurant.insert("taxRate", arr[ arr.indexOf("taxRate") + 1]);

//    gRestaurant.insert("address", arr[ arr.indexOf("address") + 1]);
//    gRestaurant.insert("phoneNumbers", arr[ arr.indexOf("phoneNumbers") + 1]);

//    gRestaurant.insert("cloudIP", arr[ arr.indexOf("cloudIP") + 1]);
    return gRestaurant;
}

QVariantMap DataThread::loadPrinterInfo() {

    if("" == _cashierPrinter) {
        QMessageBox::warning(nullptr, "错误", "前台打印机设置无效");
    }


    QVariantMap printers;

    for (int i = 0; i < _kitchPrinter.length(); i++) {
        QString pn = _kitchPrinter[i].trimmed();
        printers.insert(pn, "kitchen");
    }
    printers.insert(_cashierPrinter, "cashier");

    return printers;
}

QVariantMap DataThread::loadFlavors() {

    //读取配置文件 json数据文件
//    QFile loadFile(DATAPATH + QString("/flavors.conf"));
//    if(!loadFile.open(QIODevice::ReadOnly))
//    {
//        QMessageBox::warning(nullptr, "错误", "无法加载口味设置配置文件 ");
//        qDebug() << "could't open projects json";
//        return QVariantMap();
//    }
//    QByteArray allData = loadFile.readAll();
//    loadFile.close();

//    QJsonParseError json_error;
//    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
//    if(json_error.error != QJsonParseError::NoError)
//    {
//        QMessageBox::warning(nullptr, "错误", "无法加载口味设置配置文件，配置文件格式不合法");
//        qDebug() << "json error!";
//        return QVariantMap();
//    }

//    return jsonDoc.toVariant().toMap();

    return gFlavors;
}
int DataThread::addReceiptToOrder(int oid, const QVariantMap& receipt) {

    QString url = LOCALWEBSERVER ;
    url += "/index.php/local/term.orders/addReceiptToOrder?token="
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&oid=" + QString::number(oid)
                    + "&food=" + QJsonDocument::fromVariant(receipt).toJson().toPercentEncoding();

    QVariantMap order = curlGet(url);

    if(order.count("code") == 0) {
        //
        return -255;
    } else if( order["code"].toInt() < 0) {
        return order["code"].toInt();
    }

    return order["reid"].toInt();
}

int DataThread::removeReceiptfromOrder(int oid, int receiptid) {

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/term.orders/removeReceiptfromOrder?token="
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&oid=" + QString::number(oid)
                    + "&receiptid=" + QString::number(receiptid);

    QVariantMap order = curlGet(url);

    if(order.count("code") == 0) {
        //
        return -255;
    }

    return order["code"].toInt();
}

QList<QVariant> DataThread::loadOrders(int& total,
                                       const QString& type,
                                       QString statuses,
                                       int curpage,
                                       int days) {

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.orders/getHistOrders?token="
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&type=" + type
                    + "&status=" + statuses
                    + "&days=" + QString::number(days)
                    + "&curpage=" + QString::number(curpage)
                    + "&pagesize=10";

    QVariantMap order = curlGet(url);

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QList<QVariant>();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QList<QVariant>();
    }

    total = order["totalResult"].toInt();

    return order["data"].toList();
}

QList<QVariant> DataThread::loadTables() {

//    QFile loadFile(DATAPATH + QString("/tables.conf"));
//    if(!loadFile.open(QIODevice::ReadOnly))
//    {
//        QMessageBox::warning(nullptr, "错误", "无法加载桌台设置配置文件 ");
//        qDebug() << "could't open projects json";
//        return QList<QVariant>();
//    }
//    QByteArray allData = loadFile.readAll();
//    loadFile.close();

//    QJsonParseError json_error;
//    QJsonDocument jsonDoc(QJsonDocument::fromJson(allData, &json_error));
//    if(json_error.error != QJsonParseError::NoError)
//    {
//        QMessageBox::warning(nullptr, "错误", "无法加载桌台设置配置文件，配置文件格式不合法");
//        qDebug() << "json error!";
//        return QList<QVariant>();
//    }
//    return jsonDoc.toVariant().toList();

    return gTables;
}
QVariantMap DataThread::loadOrderByOId(int orderId, const QString& server) {
    QString url = server;

    if(server == LOCALWEBSERVER) {
        url += "/index.php/local/api.orders/getOrder?token=";
    } else {
        url += "/index.php/pos/api.orders/getOrder?token=";
    }
    url += this->m_curUserInfo.value("token").toString()
        + "&account=" + this->m_curUserInfo.value("account").toString()
        + "&language=" + this->getLanguage()
        + "&rid=" + this->m_curUserInfo.value("id").toString()
        + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
        + "&oid=" + QString::number(orderId);

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QVariantMap();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QVariantMap();
    }

    return order["data"].toMap();
}
QVariantMap DataThread::getOrderbyOidOnline(int orderId) {
    QString url = this->cloudServerUrl();
    url += "/index.php/pos/api.orders/getOrderbyOidOnline?token="
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&oid=" + QString::number(orderId);

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QVariantMap();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QVariantMap();
    }

    return order["data"].toMap();
}
QVariantMap DataThread::loadUnpayOrderBySid(const QString& sid, int status, const QString& server) {

    QString url = server;
    if(server == LOCALWEBSERVER) {
        url += "/index.php/local/api.orders/getUnpayOrderbySid?token=";
    } else {
        url += "/index.php/pos/api.orders/getUnpayOrderbySid?token=";
    }

    url += this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&sid=" + sid
                    + "&status=" + QString::number(status);

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QVariantMap();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QVariantMap();
    }

    return order["data"].toMap();
}
QVariantMap DataThread::loadUnpaiedOrderBytname(const QString& tabname, int status, const QString& server) {

    QString url = server;
    if(server == LOCALWEBSERVER) {
        url += "/index.php/local/api.orders/getUnpayOrderbySid?token=";
    } else {
        url += "/index.php/pos/api.orders/getUnpayOrderbySid?token=";
    }

    url += this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&tabname=" + tabname
                    + "&status=" + QString::number(status);

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QVariantMap();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QVariantMap();
    }

    return order["data"].toMap();
}


QList<QVariant> DataThread::loadReceiptsByOrderId(int oid) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.orders/getReceiptByOrder?token="
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&oid=" + QString::number(oid);

    QVariantMap receipt = curlGet(url);

    if( receipt.empty() ) {
        return QList<QVariant>();
    }

    if(receipt.count("code") == 0 || receipt.count("foods") == 0) {
        //
        return QList<QVariant>();
    }
    else if(receipt["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << receipt["code"].toInt() << ", msg: " << receipt["msg"].toString();
        return QList<QVariant>();
    }
    else if( receipt["foods"].toList().length() == 0) {
        return QList<QVariant>();
    }

    return receipt["foods"].toList();
}

QVariantMap DataThread::loadCategories() {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.goods4svr/loadCategories?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString();
    QVariantMap receipt = curlGet(url);

    if( receipt.empty() ) {
        return QVariantMap();
    }

    if(receipt.count("code") == 0 || receipt.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(receipt["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << receipt["code"].toInt() << ", msg: " << receipt["msg"].toString();
        return QVariantMap();
    }

    return receipt["data"].toMap();
}
QList<QVariant> DataThread::loadMenuItemsByCategoryId(int cid) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.goods4svr/loadMenusByCategoryId?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&cid=" + QString::number(cid);

    QVariantMap menus = curlGet(url);

    if( menus.empty() ) {
        return QList<QVariant>();
    }

    if(menus.count("code") == 0 || menus.count("data") == 0) {
        //
        return QList<QVariant>();
    }
    else if(menus["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << menus["code"].toInt() << ", msg: " << menus["msg"].toString();
        return QList<QVariant>();
    }

    return menus["data"].toList();
}

QByteArray DataThread::map2json(QVariantMap map)  {
    QJsonDocument jdoc = QJsonDocument::fromVariant(map);
    if( jdoc.isNull() ) {
        return QJsonDocument().toJson();
    }

    return jdoc.toJson();
}

QString map2Param(QVariantMap map) {
    QString p = "";
    foreach(QString k, map.keys()) {
        if("addr" == k) {
            p += "&" + k + "=" + map[k].toByteArray().toPercentEncoding();
        } else {
            p += "&" + k + "=" + map[k].toString();
        }
    }

    return p;
}


QVariantMap DataThread::newOrder(const QVariantMap& od) {

    QVariantList foods = od["foods"].toList();
    if(foods.length() == 0) {
        qDebug() << "no food to select";
        return QVariantMap();
    }
    QJsonDocument jdoc = QJsonDocument::fromVariant(foods);

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/term.Orders/newOrder?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&sid=" + od["sid"].toString()
            ;

    qDebug() << "new order: " << url;

    QString param = "order=" + jdoc.toJson().toPercentEncoding()
            + map2Param(od);

//    QVariantMap order = curlGet(url);
    QVariantMap order = curlPost(url, param);

    if( order.empty() ) {
        return QVariantMap();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QVariantMap();
    }

    return order["data"].toMap();
}

QList<QVariant> DataThread::newSubOrder(int oid, int count){
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/term.Orders/newSubOrders?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&oid=" + QString::number(oid)
            + "&count=" + QString::number(count);

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QList<QVariant>();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QList<QVariant>();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QList<QVariant>();
    }

    return order["data"].toList();
}
QList<QVariant> DataThread::loadSubOrders(int oid) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.Orders/getSubOrders?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&oid=" + QString::number(oid);

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QList<QVariant>();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QList<QVariant>();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QList<QVariant>();
    }

    return order["data"].toList();
}

int DataThread::deleteSubOrder(int orderId, int subOrderId) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.orders/deleteSubOrder?token="
                    + this->m_curUserInfo.value("token").toString()
                    + "&account=" + this->m_curUserInfo.value("account").toString()
                    + "&language=" + this->getLanguage()
                    + "&rid=" + this->m_curUserInfo.value("id").toString()
                    + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
                    + "&suboid=" + QString::number(subOrderId)
                    + "&oid=" + QString::number(orderId);

    QVariantMap ret = curlGet(url);
    if(ret.count("code") == 0) {
        return -1;
    }

    return ret["code"].toInt();
}


QVariantMap DataThread::getQueueNumber() {

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/term.Orders/getQueueNo?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString();

    qDebug() << "new queue number: " << url;

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QVariantMap();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QVariantMap();
    }

    return order["data"].toMap();
}

QVariantMap  DataThread::updateOrderReceipts(const QVariantMap& od) {
//    QVariantList foods = od["foods"].toList();
    if(od.count("foods") == 0) {
        qDebug() << "no food to select";
        return QVariantMap();
    }
    QJsonDocument jdoc = QJsonDocument::fromVariant(od["foods"]);

    qDebug() << "foods: " << jdoc.toJson();

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/term.Orders/updateOrderReceipts?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&foods=" + jdoc.toJson().toPercentEncoding()
            + map2Param(od);

    qDebug() << "new order: " << url;

    QVariantMap order = curlGet(url);

    if( order.empty() ) {
        return QVariantMap();
    }

    if(order.count("code") == 0 || order.count("data") == 0) {
        //
        return QVariantMap();
    }
    else if(order["code"].toInt() != 0) {
        //异常返回值，需要提示前端吗
        qDebug() << "get order result: " << order["code"].toInt() << ", msg: " << order["msg"].toString();
        return QVariantMap();
    }

    return order["data"].toMap();
}

int DataThread::finishOrder(const QVariantMap& od) {

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/term.Orders/finishOrder?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + map2Param(od);

    qDebug() << "new order: " << url;

    QVariantMap order = curlGet(url);
    if( order.empty() ) {
        return -1;
    }

    if(order.count("code") == 0 ) {
        //
        return -1;
    }

    return order["code"].toInt();
}

int  DataThread::updateOrderInfo(const QVariantMap& od) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/term.Orders/updateOrderInfo?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + map2Param(od);

//    qDebug() << "new order: " << url;

    QVariantMap order = curlGet(url);
    if( order.empty() ) {
        return -1;
    }

    if(order.count("code") == 0 ) {
        //
        return -1;
    }

    return order["code"].toInt();
}
int DataThread::updateOrderStatusbysid(const QString& sid, int srStatus, int dstStatus) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.Orders/updateOrderStatusbysid?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&sid=" + sid
            + "&srStatus=" + QString::number(srStatus)
            + "&dstStatus=" + QString::number(dstStatus);

//    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);
    if( ret.count("code") == 0 ) {
        return -1;
    }

    return ret["code"].toInt();
}

int DataThread::updateOrderStatusbytname(const QString& tabname, int srStatus, int dstStatus) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.Orders/updateOrderStatusbytname?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&tabname=" + tabname
            + "&srStatus=" + QString::number(srStatus)
            + "&dstStatus=" + QString::number(dstStatus);

//    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);
    if( ret.count("code") == 0 ) {
        return -1;
    }

    return ret["code"].toInt();
}

QList<QVariant> DataThread::search4Foods(const QString& condition) {
    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.Goods4svr/search4Foods?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&language=" + this->getLanguage()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&cond=" + condition;

    QVariantMap order = curlGet(url);
    if( order.empty() ) {
        return QList<QVariant>();
    }

    if(order.count("code") == 0 || order["code"].toInt() != 0 ) {
        //
        return QList<QVariant>();
    }

    return order["data"].toList();
}

int DataThread::updateTables2Cloud() {
    QList<QVariant> tables = this->loadTables();

    QJsonDocument jdoc = QJsonDocument::fromVariant(tables);

    QString url = cloudServerUrl();
    url += "/index.php/pos/projs.tables/updateTables?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&tables=" + jdoc.toJson().toPercentEncoding();

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() ) {
        return -1;
    }

    return ret["code"].toInt();
}

int DataThread::updatePrinter2Cloud() {
    QVariantMap printer = this->loadPrinterInfo();
    QJsonDocument jdoc = QJsonDocument::fromVariant(printer);

    QString url = cloudServerUrl();
    url += "/index.php/pos/projs.tables/updatePrinters?token="
            + gRestaurant.value("token").toString()
            + "&account=" + gRestaurant.value("account").toString()
            + "&rid=" + gRestaurant.value("id").toString()
            + "&uuid=" + gRestaurant.value("uuid").toString()
            + "&printers=" + jdoc.toJson().toPercentEncoding();

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() ) {
        return -1;
    }

    return ret["code"].toInt();
}

QVariantMap DataThread::loadReportSalesTrend(int days) {

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.reports/getLastDaysSalesData?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&days=" + QString::number(days);

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() || ret["code"].toInt() != 0) {
        return QVariantMap();
    }

    return ret["data"].toMap();
}

QList<QVariant> DataThread::loadReportSalesIncome(int days) {

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.reports/getLastDaysIncome?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&days=" + QString::number(days);

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() || ret["code"].toInt() != 0) {
        return QList<QVariant>();
    }

    return ret["data"].toList();
}

QVariantMap DataThread::loadReportTopFoodsSaled(int days) {

    QString url = LOCALWEBSERVER;
    url += "/index.php/local/api.reports/getTopFoodSales?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&days=" + QString::number(days);

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() || ret["code"].toInt() != 0) {
        return QVariantMap();
    }

    return ret["data"].toMap();
}

QList<QVariant> DataThread::loadAddressByphone(const QString& phone) {

    QString url = cloudServerUrl();
    url += "/index.php/pos/projs.consumers/loadAddressByPhone?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&phone=" + phone;

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() || ret["code"].toInt() != 0) {
        return QList<QVariant>();
    }

    return ret["data"].toList();
}
int DataThread::saveConsumerAddress(const QString& phone, const QString& addr) {

    QVariantMap contact;
    contact.insert("name", phone);
    contact.insert("phone", phone);
    contact.insert("address", addr);
    QJsonDocument jdoc = QJsonDocument::fromVariant(contact);

    QString url = cloudServerUrl();
    url += "/index.php/pos/term.consumers/updateContact?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&contact=" + jdoc.toJson().toPercentEncoding();

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() || ret.count("code") == 0) {
        return -255;
    }

    return ret["code"].toInt();

}

QVariantMap DataThread::newPayOrder4LocalOrder(int orderId, QString sid, double amount) {

    QString url = cloudServerUrl();
    url += "/index.php/pos/term.orders/newPayOrder4LocalOrder?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&oid=" + QString::number(orderId)
            + "&sid=" + sid
            + "&amount=" + QString::number(amount);

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() || ret.count("code") == 0) {
        return QVariantMap();
    }

    return ret["data"].toMap();
}


QVariantMap DataThread::getPayStatus4LocalOrder(int orderId, QString payOrderId) {

    QString url = cloudServerUrl();
    url += "/index.php/pos/term.orders/getPayStatus4LocalOrder?token="
            + this->m_curUserInfo.value("token").toString()
            + "&account=" + this->m_curUserInfo.value("account").toString()
            + "&rid=" + this->m_curUserInfo.value("id").toString()
            + "&uuid=" + this->m_curUserInfo.value("uuid").toString()
            + "&oid=" + QString::number(orderId)
            + "&payOrderId=" + payOrderId;

    qDebug() << "new order: " << url;

    QVariantMap ret = curlGet(url);

    if( ret.empty() || ret.count("code") == 0) {
        return QVariantMap();
    }

    return ret["data"].toMap();
}

