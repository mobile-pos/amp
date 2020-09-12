#include "tablesframe.h"
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>
#include <QTimer>

#include "datathread.h"
#include "homeframe.h"
#include "mainwindow.h"
#include "OrderData.h"

extern MainWindow* gWnd;

/**
    开台时输入的人数
    */
KaiTaiDlg::KaiTaiDlg(QWidget* parent) : QFrame(parent), _bmergeTab(false)
{

    this->setObjectName("kaiTai");
    this->setStyleSheet("#kaiTai{ background: rgb(128, 185, 197);}");

    _layout = new GridLayoutEx(9, this);
    _layout->setObjectName("kaiTaiLayout");
    _layout->setStyleSheet("#kaiTai{ background: gray;}");

    _vkb = new VKeyboardEx(_layout);
    _vkb->init(3);

    // 1行5个单元
    {
        //显示桌号
        POSLabelEx* _l = new POSLabelEx(QObject::tr("已选桌台："), _layout);
//        _l->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        _l->setObjectName("selTabs");
        _l->setStyleSheet("#selTabs{ background: rgb(128, 185, 197);}");
        _layout->addWidget(_l, 8, 1, 10000);

        {
            VKeyItemButton* _btn = new VKeyItemButton("Merge", this, _layout);
            _btn->setStyleSheet("background-color: rgb(204, 204, 195)");
            _btn->setText("拼桌");
            _layout->addWidget(_btn, 1);
        }

    }
    //一整行
    {
        {
            QCheckBox* _cb = new QCheckBox("预约", _layout);
            _cb->setObjectName("BookingCB");
            _cb->setEnabled(false);

            _layout->addWidget(_cb, 2);
        }
        {
            POSLabelEx* _l = new POSLabelEx(QObject::tr("输入就餐人数："), _layout);
            _l->setAlignment(Qt::AlignRight | Qt::AlignCenter);
            _layout->addWidget(_l, 4);

            LineEditEx* _number = new LineEditEx(_layout);
            _number->setText("");
            _number->setObjectName("UserNumber");
            this->connect(_number, SIGNAL(focussed(QWidget*, bool)), this, SLOT(focussed(QWidget*, bool)));

            _vkb->setTarget(_number);
            _layout->addWidget(_number, 2);
        }
         _layout->addWidget(new QLabel("", _layout), 1);
    }

    //一部分 8列
    {
        _layout->addWidget(_vkb, 7, 6);
    }
    //
    {
        VKeyItemButton* _btn = new VKeyItemButton("OK", this, _layout);
        _btn->setStyleSheet("background-color: rgb(75, 88, 143)");
        _layout->addWidget(_btn, 2);
    }
    {
        _layout->addWidget(new QLabel("", _layout), 2);
    }
    {
        VKeyItemButton* _btn = new VKeyItemButton("Cancel", this, _layout);
        _btn->setStyleSheet("background-color: rgb(121, 135, 98)");
        _layout->addWidget(_btn, 2);
    }
    {
        _layout->addWidget(new QLabel("", _layout), 2);
    }
    {
        VKeyItemButton* _btn = new VKeyItemButton(tr("预约"), this, _layout);
        _btn->setObjectName("BookingBtn");
        _btn->setStyleSheet("background-color: rgb(75, 88, 143)");
        _layout->addWidget(_btn, 2);
    }

}
void KaiTaiDlg::addTab(const QString& tab) {
    if(_tabs.indexOf(tab) >= 0) {
        return;
    }

    TablesFrame* tb = dynamic_cast<TablesFrame*>(this->parentWidget());
    if( false == _bmergeTab ) {

        foreach(QString t, _tabs) {
            tb->_rediscli->sendCommand(Command::HSET("res:info:tables:" + t, "satdown", "0"));
            tb->_rediscli->sendCommand(Command::HSET("res:info:tables:" + t, "time", ""));
        }

        _tabs.clear();
    }

   _tabs.push_back(tab);

   POSLabelEx* _l = dynamic_cast<POSLabelEx*>( _layout->getItembyObjectName("selTabs") );

   int count = _tabs.length();
   _l->setText( QString("已选中台桌(%1)").arg(count));

   tb->_rediscli->sendCommand(Command::HSET("res:info:tables:" + tab, "satdown", "2"));
   tb->_rediscli->sendCommand(Command::HSET("res:info:tables:" + tab, "time", QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));

   QString title = "";
   foreach(QString t, _tabs) {
       if( title == "" ){
           title = t;
       } else {
            title += "-" + t;
       }
   }
   _l->setText( _l->text() + "-" + title);
}

