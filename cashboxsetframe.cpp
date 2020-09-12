#include "cashboxsetframe.h"
#include <stdio.h>
#include <QDebug>

#include <stdlib.h>

#include <string.h>
#include <unistd.h>
#include <fcntl.h>   //define O_WRONLY and O_RDONLY
#include "datathread.h"

#define OPENCASHBOXREQ "pos:cashbox:event:req:open"
#define CHECKCASHBOXREQ "pos:cashbox:event:req:check"
#define CLOSECASHBOXREQ "pos:cashbox:event:req:close"

#define OPENCASHBOXRSP "pos:cashbox:event:rsp:open"
#define CHECKCASHBOXRSP "pos:cashbox:event:rsp:check"
#define CLOSECASHBOXRSP "pos:cashbox:event:rsp:close"

CashBoxSetFrame::CashBoxSetFrame(QWidget *parent) :
    QFrame(parent)
{
    _redis = new RedisClient(this);
    _redis->connectToServer(REDISSERVER, REDISPORT);

    _redisCmd = new RedisClient(this);
    _redisCmd->connectToServer(REDISSERVER, REDISPORT);
    init();
}

CashBoxSetFrame::~CashBoxSetFrame()
{
}

void CashBoxSetFrame::init() {
    _label = new QLabel(this);
    _display = new QLineEdit(this);

    _ctrlID = new LineEditEx(this);
    this->connect(_ctrlID, SIGNAL(focussed(QWidget*, bool)), this, SLOT(focussed(QWidget*, bool)));
    _dispID = new LineEditEx(this);
    this->connect(_dispID, SIGNAL(focussed(QWidget*, bool)), this, SLOT(focussed(QWidget*, bool)));

    _vkb = new VKeyboardEx(this);
    _vkb->hide();

    _kbOpen = new VKeyItemButton("Open", this, this);
    _kbOpen->setText(tr("打开"));

    _kbSet = new VKeyItemButton("CheckStatus", this, this);
    _kbSet->setText(tr("检测状态"));
}

void CashBoxSetFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);
/**
+-------------------------+
|
|  GPIOXX:    |
|  [------]   | OPEN
| [ vkeybox ] |
*/

    QRect rect = this->geometry();

    _label->setGeometry(10, 10, 80, 30);
    _display->setGeometry(95, 10, 240, 30);

    _ctrlID->setGeometry(95, 45, 80, 30);
    _dispID->setGeometry(95, 80, 80, 30);

    _kbOpen->setGeometry(250, 45, 80, 30);
    _kbSet->setGeometry(250, 80, 80, 30);

    _vkb->setGeometry(0, 120, 400, 300);
}

void CashBoxSetFrame::showEvent(QShowEvent * e) {
    QFrame::showEvent(e);

    _label->setText( tr("钱箱状态"));
    _display->setText("钱箱关闭");
}


void CashBoxSetFrame::focussed(QWidget* target, bool hasFocus) {
    if(true == hasFocus) {
        LineEditEx* editor = dynamic_cast<LineEditEx*>(target);
        _vkb->setTarget( editor );
        _vkb->show();
    } else {
        _vkb->hide();
    }
}

void CashBoxSetFrame::onKeyDown(const QString& value) {
    if( "Open" == value ) {
        _redisCmd->sendCommand(Command::PUBLISH(QString("%1:%2").arg(OPENCASHBOXREQ).arg(gRID), "1"));
    } else if("CheckStatus" == value) {
        _redisCmd->sendCommand(Command::PUBLISH(QString("%1:%2").arg(CHECKCASHBOXREQ).arg(gRID), "1"));
    }

}
void CashBoxSetFrame::onVKeyDown(const QString& value) {

}


void CashBoxSetFrame::connected() {
    qDebug() << "HomeFrame::connected redis client connected: ";
    _redis->sendCommand(Command::Auth(REDISAUTH));
}
void CashBoxSetFrame::disconnected() {
    usleep(2 * 1000 * 1000);
    _redis->connectToServer(REDISSERVER, REDISPORT);
}

void CashBoxSetFrame::onReply(const QString& cmd, Redis::Reply value) {
    qDebug() << "onReply cmd: " << cmd <<", value: " << value.value().toString();

    QString key = cmd.toLower().trimmed();
    if(key.startsWith("auth")) {

        _redis->sendCommand(Redis::Command::SUBSCRIBE({
                                                             QString("%1:%2").arg(OPENCASHBOXRSP).arg(gRID),
                                                             QString("%1:%2").arg(CHECKCASHBOXRSP).arg(gRID),
                                                             QString("%1:%2").arg(CLOSECASHBOXRSP).arg(gRID)
                                                         }));
    }
}
void CashBoxSetFrame::messageReceived(const QString& channel, const QVariant& payload) {
    qDebug() << "HomeFrame::messageReceived redis client received: " << channel << ", value: " << payload;

    if(channel.startsWith(QString("%1:%2").arg(CHECKCASHBOXRSP).arg(gRID))) {
        if(payload.toString().length() > 1) {
            _display->setText( QString("[check event] %1 ").arg(payload.toString()));
        } else {
            _display->setText( QString("[check event]cash box is %1")
                             .arg( payload.toInt( ) == 1 ? "opened" : "closed"));
        }
    } else if(channel.startsWith(QString("%1:%2").arg(OPENCASHBOXRSP).arg(gRID))) {
        _display->setText( QString("[open event] %1 ").arg(payload.toString()));
    }
}
