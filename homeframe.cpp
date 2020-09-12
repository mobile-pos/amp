#include "homeframe.h"
#include <QLine>
#include <QDateTime>
#include <QMessageBox>

#include "datathread.h"

#include "mainwindow.h"
#include "vkeyboardex.h"

extern MainWindow* gWnd;

#define KEYEXPIRED "__keyevent@0__:expired"

#define OPENCASHBOXREQ "pos:cashbox:event:req:open"
#define CHECKCASHBOXREQ "pos:cashbox:event:req:check"
#define CLOSECASHBOXREQ "pos:cashbox:event:req:close"

#define OPENCASHBOXRSP "pos:cashbox:event:rsp:open"
#define CHECKCASHBOXRSP "pos:cashbox:event:rsp:check"
#define CLOSECASHBOXRSP "pos:cashbox:event:rsp:close"

//#"外部请求的提示"
#define NOTIFYWITHOUTCMD "notify:from:command"


//电话接入通知
#define CALLINNOFITY "notify:phone:is:callin"

HomeFrame::HomeFrame(QWidget *parent) :
    QFrame(parent)
  , m_tabFrame(nullptr)
  , m_orFrame(nullptr)
{

    this->setStyleSheet("background-color: rgb(224, 224, 224)");
    _leftlayout = new GridLayoutEx(6, this);

//    _leftlayout->setStyleSheet("background-color: rgb(224, 224, 224)");

    m_parentWnd = parent;

    qDebug() << "HomeFrame::init";
    initNavPane();

    initTablePane();
    initOrderPane();

    _rediscli = new RedisClient(this);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);

    qDebug() << "HomeFrame::init end";
}

HomeFrame::~HomeFrame()
{
}


void HomeFrame::resizeEvent(QResizeEvent *event) {

    QFrame::resizeEvent(event);
/**
page 1
+--------------------------+
|                          |
|   tables                 |
|                          |
+--------------------------+
|                          |
|   orders for audit       |
|          from online     |
|                          |
+--------------------------+


page 2
+--------------------------+
|                          |
|   orders for ready       |
+--------------------------+
 */
    QRect rect = this->frameGeometry();
    _leftlayout->setRowHeight(rect.height() / 25);
    _leftlayout->setGeometry(0, 0, 250, rect.height());

    QRect lrect = _leftlayout->geometry();

    m_tabFrame->setGeometry( lrect.right(), 0, rect.width() - lrect.width(), rect.height());
    m_orFrame->setGeometry( lrect.right(), 0, rect.width() - lrect.width() - 3, rect.height());
    _bookingFrame->setGeometry( lrect.right(), 0, rect.width() - lrect.width() - 3, rect.height());
}