void KaiTaiDlg::resizeEvent(QResizeEvent *e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();

    if( rect.height() > 40) {
        _layout->setRowHeight( rect.height() / 8); // 6行
        _layout->setGeometry(10, 10, rect.width() - 20, rect.height() - 20);
    }
}
void KaiTaiDlg::showEvent(QShowEvent *event) {
    QFrame::showEvent(event);


    POSLabelEx* _l = dynamic_cast<POSLabelEx*>( _layout->getItembyObjectName("selTabs") );
    LineEditEx* _le = dynamic_cast<LineEditEx*>( _layout->getItembyObjectName("UserNumber"));
    _le->setText( "" );

    QCheckBox* cb = dynamic_cast<QCheckBox*>(_layout->getItembyObjectName("BookingCB"));
    cb->setChecked(false);

    _bmergeTab = false;
    _layout->getItembyObjectName("Merge")->setStyleSheet("color: rgb(0, 32, 224)");
}

void KaiTaiDlg::onKeyDown(const QString& value) {
    qDebug() << "KaiTaiDlg::onKeyDown: " << value;


    POSLabelEx* _l = dynamic_cast<POSLabelEx*>( _layout->getItembyObjectName("selTabs"));
    LineEditEx* _le = dynamic_cast<LineEditEx*>( _layout->getItembyObjectName("UserNumber"));
    if( "OK" == value ) {
        if(_le->text().toInt() == 0) {
            _le->setPlaceholderText(QObject::tr("就餐人数不能为空"));
            return;
        }
        if(_tabs.length() == 0) {
            _l->setText(QObject::tr("未选择好台桌，请直接选择台桌"));
            return;
        }

        kaiTai();

        _tabs.clear();
        _l->setText(QObject::tr("已选中台桌："));
        _le->setText("1");

        this->hide();
    } else if("Cancel" == value) {

        TablesFrame* tb = dynamic_cast<TablesFrame*>(this->parentWidget());
        foreach(QString tab, _tabs) {
            tb->_rediscli->sendCommand(Command::HSET("res:info:tables:" + tab, "satdown", "0"));
            tb->_rediscli->sendCommand(Command::HSET("res:info:tables:" + tab, "time", ""));
        }

        //清空
        _tabs.clear();

        _l->setText(QObject::tr("已选中台桌："));
        _le->setText("1");
        this->hide();

    } else if("Merge" == value) {
        //拼桌
        _bmergeTab = !_bmergeTab;
        VKeyItemButton* kb = dynamic_cast<VKeyItemButton*>( _layout->getItembyObjectName("Merge") );

        if( _bmergeTab == true) {
            kb->setStyleSheet("color: rgb(224, 32, 0))");

        } else {
            kb->setStyleSheet("color: rgb(0, 32, 224)");

        }

        this->show();
    } else if("BookingBtn" == value) {
        QCheckBox* cb = dynamic_cast<QCheckBox*>(_layout->getItembyObjectName("BookingCB"));

        cb->setChecked( !cb->isChecked() );
    }

}

void KaiTaiDlg::onVKeyDown(const QString& value) {
    qDebug() << "KaiTaiDlg::onVKeyDown: " << value;

}

void KaiTaiDlg::kaiTai(){
    QString tabs = "";
    foreach(QString tab, _tabs) {
        if( tabs.length() == 0) {
            tabs += tab;
        } else {
            tabs += "-" + tab;
        }
    }
    OrderBean ob(0, QString("TC-%1").arg(tabs),
                 QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss"),
                 0.0,
                 "",
                 0);

    QCheckBox* cb = dynamic_cast<QCheckBox*>( _layout->getItembyObjectName("BookingCB"));
    if( cb->isChecked() ) {
        ob.status = POSDATA::OD_BOOKING_SHARE;
    }
    gWnd->openMenuFrame(ob);
}

//////////////////////////////////////////////////////////////////
/// \brief TabPushButton::TabPushButton
/// \param tname
/// \param target
/// \param parent
///
TabPushButton::TabPushButton(const QString& tname, TablesFrame* target, QWidget* parent)
    : QToolButton(parent)
    , m_pWnd(target)
{
    this->setObjectName(tname);
    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
}
void TabPushButton::onClicked() {
    //需要先确定这个台桌是否绑定了订单
    //获取订单号

    emit(onTabClicked(this->objectName()));
}



