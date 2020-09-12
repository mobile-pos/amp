#include "consumertypeframe.h"
#include "tablesframe.h"
#include <QDateTime>
#include <QLabel>
#include <QMessageBox>

#include "datathread.h"

#define SELECTEDBGCOLOR "background-color: rgb(180, 228, 69)"
#define BGCOLOR "background-color: rgb(92, 105, 60)"

ConsumerTypeFrame::ConsumerTypeFrame(QWidget *parent) :
    QFrame(parent),
    _lastbtn(nullptr)
{
    _leftControl = new GridLayoutEx(1, this);
    _leftControl->setObjectName("消费者类型变更操作面板");
//    _leftControl->setStyleSheet("background-color: rgb(255, 88, 88)");

//    _vkb = new VKeyboardEx(this);
//    _vkb->setObjectName("虚拟键盘");
//    _vkb->hide();
//    _vkb->init(1);

    _tablayout = new GridLayoutEx(5, this);
    _tablayout->setObjectName("台桌面板");
    _tablayout->hide();

    _tabs = new QMap<QString, QVariantMap>();

    _rediscli = new RedisClient(this);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);

    init();
}

ConsumerTypeFrame::~ConsumerTypeFrame()
{

}

void ConsumerTypeFrame::init() {
    //第一页为电话号码键盘，如果选择了字符输入，则有字符键盘
    //第二页为桌号选择页面
    _leftControl->clean();
    _tablayout->clean();
    _tabs->clear();

    qDebug() << "ConsumerTypeFrame::init";
    {
        //联系方式
        _lname = new POSLabelEx( QObject::tr("电话"), this);
        _lname->setAlignment(Qt::AlignRight |Qt::AlignCenter);

        _name = new LineEditEx(this, true);
        _name->setEnabled(false);
        this->connect(_name, SIGNAL(onInputFinished(int, const QString&)), this, SLOT(onInputFinished(int, const QString&)));

        _laddr = new POSLabelEx(QObject::tr("地址"), this);
        _laddr->setAlignment(Qt::AlignRight |Qt::AlignCenter);

        _addr = new LineEditEx(this, true);
        _addr->setEnabled(false);

        _consAddresses = new QComboBox(this);
        connect(_consAddresses, SIGNAL(currentTextChanged(const QString &)), this, SLOT(currentTextChanged(const QString &)));
        _consAddresses->hide();

        _isDelevery = new QRadioButton(tr("送餐到家"), this);
        _isPickupSelf = new QRadioButton(tr("到店自取"), this);
        connect(_isDelevery, SIGNAL(released()), this, SLOT(onDeleveryClicked()));
        connect(_isPickupSelf, SIGNAL(released()), this, SLOT(onPickupSelfClicked()));

        _isDelevery->hide();
        _isPickupSelf->hide();
    }
    {
        {
            _leftControl->addWidget(new QLabel(_leftControl));
        }
        //类型
        {
            VKeyItemButton *_b = new VKeyItemButton("TC-", this, _leftControl);
            _b->setText(QObject::tr("堂吃"));
            _leftControl->addWidget(_b, 1);
        }

        {
            VKeyItemButton *_b = new VKeyItemButton("QD-", this, _leftControl);
            _b->setText(QObject::tr("外卖"));
            _leftControl->addWidget(_b);
        }
        {
            VKeyItemButton *_b = new VKeyItemButton("QP-", this, _leftControl);
            _b->setText(QObject::tr("外等"));
            _leftControl->addWidget(_b);
        }

        {
            _leftControl->addWidget( new QLabel(_leftControl));
        }
        {
            VKeyItemButton *_b = new VKeyItemButton("Submit", this, _leftControl);
            _b->setText(QObject::tr("确定"));
            _leftControl->addWidget(_b);
        }
    }
    {
        //台桌显示
        QList<QVariant> tables = DataThread::inst().loadTables();
        foreach(QVariant tab, tables) {
            updateTable(tab.toMap());
        }
    }

    qDebug() << "ConsumerTypeFrame::init end";
}

void ConsumerTypeFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect frame = this->frameGeometry();

    int lw = frame.width() * 0.2;

    _leftControl->setRowHeight( frame.height() / 10);
    _leftControl->setGeometry( frame.left(), frame.top(), lw, frame.height());

    lw += 5;
    _lname->setGeometry(lw, 0, 60, 50);
    _name->setGeometry(65 + lw, 0, frame.width() - 80, 50);

    _laddr->setGeometry(lw, 55, 60, 50);

    _addr->setGeometry(65 + lw, 55, frame.width() - 80, 50);

    int lh = 110;