void HomeFrame::showEvent(QShowEvent*e)  {
    QFrame::showEvent(e);

    this->showTabFrame();
}
void HomeFrame::initNavPane() {
    // 一行6个单元
    QString bgcolor = "background-color: rgb(131, 122, 122)";
    { //电话接入
        {
            POSLabelEx* _l = new POSLabelEx(QObject::tr("电话接入"), _leftlayout);

            _leftlayout->addWidget(_l, 6);
        }
        {
            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QObject::tr("线路1"));
            _b->setObjectName("CallInAtLine1");
            this->connect(_b, SIGNAL(released()), this, SLOT(onCallInLin1Clicked()));

            _leftlayout->addWidget(_b, 6, 2);
        }
        {
            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QObject::tr("线路2"));
            _b->setObjectName("CallInAtLine2");
            _leftlayout->addWidget(_b, 6, 2);
            this->connect(_b, SIGNAL(released()), this, SLOT(onCallInLin2Clicked()));
        }
    } {
        bgcolor = "background-color: rgb(102, 114, 122)";
        //外送， 排队
        {
            QLabel* _l = new QLabel("", _leftlayout);
            _leftlayout->addWidget(_l, 6);
        }
        {
            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QObject::tr("堂吃"));
            this->connect(_b, SIGNAL(released()), this, SLOT(on_m_tangshi_clicked()));
            _leftlayout->addWidget(_b, 2, 2);
        }
        {
            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QObject::tr("外送"));
            this->connect(_b, SIGNAL(released()), this, SLOT(on_m_delevery_clicked()));
            _leftlayout->addWidget(_b, 2, 2);
        }
        {
            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QObject::tr("外等"));
            this->connect(_b, SIGNAL(released()), this, SLOT(on_m_waideng_clicked()));
            _leftlayout->addWidget(_b, 2, 2);
        }
    } {
        bgcolor = "background-color: rgb(123, 119, 124)";
        {
            QLabel* _l = new QLabel("", _leftlayout);
            _leftlayout->addWidget(_l, 6);
        }
        {
            QLabel* _l = new POSLabelEx(QObject::tr("电话号"), _leftlayout);
            _leftlayout->addWidget(_l, 6);
        }
        {
            _callid = new QLineEdit(_leftlayout);

            _leftlayout->addWidget(_callid, 6);
        }
        //虚拟键盘
        for(int i = 0; i < 10; i++) {
            VKeyItemButton* _b = new VKeyItemButton(QString::number(i), this, _leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QString::number(i));
            _leftlayout->addWidget(_b, 2, 2);
        }
        {
            //删除
            VKeyItemButton* _b = new VKeyItemButton("Del", this, _leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QObject::tr("删除"));
            _leftlayout->addWidget(_b, 2, 2);
        }
        {
            //确定
            VKeyItemButton* _b = new VKeyItemButton("OK", this, _leftlayout);
            _b->setStyleSheet(bgcolor);
            _b->setText(QObject::tr("确定"));
            _leftlayout->addWidget(_b, 2, 2);
        }
    } {
        bgcolor = "background-color: rgb(89, 94, 85)";
//        {
//            QLabel* _l = new QLabel("", _leftlayout);
//            _leftlayout->addWidget(_l, 6);
//        }
//        {
//            POSLabelEx* _l = new POSLabelEx(QObject::tr("当日外卖订单"), _leftlayout);
//            _leftlayout->addWidget(_l, 6);
//        }
//        {
//            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
//            _b->setStyleSheet(bgcolor);
//            this->connect(_b, SIGNAL(released()), this, SLOT(on_deliveryUnpaiedOrders_clicked()));
//            _b->setText(QObject::tr("外卖未付"));
//            _leftlayout->addWidget(_b, 3, 2);
//        }
//        {
//            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
//            _b->setStyleSheet(bgcolor);
//            this->connect(_b, SIGNAL(released()), this, SLOT(on_deliveryPaiedOrders_clicked()));
//            _b->setText(QObject::tr("外卖已付"));
//            _leftlayout->addWidget(_b, 3, 2);
//        }
//        {
//            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
//            _b->setStyleSheet(bgcolor);
//            this->connect(_b, SIGNAL(released()), this, SLOT(on_pickupUnpaiedOrders_clicked()));
//            _b->setText(QObject::tr("外等未付"));
//            _leftlayout->addWidget(_b, 3, 2);
//        }
//        {
//            POSToolButtonEx* _b = new POSToolButtonEx(_leftlayout);
//            _b->setStyleSheet(bgcolor);
//            this->connect(_b, SIGNAL(released()), this, SLOT(on_pickupPaiedOrders_clicked()));
//            _b->setText(QObject::tr("外等已付"));
//            _leftlayout->addWidget(_b, 3, 2);
//        }
        {
            QToolButton* _b = new QToolButton(_leftlayout);
            this->connect(_b, SIGNAL(released()), this, SLOT(on_m_allUnpaiedOrders_clicked()));
            _b->setText("未付订单");
            _leftlayout->addWidget(_b, 3, 2);
        }
        {
            QToolButton* _b = new QToolButton(_leftlayout);
            this->connect(_b, SIGNAL(released()), this, SLOT(on_m_allOrders_clicked()));
            _b->setText("已付订单");
            _leftlayout->addWidget(_b, 3, 2);
        }
    }
}
void HomeFrame::initTablePane() {

    m_tabFrame = new TablesFrame(this);
    this->m_tabFrame->showMaximized();

}

void HomeFrame::retranslateUI() {
    m_tabFrame->init();


}


void HomeFrame::initOrderPane() {
    m_orFrame = new OrdersFrame(OrderType("AD"), QString::number(POSDATA::OD_ACCESS), this);
    this->m_orFrame->hide();

    _bookingFrame = new OrdersFrame(OrderType("ALL"), QString::number(POSDATA::OD_BOOKING_SHARE), this);
    _bookingFrame->hide();
}