///////////////////////////////////////////////////
/// \brief TablesFrame::TablesFrame
/// \param parent
///
TablesFrame::TablesFrame(QWidget *parent) :
    QFrame(parent)
{
    qDebug() << "TablesFrame::init";
//    this->setStyleSheet("background-color: rgb(123, 127, 133)");

    _layout = new GridLayoutEx(8, this);
    _appFrm = new AppOrdersFrame(this);

    _rediscli = new RedisClient(this);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);

    _ktFrm = new KaiTaiDlg(this);
    _ktFrm->hide();

    init();
    qDebug() << "TablesFrame::init end";
}

TablesFrame::~TablesFrame()
{
    _appFrm = nullptr;
}

void TablesFrame::init() {

    _layout->clean();
    m_tables.clear();

    QList<QVariant> tables = DataThread::inst().loadTables();

    foreach (QVariant tab, tables) {
        QVariantMap item = tab.toMap();

        if( item.count("id") == 0
                || item.count("name") == 0
                || item.count("satdown") == 0) {
            continue;
        }
        updateTable(item);
    }
    _layout->resetPages();

    _appFrm->retranslateUI();

}
void TablesFrame::resizeEvent(QResizeEvent *e) {
    QFrame::resizeEvent(e);
    QRect rect = this->geometry();

    QRect glRect(0, 0, rect.width(), rect.height());


    int th = rect.height() * 0.6;
    if( rect.height() * 0.3 < 320) {
        th = rect.height() - 320;
    }

    _layout->setRowHeight(th / 5);
    _layout->setGeometry(0, 0, rect.width(), th);

    _appFrm->setGeometry(0, th, rect.width(), rect.height() - th);

    _ktFrm->setGeometry(rect.width() / 2 - 200, th, 400, 300);
}

void TablesFrame::showEvent(QShowEvent *e) {
    QFrame::showEvent(e);

    _rediscli->sendCommand(Redis::Command::KEYS("res:info:tables:" + QString::number(gRID) + ":*"));
}

