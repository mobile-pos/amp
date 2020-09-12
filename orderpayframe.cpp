#include "orderpayframe.h"
#include <QLabel>
#include <QLineEdit>
#include <QListWidget>
#include <QDebug>
#include <QMessageBox>
#include <QDateTime>
#include <QDateTime>

#include "datathread.h"

#include "vkeyboardex.h"
#include "mainwindow.h"

extern MainWindow* gWnd;


#define OPENCASHBOXREQ "pos:cashbox:event:req:open"
#define CHECKCASHBOXREQ "pos:cashbox:event:req:check"
#define CLOSECASHBOXREQ "pos:cashbox:event:req:close"



OrderPayFrame::OrderPayFrame(OrderBean* ob, QWidget *parent) :
    QFrame(parent)
   , _ob(ob)
{
    qDebug() << "OrderPayFrame::init";

    _layout = new GridLayoutEx(8, this);
    _layout->setCellBord();

    _tipFrm = new GratuityFrame(this->parentWidget());
    _tipFrm->hide();
    this->connect(_tipFrm, SIGNAL(onGratuityDone(QList<GratuityItem>&)), this, SLOT(onGratuityDone(QList<GratuityItem>&)));

    _redis =  new RedisClient(this);
    _redis->connectToServer(REDISSERVER, REDISPORT);

    init();

    _qrcFrm = new QRCodePayFrame(this);
    _qrcFrm->hide();
    this->connect(_qrcFrm, SIGNAL(onlinePayFinished(int, int)), this, SLOT(onlinePayFinished(int, int)));

    qDebug() << "OrderPayFrame::init end";


}

OrderPayFrame::~OrderPayFrame()
{
}

void OrderPayFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();
    _layout->setRowHeight( rect.height() / 12);

    _layout->setGeometry(0, 0, rect.width(), rect.height());

    _tipFrm->setGeometry(0, 0, rect.left(), rect.height() * 0.6);
}
void OrderPayFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    _paied->setText("");
}

void OrderPayFrame::hideEvent(QHideEvent *event) {
    QFrame::hideEvent(event);

    this->_tipFrm->hide();
}