void HomeFrame::showTabFrame() {
    this->m_orFrame->hide();
    _bookingFrame->hide();

    this->m_tabFrame->show();
    this->m_tabFrame->updateTables();
}

void HomeFrame::showOrderFrame() {
    this->m_tabFrame->hide();
    _bookingFrame->hide();

    this->m_orFrame->show();
    m_orFrame->updateOrders();
}

void HomeFrame::showBookingOrderFrame() {
    this->m_tabFrame->hide();
    m_orFrame->hide();

    _bookingFrame->show();
    _bookingFrame->updateOrders();
}


void HomeFrame::on_m_tangshi_clicked()
{
    this->showTabFrame();
}

void HomeFrame::on_m_allOrders_clicked()
{
    m_orFrame->setFilterType(OrderType::ALL);
    m_orFrame->setFilterStatus( QString::number(POSDATA::OD_ALLSTATUS));
    this->showOrderFrame();

}

void HomeFrame::on_pickupPaiedOrders_clicked()
{
    m_orFrame->setFilterType(OrderType::P);
    m_orFrame->setFilterStatus( QString("%1|%2|%3").arg(POSDATA::OD_ODPAIED).arg(POSDATA::OD_FINISHORDER).arg(POSDATA::OD_REJECTOD));
    this->showOrderFrame();
}

void HomeFrame::on_pickupUnpaiedOrders_clicked()
{
    m_orFrame->setFilterType(OrderType::P);
    m_orFrame->setFilterStatus( QString("%1|%2|%3").arg(POSDATA::OD_ACCESS).arg(POSDATA::OD_SUBMIT).arg(POSDATA::OD_COOKING));

    this->showOrderFrame();
}

void HomeFrame::on_deliveryUnpaiedOrders_clicked()
{
    m_orFrame->setFilterType(OrderType::D);
    m_orFrame->setFilterStatus( QString("%1|%2|%3").arg(POSDATA::OD_ACCESS).arg(POSDATA::OD_SUBMIT).arg(POSDATA::OD_COOKING));
    this->showOrderFrame();
}

void HomeFrame::on_deliveryPaiedOrders_clicked()
{
    m_orFrame->setFilterType(OrderType::D);
    m_orFrame->setFilterStatus( QString("%1|%2|%3").arg(POSDATA::OD_ODPAIED).arg(POSDATA::OD_FINISHORDER).arg(POSDATA::OD_REJECTOD));
    this->showOrderFrame();
}


void HomeFrame::on_m_onlineUnpaiedOrders_clicked()
{
//    m_orFrame->setFilterType("online-unpaied");
    m_orFrame->setFilterType(OrderType::A);
    m_orFrame->setFilterStatus( QString("%1|%2|%3").arg(POSDATA::OD_ACCESS).arg(POSDATA::OD_SUBMIT).arg(POSDATA::OD_COOKING));
    this->showOrderFrame();
}

void HomeFrame::on_m_onlinePaiedOrders_clicked()
{
//    m_orFrame->setFilterType("online-paied");
    m_orFrame->setFilterType(OrderType::A);
    m_orFrame->setFilterStatus( QString("%1|%2|%3").arg(POSDATA::OD_ODPAIED).arg(POSDATA::OD_FINISHORDER).arg(POSDATA::OD_REJECTOD));
    this->showOrderFrame();
}

void HomeFrame::hide() {
    QFrame::hide();

    m_orFrame->hide();
    m_tabFrame->hide();
}