void TablesFrame::updateTable(QVariantMap tab) {

    TableItem ti;
    if(tab.count("name") == 0) {
        return;
    }

    ti.name = tab["name"].toString();
    ti.satdown = tab["satdown"].toInt();
    ti.oid = tab["oid"].toInt();

   //新建一个button
   if( m_tables.count(ti.name) == 0
           || m_tables[ti.name].btn == nullptr) {

       ti.btn = new TabPushButton(ti.name, this, _layout);
       ti.btn->setText(ti.name);

       _layout->addWidget(ti.btn);
       this->connect(ti.btn, SIGNAL(onTabClicked(const QString&)), this, SLOT(onTabClicked(const QString&)));
       m_tables[ti.name] = ti;
   } else {
       ti.btn = m_tables[ti.name].btn;
   }

   int booking = 0;

   if( tab.count("cookingTime") > 0) {
       QDateTime cookingTime = QDateTime::fromString(tab["cookingTime"].toString(), "yyyy-MM-dd hh:mm:ss");
       QDateTime cur_time = QDateTime::currentDateTime();

       if(cookingTime.isValid()) {
           if( cur_time.secsTo(cookingTime) > 30 * 60) {
               booking = POSDATA::TAB_BOOKING_SHARE;
           } else if( cur_time.secsTo(cookingTime) > 0 ){
               booking = POSDATA::TAB_BOOKING_TAKEUP;
           }

           if(POSDATA::TAB_BOOKING_TAKEUP == booking) {
               //更新数据库状态
               DataThread::inst().updateOrderStatusbytname( ti.name, POSDATA::OD_BOOKING_SHARE, POSDATA::OD_BOOKING_TAKEUP);
           }
       }
    }

   if( ti.satdown == POSDATA::TAB_TEMP_TAKEUP ) {
      //临时占用， 占用不能超过半小时
      ti.satTime = tab.count("time") > 0 ? tab["time"].toString() : "";

      QDateTime sit_time = QDateTime::fromString(ti.satTime, "yyyy-MM-dd hh:mm:ss");
      QDateTime cur_time = QDateTime::currentDateTime();

      qDebug() << "sit_time: " << sit_time << ", cur_time: " << cur_time;
      if(sit_time.isValid() == false || abs(cur_time.secsTo(sit_time)) > 60 * 1)  {
          //已过期
          _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + ti.name, "satdown", QString::number(POSDATA::TAB_UNSITDOWN)));
          _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + ti.name, "time", ""));

          ti.satdown = 0;
          ti.oid = 0; //清订单
      } else {
          ti.btn->setStyleSheet("background-color: rgb(235, 224, 180)");
          QString label = QObject::tr("%1\n临时占位\n%2").arg(ti.name, sit_time.toString("hh:mm"));
          ti.btn->setText(label);
      }

   } else if( ti.satdown == POSDATA::TAB_SATDOWN ) {
       //正常坐下
       ti.satTime = tab.count("time") > 0 ? tab["time"].toString() : "";

       QDateTime dt = QDateTime::fromString(ti.satTime, "yyyy-MM-dd hh:mm:ss");
//           qDebug() <<" table satdown at: " << dt.toString();
       QString label = QObject::tr("%1\n已开台\n%2").arg(ti.name, dt.toString("hh:mm"));

       //如果还没有oid，则显示黄色，
       if(ti.oid == 0) {
           ti.btn->setStyleSheet("background-color: rgb(206, 182, 167)");
       } else {
            ti.btn->setStyleSheet("background-color: rgb(170, 155, 177)");
       }

       if(0 == booking) {
            ti.btn->setText(ti.name + "\n" + tr("已开台") + "\n" + dt.toString("hh:mm"));

       } else {
           ti.btn->setText(ti.name + "\n" + tr("已开台，有预约") + "\n" + dt.toString("hh:mm"));

       }

   } else if(ti.satdown == POSDATA::TAB_UNSITDOWN) {
       ti.btn->setStyleSheet("background-color: rgb(99, 163, 175)");
       ti.btn->setText(QObject::tr("%1\n空台").arg(ti.name));


       if(0 == booking) {
           ti.btn->setText(QObject::tr("%1\n空台").arg(ti.name));

       } else {
           ti.btn->setText(QObject::tr("%1\n空台，有预约").arg(ti.name));
       }

   }

   if( booking == POSDATA::TAB_BOOKING_SHARE ){
       ti.btn->setStyleSheet("background-color: rgb(240, 178, 178)");

   } else if(booking == POSDATA::TAB_BOOKING_TAKEUP) {
       ti.btn->setStyleSheet("background-color: rgb(238, 121, 121)");

   }

//   qDebug() << "tab name: " << ti.name << ", satdown: " << ti.satdown << ", time: " << ti.satTime;
   m_tables[ti.name] = ti;

   //更新缓存中
    _rediscli->sendCommand(Redis::Command::HKEYS("res:info:tables:" + QString::number(gRID) + ti.name));
}
void TablesFrame::tab2Redis(const QString& tname) {

    if(m_tables.count(tname) == 0
            || tname == "") {
        return;
    }

    QStringList tabs = tname.split("-");

    //设置桌子过期时间，即每天凌晨4点过期
    QDateTime today = QDateTime::currentDateTime();
    QDateTime tomorrow;
    tomorrow.setDate(today.date().addDays(1));
    tomorrow.setTime( QTime(4, 0, 0));



    foreach(QString tname, tabs) {
        _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + tname ,
                                             "name",
                                             tname));

        _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + tname ,
                                             "satdown",
                                             "0"));
        _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + tname ,
                                             "time",
                                             QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));

        _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + tname ,
                                             "oid",
                                             QString::number(0)));

         _rediscli->sendCommand(Command::EXPIRE("res:info:tables:" + QString::number(gRID) + ":" + tname , today.secsTo(tomorrow)));
    }
}

void TablesFrame::updateTables() {
    _rediscli->sendCommand(Redis::Command::KEYS("res:info:tables:" + QString::number(gRID) + ":*" ));

}
int TablesFrame::getOrderId(const QString& tabName) {
    if( m_tables.count( tabName) == 0) {
        return 0;
    }
    return m_tables[tabName].oid;
}
void TablesFrame::resetTable(const QString& tname) {
    tab2Redis(tname);
}

void TablesFrame::connected() {
    qDebug() << " table frame connect to redis OK";

    _rediscli->sendCommand(Command::Auth(REDISAUTH));
//    updateTables();
//    init();

}
void TablesFrame::disconnected() {
    usleep(2 * 1000 * 1000);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);
}