void OrderPayFrame::init() {
    //一行 8个单元
    QString bgcolor = "background-color: rgb(182, 167, 125)";
    QString kb1bgcolor = "background-color: rgb(145, 142, 131)";
    QString kb2bgcolor = "background-color: rgb(112, 116, 107)";
    {
        {
            VKeyItemButton* _l = new VKeyItemButton("VISA", this, _layout);
            _l->setText("Exact Visa");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2);
        } {
            _paied = new QLineEdit(_layout);
            _paied->setText("0");
            _layout->addWidget(_paied, 3);
        } {
            POSToolButtonEx* _l = new POSToolButtonEx(_layout);
            this->connect(_l, SIGNAL(released()), this, SLOT(on_tips_clicked()));
            _l->setText(QObject::tr("小费"));
            _layout->addWidget(_l, 3);
        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("MASTER", this, _layout);
            _l->setText("Exact Master");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2);
        } {
            for(int i = 0; i< 3; i++) {
                VKeyItemButton* _l = new VKeyItemButton("KB:" + QString::number(i + 1), this, _layout);
                if(i % 2 == 0 ) {
                    _l->setStyleSheet(kb1bgcolor);
                } else {
                    _l->setStyleSheet(kb1bgcolor);
                }
                _l->setText( QString::number( i + 1));
                _layout->addWidget(_l, 1);
            }

            QLabel* _l = new QLabel(_layout);
            _layout->addWidget(_l, 1);

        }  {
            //占了2列，2行
//            POSToolButtonEx* _l = new POSToolButtonEx(_layout);
            VKeyItemButton* _l = new VKeyItemButton("Clean", this, _layout);
            _l->setText("Clean");
            _layout->addWidget(_l, 2, 2);
        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("AMEX", this, _layout);
            _l->setText("Exact AMEX");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2);
        } {
            for(int i = 3; i< 6; i++) {
                VKeyItemButton* _l = new VKeyItemButton("KB:" + QString::number(i + 1), this, _layout);
                if(i % 2 == 0 ) {
                    _l->setStyleSheet(kb1bgcolor);
                } else {
                    _l->setStyleSheet(kb1bgcolor);
                }
                _l->setText( QString::number( i + 1));
                _layout->addWidget(_l, 1);
            }

            QLabel* _l = new QLabel(_layout);
            _layout->addWidget(_l, 1);

        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("DISC", this, _layout);
            _l->setText("Exact Disc");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2);
        } {
            for(int i = 6; i< 9; i++) {
                VKeyItemButton* _l = new VKeyItemButton("KB:" + QString::number(i + 1), this, _layout);
                if(i % 2 == 0 ) {
                    _l->setStyleSheet(kb1bgcolor);
                } else {
                    _l->setStyleSheet(kb1bgcolor);
                }
                _l->setText( QString::number( i + 1));
                _layout->addWidget(_l, 1);
            }

            QLabel* _l = new QLabel(_layout);
            _layout->addWidget(_l, 1);

        }  {
            //占了2列，2行
            VKeyItemButton* _l = new VKeyItemButton("Del", this, _layout);
            _l->setStyleSheet(kb1bgcolor);
            _l->setText("Delete");
            _layout->addWidget(_l, 2, 1);

        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("ExactCASH", this, _layout);
            _l->setText("Exact Cash");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2);
        } {
            {
                VKeyItemButton* _l = new VKeyItemButton("KB:" + QString::number(0), this, _layout);
                _l->setText( "0" );
                _l->setStyleSheet(kb1bgcolor);
                _layout->addWidget(_l, 1);
            } {
                VKeyItemButton* _l = new VKeyItemButton("KB:.", this, _layout);
                _l->setText( "." );
                _l->setStyleSheet(kb2bgcolor);
                _layout->addWidget(_l, 1);
            }

            {
                VKeyItemButton* _l = new VKeyItemButton("+", this, _layout);
                _l->setEnabled(false);
                _l->setStyleSheet(kb1bgcolor);
                _layout->addWidget(_l, 1);
            }
        }  {
            //订单应付费用
            _amount = new QLineEdit(_layout);
            _amount->setText( QString::number(_ob->amount) );
            _layout->addWidget(_amount, 3);
        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("1000", this, _layout);
            _l->setText("-");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2, 1, 1000);
        } {
            _tipSummary = new QListWidget(_layout);
            _tipSummary->addItem("Credit cars summary:");
            _layout->addWidget(_tipSummary, 6, 4);
        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("1001", this, _layout);
            _l->setText("-");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2, 1, 1001);
        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("100", this, _layout);
            _l->setText("100");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2, 1, 1002);
        }
    } {
        {
            VKeyItemButton* _l = new VKeyItemButton("WeiXinPay", this, _layout);
            _l->setText(tr("微信支付"));
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2, 1, 1003);
        }
    } {
        //一行
        {
            VKeyItemButton* _l = new VKeyItemButton("AliPay", this, _layout);
            _l->setText(tr("支付宝支付"));
            _l->setStyleSheet(bgcolor);
            _l->setEnabled(false);
            _layout->addWidget(_l, 2, 1, 1004);
        }
    } {
        {
            //占了2列，2行
            VKeyItemButton* _l = new VKeyItemButton("Finish", this, _layout);
            _l->setText(QObject::tr("结算"));
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 2, 2);
        } {
            QLabel* _l = new QLabel(_layout);
            _layout->addWidget(_l, 4);
        } {
            QLabel* _l = new QLabel(_layout);
            _layout->addWidget(_l, 2);
        } {
            QLabel* _l = new QLabel(_layout);
            _layout->addWidget(_l, 1);
        } {
            //占了2列，2行
            VKeyItemButton* _l = new VKeyItemButton("Cash", this, _layout);
            _l->setText("Cash");
            _l->setStyleSheet(bgcolor);
            _layout->addWidget(_l, 1, 1);

        }
    }
}

void OrderPayFrame::connected() {
    qDebug() << "OrderPayFrame redis connected";
    _redis->sendCommand(Command::Auth(REDISAUTH));
}
void OrderPayFrame::disconnected() {
    qDebug() << "OrderPayFrame redis disconnected";
    usleep(2 * 1000 * 1000);
    _redis->connectToServer(REDISSERVER, REDISPORT);
}