//    _vkb->setGeometry(lw, frame.top() + lh, frame.width() - lw, frame.height() - lh);
    _tablayout->setGeometry(lw, frame.top() + lh, frame.width() - lw, frame.height() - lh);


    QRect rect = this->_addr->geometry();
    _consAddresses->setGeometry(rect.x(), rect.bottom() + 10,
                                rect.width() ,
                                50);

    _isDelevery->setGeometry(rect.x(), _consAddresses->geometry().bottom() + 20,
                             80, 50);
    _isPickupSelf->setGeometry(rect.x() + 110, _consAddresses->geometry().bottom() + 20,
                             80, 50);
}
void ConsumerTypeFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    this->_name->setText("");
    this->_addr->setText("");

    _consAddresses->hide();

    qDebug() << "变更用户类型 1 更新 _leftControl 界面...";
    _leftControl->show();

    //每次显示一次，就刷新一次桌信息
    if(_rediscli != nullptr) {
        _rediscli->sendCommand(Redis::Command::KEYS("res:info:tables:*"));
    }

    if(this->_type.startsWith("QD-")) {
        _isDelevery->show();
        _isPickupSelf->show();
    } else {
        _isDelevery->hide();
        _isPickupSelf->hide();
    }

    _isDelevery->setChecked(false);
    _isPickupSelf->setChecked(false);
}

void ConsumerTypeFrame::retranslateUI() {
    this->init();
}

//选择历史
void ConsumerTypeFrame::currentTextChanged(const QString & txt) {
    _addr->setText(txt);
    _consAddresses->hide();
}
void ConsumerTypeFrame::onInputFinished(int code, const QString& txt) {

    QList<QVariant> addresses = DataThread::inst().loadAddressByphone(txt);
    if(addresses.length() > 0) {
        _consAddresses->clear();

        foreach(QVariant data, addresses) {
            QVariantMap addr = data.toMap();
            _consAddresses->addItem(addr["address"].toString());
        }
        _consAddresses->show();
    }
}

void ConsumerTypeFrame::onDeleveryClicked() {
    _isPickupSelf->setChecked(false);
}
void ConsumerTypeFrame::onPickupSelfClicked() {
    _isDelevery->setChecked(false);
}

void ConsumerTypeFrame::onKeyDown(const QString& value) {
    qDebug() << " on table KeyDown: " << value;


    if(value.startsWith("TC-") ) {
        _type = value.mid(0, 3);
        on_tangchi_type_clicked() ;

        if(value.mid(3).length() > 0) {
            done(_type, value.mid(3), "");
        }
    } else if (  value.startsWith("QD-") ) {
        _type = value.mid(0, 3);
        on_delevery_type_clicked();

    } else if ( value.startsWith("QP-")) {
        _type = value.mid(0, 3);
        on_pickup_type_clicked();

    } else if(value == "Submit") {
        //只用于确认 外卖
        if(_type == "QD-") {
            if(this->_name->text().length() == 0 || this->_name->text() == "null") {
                //需要添电话
                QMessageBox::warning(this,
                                     tr("提示"),
                                     tr("电话不能为空"));
                return;
            }
            if(_isDelevery->isChecked() == false && _isPickupSelf->isChecked() == false) {
                QMessageBox::warning(this,
                                     tr("提示"),
                                     tr("请选择取餐方式"));
                return;
            }
            if(_isDelevery->isChecked()) {
                _type = "QD-";
            } else {
                _type = "QP-";
            }
            done(_type, this->_name->text(), this->_addr->text());
        }
        this->hide();
    } else {
        //普通台桌选择
        done(_type, value, "");
        this->hide();
    }
}
void ConsumerTypeFrame::onVKeyDown(const QString& kvalue) {
    qDebug() << " onVKeyDown: " << kvalue;

    QString lastVal = "";

    if(kvalue.startsWith("TC-")) {
        done(_type, kvalue, "");
    }
}


void ConsumerTypeFrame::connected() {
    qDebug() << " table frame connect to redis OK";

    _rediscli->sendCommand(Command::Auth(REDISAUTH));
}
void ConsumerTypeFrame::disconnected() {
    usleep(2 * 1000 * 1000);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);
}