void TablesFrame::onMonitor4TablStatus() {
    if(this->isHidden()) {
        return;
    }
    _rediscli->sendCommand(Redis::Command::KEYS("res:info:tables:" + QString::number(gRID) + ":*" ));
}


void TablesFrame::onReply(const QString& cmd, Redis::Reply value) {
//    qDebug() << "redis reader: " << cmd << ", type: " << value.type() << ", value: " << value.value();

    QString key = cmd.toLower().trimmed();
    if(key.startsWith("auth")) {
        this->init();

    } else if(key.startsWith("hgetall res:info:tables") == true) {

        QString tname =  key.toLower().replace(0, QString("hgetall res:info:tables:" + QString::number(gRID) + ":").length(), "");
        tname = tname.simplified().toUpper();

        //获取所有的桌号信息
        if( value.value().toList().length() == 0) {
            return;
        }

        QList<QVariant> item = value.value().toList();

        QVariantMap tab;
        for(int i = 0; i < item.length(); i++) {
            if( item[i].toString() == "id") {
                tab.insert("id", item[i + 1].toInt());

            } else if(  item[i].toString() == "name") {
                tab.insert("name", item[i + 1].toString());

            } else if(  item[i].toString() == "satdown") {
                tab.insert("satdown", item[i + 1].toInt());

            } else if(  item[i].toString() == "time") {
                tab.insert("time", item[i + 1].toString());

            } else if(  item[i].toString() == "oid") {
                tab.insert("oid", item[i + 1].toInt());

            } else if( item[i].toString() == "cookingTime") {
                tab.insert("cookingTime", item[i + 1].toString());

            } else if( item[i].toString() == "sid" ) {
                tab.insert("sid", item[i + 1].toString());

            }
        }
        this->updateTable(tab);

    } else if(key.startsWith("keys res:info:tables:" + QString::number(gRID) + ":" )) {
        //遍历所有已更新的桌号
        if( value.value().toList().length() == 0) {
            return;
        }
        QList<QVariant> item = value.value().toList();

        foreach(QVariant hk, item) {
//            qDebug() << " table check hkey: " << hk.toString();

            _rediscli->sendCommand(Redis::Command::HGETALL(hk.toString()));
        }
    } else if(key.startsWith("hkeys res:info:tables:" + QString::number(gRID) + ":" )) {
        QString tname =  cmd;
        tname = tname.replace(0, QString("hkeys res:info:tables:" + QString::number(gRID) + ":" ).length(), "");
        tname = tname.simplified().toUpper();

        if(value.value().toList().length() == 0 && tname.length() > 0) {
            //没有，存入缓存中
            this->tab2Redis(tname);
        }
    } else if( key.startsWith("hget res:info:tables:" + QString::number(gRID) + ":" ) && key.endsWith("sid")) {
        ////查询 桌台的订单号
        QString pattern(QString("res:info:tables:%1:(.*)\\s+.*").arg(gRID));
        QRegExp rx(pattern);

        key.indexOf(rx);
        QString tname = rx.cap(1).toUpper();

        QString sid = value.value().toString();

        QVariantMap order;
        if(sid != "") {

            order = DataThread::inst().loadUnpayOrderBySid(sid, POSDATA::OD_ACCESS);
            if(order.empty()) {
                order = DataThread::inst().loadUnpayOrderBySid(sid, POSDATA::OD_BOOKING_TAKEUP);
            }
        } else {
            //根据桌号来查找
            order = DataThread::inst().loadUnpaiedOrderBytname(tname, POSDATA::OD_ACCESS);
            if(order.empty()) {
                order = DataThread::inst().loadUnpaiedOrderBytname(tname, POSDATA::OD_BOOKING_TAKEUP);
            }
        }


        if( order.empty() == false ) {

            OrderBean ob(order);

            gWnd->openMenuFrame(ob);

        } else {
            _ktFrm->addTab(tname);

            _ktFrm->show();
        }

        _rediscli->sendCommand(Command::HGETALL("res:info:tables:" + QString::number(gRID) + ":" + tname));
    }


    if(key.startsWith("keys res:info:tables:"+ QString::number(gRID) + ":*" )) {
        QTimer::singleShot(1000, this, SLOT(onMonitor4TablStatus()));
    }

}

void TablesFrame::onTabClicked(const QString& tname) {
    qDebug() << "TablesFrame::onTabClicked: " << tname;

    _rediscli->sendCommand(Command::HGET("res:info:tables:" + QString::number(gRID) + ":" + tname, "sid"));
}