void OrderPayFrame::onReply(const QString& cmd, Redis::Reply value) {
    qDebug() << "onReply cmd: " << cmd <<", value: " << value.value().toString();

    QString key = cmd.toLower().trimmed();
    if(key.startsWith("auth")) {
    }
}
bool isDigitString(const QString& src) {
    const char *s = src.toUtf8().data();
    while(*s && *s>='0' && *s<='9')s++;
    return !bool(*s);
}

void OrderPayFrame::onKeyDown(const QString& value) {
    if("Del" == value) {
        std::string cid = _paied->text().toStdString();
        if(cid.length() == 0 ){
            return;
        }
        cid = cid.substr(0, cid.length() - 1);
        _paied->setText(cid.c_str());

        double paied = _paied->text().toDouble();
        _amount->setText( QString::number(_ob->amount - paied) );

    } else if( "Clean" == value) {
        _paied->setText("");

        double paied = _paied->text().toDouble();
        _amount->setText( QString::number(_ob->amount - paied) );
    } else if("OK" == value) {

        qDebug() <<" paied OK: " << _paied->text().toDouble();

    } else if( value.startsWith("KB:")) {
        QString num = value.mid(3);

        _paied->setText( _paied->text() + num);

        double paied = _paied->text().toDouble();
        //if( paied >= _ob->amount) {
            _amount->setText( QString::number(_ob->amount - paied) );
        //}
    } else if( value.startsWith("VKB:")) {
        QString num = value.mid(4);

        double paied = num.toDouble();
        _amount->setText( QString::number(_ob->amount - paied) );
    } else if("Cash" == value) {
        // 现金结算完成
        _ob->finishTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        //_ob.payType = "cash";
        _redis->sendCommand(Command::PUBLISH(QString("%1:%2").arg(OPENCASHBOXREQ).arg(gRID), "1"));
        emit(onFinishPay());
    } else if("ExactCASH" == value) {
        //_ob.payType = "cash";
        _ob->payType = POSDATA::CASH;
        _ob->status = POSDATA::OD_ODPAIED;
        _redis->sendCommand(Command::PUBLISH(QString("%1:%2").arg(OPENCASHBOXREQ).arg(gRID), "1"));

        this->hide();

        emit(onFinishPay());
    } else if("Finish" == value) {
        //更新状态为 结单状态（16)，主要是标识小费
        this->_ob->status = POSDATA::OD_FINISHORDER;
        DataThread::inst().updateOrderInfo(this->_ob->toMap());

        this->hide();

        emit(onFinishPay());
    } else if( value == "+") {
//        _paied->setText( QString::number(_paied->text().toDouble() + value.toInt()));
    } else if( value == "WeiXinPay") {
        newWeiXinQRCodePay();
    } else if(value == "AliPay") {
        newAliQRCodePay();
    } else {
        //VISA  直接结束订单, CASH等需要弹出钱箱
        _ob->finishTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
        _ob->payType = POSDATA::CASH;
        _ob->status = POSDATA::OD_ODPAIED;

        if( value.toUpper() == "VISA"
                || "MASTER" == value.toUpper()
                || "DISC" == value.toUpper()
                || "AMEX" == value.toUpper()) {
            _ob->payType = POSDATA::CREDIT;
            _ob->creditType = value.toUpper();

            int laststatus = _ob->status;
            //信用卡，直接结算，后面再录入小费
            if(0 == DataThread::inst().finishOrder( this->_ob->toMap() )) {
                //结算完成
                this->hide();
                //清空坐位
                emit(onFinishPay());
                gWnd->Home();

            } else {
                _ob->status = laststatus;
                QMessageBox::warning(this->parentWidget(), "出错", "订单结算提交失败，请重试", QMessageBox::Ok);
            }
        } else {
            qDebug() << "pop money box " << value;
            //关闭钱箱才返回主界面
        }
    }

    if(value != "WeiXinPay" && value != "AliPay") {

        this->_qrcFrm->hide();
    }
}