void ConsumerTypeFrame::updateTable(QVariantMap tab) {

    if(tab.count("name") == 0) {
        return;
    }

    QString tname = tab["name"].toString();

    VKeyItemButton* kb = nullptr;
    if(_tabs->count( tname ) == 0) {
        kb = new VKeyItemButton( tname, this, _tablayout);
        kb->setObjectName("TC-" + tname);
        _tablayout->addWidget(kb, 1, 1);

        _tabs->insert(tname, tab );
    } else {
        kb =  dynamic_cast<VKeyItemButton*>(_tablayout->getItembyObjectName("TC-" + tname));
    }

    if(nullptr != kb && 0 != tab["satdown"].toInt()) {
        kb->setStyleSheet("background-color: rgb(170, 155, 177)");
        kb->setText( tab["name"].toString() + "\n" +
                QDateTime::fromString(tab["time"].toString(), "yyyy-MM-dd hh:mm:ss").toString("hh:mm"));
    } else {
        kb->setStyleSheet("background-color: rgb(99, 163, 175)");
    }

    _tablayout->resizeItems();
}
void ConsumerTypeFrame::onReply(const QString& cmd, Redis::Reply value) {
    QString key = cmd.toLower().trimmed();
    if(key.startsWith("auth")) {

    } else if(key.startsWith("hgetall res:info:tables:" + QString::number(gRID) + ":") == true) {
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
            }
        }
        this->updateTable(tab);

    } else if(key.startsWith("keys res:info:tables:" + QString::number(gRID) + ":*")) {
        //遍历所有已更新的桌号
        if( value.value().toList().length() == 0) {
            return;
        }
        QList<QVariant> item = value.value().toList();
        foreach(QVariant hk, item) {
            _rediscli->sendCommand(Redis::Command::HGETALL(hk.toString()));
        }
    }
}

void ConsumerTypeFrame::on_tangchi_type_clicked() {
    if(nullptr != _lastbtn ) {
       _lastbtn->setStyleSheet(BGCOLOR);
    }
    qDebug() << "变更用户类型 1 堂吃界面...";
//    _vkb->hide();
    qDebug() << "变更用户类型 2 堂吃界面...";
    _name->setText("");

    _name->setEnabled(false);
    _addr->setEnabled(false);

    _isDelevery->hide();
    _isPickupSelf->hide();

//    _addr->setPlainText("");
    qDebug() << "变更用户类型 3 堂吃界面...";

    _tablayout->show();
    qDebug() << "变更用户类型 4 堂吃界面...";

    _lastbtn = _leftControl->getItembyID(1);
    _lastbtn->setStyleSheet(SELECTEDBGCOLOR);

    qDebug() << "变更用户类型 5 堂吃界面...";

}
void ConsumerTypeFrame::on_delevery_type_clicked() {
    if(nullptr != _lastbtn ) {
       _lastbtn->setStyleSheet(BGCOLOR);
    }
    _tablayout->hide();
    _name->setText("");
    _name->setEnabled(true);
    _addr->setEnabled(true);

    _isDelevery->show();
    _isPickupSelf->show();
//    _addr->setPlainText("");

//    _vkb->show();
    this->_name->setFocus();

    _lastbtn = _leftControl->getItembyID(2);
    _lastbtn->setStyleSheet(SELECTEDBGCOLOR);
    //需要设置地址，所以要有两个输入框，一个是电话联系方式，一个地址
}

void ConsumerTypeFrame::on_pickup_type_clicked() {
    _tablayout->hide();
    _name->setText("");

    _isDelevery->hide();
    _isPickupSelf->hide();
//    _addr->setPlainText("");

    //初始化一个虚拟的号
    QVariantMap num = DataThread::inst().getQueueNumber();
    if(num.empty()) {
        //没有获取到排队号
        qDebug() << "没有获取到排队号";
        QMessageBox::warning(this, "出错", "没获取到排队号");
        return;
    }

    done(_type, QString("%1").arg( num["qno"].toInt(), 5, 10, QLatin1Char('0')), QObject::tr("当前排队人数") + ": " + num["count"].toString());
}

void ConsumerTypeFrame::updateCurName(const QString& type, const QString& name, const QString& addr) {
    qDebug() << "变更用户类型 1 更新数据...";
    if(type.startsWith("TC-")) {
        //如果是占了桌位，侧变更后需要退桌
        onKeyDown(type);

        qDebug() << "switch to tangchi: " << type;

    } else if(type.startsWith("QD-")) {
        onKeyDown(type);

        qDebug() << "switch to delevery: " << type;
        _name->setText(name);
        if(name.length() > 0) {
            this->onInputFinished(0, name);
        }
//        _addr->setPlainText(addr);
    }
    qDebug() << "变更用户类型 2 更新数据 end";
}