void HomeFrame::on_m_waideng_clicked()
{

    this->hide();


    //初始化一个虚拟的号
    QVariantMap num = DataThread::inst().getQueueNumber();
    if(num.empty()) {
        //没有获取到排队号
        qDebug() << "没有获取到排队号";
        QMessageBox::warning(this, "出错", "没获取到排队号");
        return;
    }
    OrderBean ob(0, QString("QP-%1").arg( num["qno"].toInt(), 5, 10, QLatin1Char('0')),
                 QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"), 0.0, "0", 0);
    ob.addr = QObject::tr("当前排队人数") + ": " + num["count"].toString(); //num["count"].toInt();
    ob.queueNumber = num["count"].toInt();
    gWnd->openMenuFrame(ob);
}

void HomeFrame::on_m_delevery_clicked()
{

    this->hide();

    //初始化一个虚拟的号
    QVariantMap num = DataThread::inst().getQueueNumber();
    if(num.empty()) {
        //没有获取到排队号
        qDebug() << "没有获取到排队号";
        QMessageBox::warning(this, "出错", "没获取到排队号");
        return;
    }
    OrderBean ob(0, QString("QD-%1").arg("null"),
                 QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"), 0.0, "0", 0);
    ob.queueNumber = num["count"].toInt();
    gWnd->openMenuFrame(ob);
}

void HomeFrame::onKeyDown(const QString& value) {
    if("Del" == value) {
        std::string cid = _callid->text().toStdString();
        if(cid.length() == 0 ){
            return;
        }
        cid = cid.substr(0, cid.length() - 1);
        _callid->setText(cid.c_str());

    } else if("OK" == value) {

        OrderBean ob(0, "QP-" + _callid->text(), "", 0, "0", 0);
        gWnd->openMenuFrame(ob);

    } else {
        _callid->setText( _callid->text() + value);
    }
}

void HomeFrame::connected() {
    qDebug() << "HomeFrame::connected redis client connected: ";

    _rediscli->sendCommand(Command::Auth(REDISAUTH));
}
void HomeFrame::disconnected() {
    usleep(2 * 1000 * 1000);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);
}

void HomeFrame::onReply(const QString& cmd, Reply value) {

    if(cmd.toLower().startsWith("auth")) {

        _rediscli->sendCommand(Redis::Command::SUBSCRIBE({
                                                             KEYEXPIRED,
                                                             OPENCASHBOXRSP,
                                                             CHECKCASHBOXRSP,
                                                             CLOSECASHBOXRSP,
                                                             NOTIFYWITHOUTCMD,
                                                             CALLINNOFITY
                                                         }));
    }
}
void HomeFrame::messageReceived(const QString& channel, const QVariant& payload) {
    qDebug() << "HomeFrame::messageReceived redis client received: " << channel << ", value: " << payload;
    if(channel.startsWith(KEYEXPIRED)) {
        //过期，目前主要用于预约下厨时间
        QString key = payload.toString();
        //key = order:cookingtime:timeout:<sid>
        if( key.startsWith("order:cookingtime:timeout:")) {


            QString sid = key.mid( QString("order:cookingtime:timeout:").length() );

            QMessageBox::information(nullptr, tr("提示"), tr("预约时间到: ") + sid);

            qDebug() <<" cooking time is now: " << sid;
            //通过 sid 获取订单号，并打印订单
            QVariantMap o = DataThread::inst().loadUnpayOrderBySid(sid);
            if(o.empty()) {
                qDebug() <<" not found order by sid: " << sid;
                return;
            }

            QString tname = sid.mid(QString("TC-").length());

            _rediscli->sendCommand(Command::HSET("res:info:tables:" + tname, "satdown", "1"));
            _rediscli->sendCommand(Command::HDEL("res:info:tables:" + tname, "cookingTime"));

            //打印
            OrderBean rb(o);
            OrderPainter::Print(rb, 1, DataThread::inst().getCashierPrinter());
        } else if(key.startsWith("res:info:tables:")) {
            //桌子信息过期，需要重新加载
            QString tname =  key;
            tname = tname.replace(0, QString("res:info:tables:").length(), "");
            tname = tname.simplified();
            if(tname.length() > 0) {
                this->m_tabFrame->resetTable(tname);
            }
        }
    }
    else if (channel.startsWith(OPENCASHBOXRSP)) {
        //cash box open status
        if(payload.toString().length() > 1) {
            QMessageBox::warning(nullptr,
                             tr("Tips"),
                             QString("[open event] %1 ").arg(payload.toString()));
        } else {
            QMessageBox::warning(nullptr,
                             tr("Tips"),
                             QString("[open event] cashbox is %1")
                               .arg (payload.toInt() == 1 ? "opened" : "closed") );
        }

    } else if(channel.startsWith(CHECKCASHBOXRSP)) {

//        if(payload.toString().length() > 1) {
//            QMessageBox::warning(nullptr,
//                             tr("Tips"),
//                             QString("[check event] %1 ").arg(payload.toString()));
//        } else {
//            QMessageBox::warning(nullptr,
//                             tr("Tips"),
//                             QString("[check event]cash box is %1")
//                             .arg( payload.toInt( ) == 1 ? "opened" : "closed"));
//        }

    } else if(channel.startsWith(CLOSECASHBOXRSP)) {
        //cash box open status
        if(payload.toString().length() > 1) {
            QMessageBox::warning(nullptr,
                             tr("Tips"),
                             QString("[close event] %1 ").arg(payload.toString()));
        } else {
            QMessageBox::warning(nullptr,
                             tr("Tips"),
                                 QString("[close event] cashbox is %1")
                                   .arg (payload.toInt() == 1 ? "closed" : "opened") );
        }

    } else if(channel.startsWith(NOTIFYWITHOUTCMD)) {
        QMessageBox::warning(nullptr,
                         tr("Tips"),
                         QString("[notify event] %1 ").arg(payload.toString()));

    } else if(channel.startsWith(CALLINNOFITY)) {
       POSToolButtonEx* btn = dynamic_cast<POSToolButtonEx*>(
                   _leftlayout->getItembyObjectName("CallInAtLine1"));
       if(btn != nullptr) {
           btn->setText(payload.toString());
       }
    }
}

void HomeFrame::on_m_allOrderingOrders_clicked() {
    m_orFrame->setFilterType(OrderType::ALL);
    m_orFrame->setFilterStatus( QString("%1").arg(POSDATA::OD_ACCESS));
    this->showOrderFrame();
}

void HomeFrame::on_m_allUnpaiedOrders_clicked() {
    m_orFrame->setFilterType(OrderType::ALL);
    m_orFrame->setFilterStatus( QString("%1|%2").arg(POSDATA::OD_SUBMIT).arg(POSDATA::OD_COOKING));
    this->showOrderFrame();
}
void HomeFrame::on_m_allPaiedOrders_clicked() {
    m_orFrame->setFilterType(OrderType::ALL);
    m_orFrame->setFilterStatus( QString("%1|%2|%3").arg(POSDATA::OD_ODPAIED).arg(POSDATA::OD_FINISHORDER).arg(POSDATA::OD_REJECTOD));
    this->showOrderFrame();
}

void HomeFrame::onBookingUnpaiedOrdersClicked() {
    _bookingFrame->setFilterType(OrderType::ALL);
    _bookingFrame->setFilterStatus( QString("%1|%2").arg(POSDATA::OD_BOOKING_SHARE).arg(POSDATA::OD_BOOKING_TAKEUP));
    this->showBookingOrderFrame();
}

void HomeFrame::onCallInLin1Clicked() {
    POSToolButtonEx* btn = dynamic_cast<POSToolButtonEx*>(
                _leftlayout->getItembyObjectName("CallInAtLine1"));
    if(btn != nullptr && btn->text().startsWith(tr("线路")) == false) {

        //初始化一个虚拟的号
        QVariantMap num = DataThread::inst().getQueueNumber();
        if(num.empty()) {
            //没有获取到排队号
            qDebug() << "没有获取到排队号";
            QMessageBox::warning(this, "出错", "没获取到排队号");
            return;
        }
        OrderBean ob(0, QString("QD-%1").arg( btn->text() ),
                     QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"), 0.0, "0", 0);
        ob.queueNumber = num["count"].toInt();
        gWnd->openMenuFrame(ob);
    }
}
void HomeFrame::onCallInLin2Clicked() {
    POSToolButtonEx* btn = dynamic_cast<POSToolButtonEx*>(
                _leftlayout->getItembyObjectName("CallInAtLine2"));
    if(btn != nullptr && btn->text().startsWith(tr("线路")) == false) {

        //初始化一个虚拟的号
        QVariantMap num = DataThread::inst().getQueueNumber();
        if(num.empty()) {
            //没有获取到排队号
            qDebug() << "没有获取到排队号";
            QMessageBox::warning(this, "出错", "没获取到排队号");
            return;
        }
        OrderBean ob(0, QString("QD-%1").arg( btn->text() ),
                     QDateTime::currentDateTime().toString("yyyy.MM.dd hh:mm:ss"), 0.0, "0", 0);
        ob.queueNumber = num["count"].toInt();
        gWnd->openMenuFrame(ob);
    }

}