void OrderPayFrame::on_tips_clicked() {
    if( !this->_tipFrm->isHidden() ) {
        this->_tipFrm->hide();
        return;
    }

    this->_tipFrm->updateOrder(this->_ob);
    this->_tipFrm->show();
}
void OrderPayFrame::onGratuityDone(QList<GratuityItem>& tips) {

    foreach (GratuityItem tip, tips) {
        QString item = QString("%1=====%2======%3").arg(tip._creditName, 5).arg(tip._tips, 5).arg(tip._amount, 5);
        _tipSummary->addItem(item);
    }
}
void OrderPayFrame::updateOrder(OrderBean* ob) {
    _ob = ob;
    _amount->setText(QString("%1").arg(_ob->amount));
    _tipSummary->clear();
    _tipSummary->addItem("Credit cars summary:");

    VKeyItemButton* vk1 = dynamic_cast<VKeyItemButton*>(_layout->getItembyID(1000));
    VKeyItemButton* vk2 = dynamic_cast<VKeyItemButton*>(_layout->getItembyID(1001));
    VKeyItemButton* vk3 = dynamic_cast<VKeyItemButton*>(_layout->getItembyID(1002));

    int namount = (int)_ob->amount;
    if( namount < _ob->amount) {
        vk1->setObjectName( "VKB:" + QString::number( namount + 1) );
        vk1->setText(QString::number(namount + 1));
    } else {
        vk1->setObjectName( "VKB:" + QString::number(namount) );
        vk1->setText(QString::number(namount));
    }

    if( namount % 10 != 0) {
        int v2 = namount / 10;
        v2 = v2 * 10 + 10;
        vk2->setObjectName("VKB:" + QString::number(v2));
        vk2->setText(QString::number(v2));
    } else {
        vk2->setObjectName("VKB:" + QString::number( namount * 10));
        vk2->setText(QString::number(namount * 10));
    }

    if( namount % 100 != 0) {
        int v3 = namount / 100;
        vk3->setObjectName( "VKB:" + QString::number(v3 * 100 + 100));
        vk3->setText( QString::number(v3 * 100 + 100));
    } else {
        vk3->setObjectName( "VKB:" + QString::number(namount * 100));
        vk3->setText( QString::number(namount * 100));
    }

}

void OrderPayFrame::onlinePayFinished(int oid, int type) {

    OrderBean ob;
    ob.oid = oid;
    ob.finishTime = QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss");
    ob.payType = type;
    ob.status = POSDATA::OD_ODPAIED;

    DataThread::inst().finishOrder(ob.toMap());

    this->hide();
    this->_qrcFrm->hide();

    emit(onFinishPay());
}
void OrderPayFrame::newWeiXinQRCodePay() {
    QVariantMap ret = DataThread::inst().newPayOrder4LocalOrder(this->_ob->oid, this->_ob->sid, this->_ob->amount);
    if(ret.isEmpty()) {
        QMessageBox::warning(this, tr("出错"), tr("获取订单二维码失败，清检查网络"));
        return;
    }

    _qrcFrm->setMinimumSize(300, 300); //(0, 0, 300, 300);
    _qrcFrm->move( _tipSummary->geometry().left(), _tipSummary->geometry().top() );

    _qrcFrm->show();
    _qrcFrm->updateQRCode(QUrl( ret["payParams"].toMap().value("codeImgUrl").toString()),
            ret["payOrderId"].toString(),
            POSDATA::WEIPAY,
            this->_ob->sid);
}

void OrderPayFrame::newAliQRCodePay() {
//    QVariantMap ret = DataThread::inst().newPayOrder4LocalOrder(this->_ob->oid, this->_ob->sid, this->_ob->amount);
//    if(ret.isEmpty()) {
//        QMessageBox::warning(this, tr("出错"), tr("获取订单二维码失败，清检查网络"));
//        return;
//    }
//    _qrcFrm->setMinimumSize(300, 300); //(0, 0, 300, 300);
//    _qrcFrm->move( _tipSummary->geometry().left(), _tipSummary->geometry().top() );

//    _qrcFrm->show();
//    _qrcFrm->updateQRCode(QUrl( ret["payParams"].toMap().value("codeImgUrl").toString()),
//            ret["payOrderId"].toString(),
//            POSDATA::ALIPAY,
//            this->_ob->sid);
}
