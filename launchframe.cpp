#include "launchframe.h"
#include "datathread.h"

#include <QMessageBox>
#include <QJsonParseError>

#include "mainwindow.h"

static int setupSteps = 0;
QApplication* gApp = nullptr;

LaunchFrame::LaunchFrame(QWidget *parent) :
    QFrame(parent)
{

    this->setObjectName("launchWnd");
    this->setStyleSheet("LaunchFrame#launchWnd{ border-image: url(:/res/img/grass.jpg);}");

    _rediscli = new RedisClient(this);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);

    _loading = new QLabel(this);
    _loading->setText("Loading ...");
}



LaunchFrame::~LaunchFrame()
{
}


void LaunchFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

}

void LaunchFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();
    _loading->setGeometry(10, 10, rect.width() - 10, 50);
}

void LaunchFrame::switchMainFrame() {

    static MainWindow *w = nullptr;


    if(nullptr == w) {
        w = new MainWindow();
    }
    w->setMinimumSize(800, 600);
//    w->setMaximumSize(1024, 768);
    w->showFullScreen();

    gApp->connect( gApp, SIGNAL( lastWindowClosed() ), w, SLOT( quit() ) );

}


void LaunchFrame::connected() {
    qDebug() << "redis connect to local server OK";
    _rediscli->sendCommand(Command::Auth(REDISAUTH));
}
void LaunchFrame::disconnected() {
    usleep(2 * 1000 * 1000);
    _rediscli->connectToServer(REDISSERVER, REDISPORT); //重连
}

bool LaunchFrame::loadTables() {
    QFile tab(QString("%1/tables.conf").arg(WORKDIR));
    if( tab.exists() == false)  {
        qDebug() << "load tables conf failed: file is not exists";
        return false;
    }

    if(!tab.open(QIODevice::ReadWrite))
    {
        qDebug() << "load tables conf failed: file opened failed";
        return false;
    }

    QString data = tab.readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(QByteArray::fromPercentEncoding(data.toLocal8Bit()), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        //QMessageBox::warning(nullptr, "错误", "无法加载口味设置配置文件，配置文件格式不合法");
        qDebug() << "load local data failed, json error: " << json_error.errorString();
        return false;

    } else {
        gTables = jsonDoc.toVariant().toList();

        this->_rediscli->sendCommand(Command::GET("echo"));
        return true;
    }
}

bool LaunchFrame::loadFlavors() {
    QFile flavors(QString("%1/flavors.conf").arg(WORKDIR));
    if( flavors.exists() == false)  {
        qDebug() << "load flavors conf failed: file is not exists";
        return false;
    }
    if(!flavors.open(QIODevice::ReadWrite))
    {
        qDebug() << "load flavors conf failed: file opened failed";
        return false;
    }

    QString data = flavors.readAll();
    QJsonParseError json_error;
    QJsonDocument jsonDoc(QJsonDocument::fromJson(QByteArray::fromPercentEncoding(data.toLocal8Bit()), &json_error));
    if(json_error.error != QJsonParseError::NoError)
    {
        //QMessageBox::warning(nullptr, "错误", "无法加载口味设置配置文件，配置文件格式不合法");
        qDebug() << "load local data failed, json error: " << json_error.errorString();
        return false;

    } else {
        gFlavors = jsonDoc.toVariant().toMap();

        this->_rediscli->sendCommand(Command::GET("echo"));
        return true;
    }
}


void LaunchFrame::onReply(const QString& cmd, Reply value) {
    //
    setupSteps++;
    qDebug() << "LaunchFrame::onReply step: " << setupSteps << ", cmd: " << cmd << ", value: " << value.value().toString();
    QString key = cmd.toLower().trimmed();


    if(key.startsWith("auth")) {
        if(value.value().toString().trimmed() != "OK") {
            setupSteps --;
            this->_loading->setText("AMP: Connect to Server Failed");
            return;
        }
    } else if(key.startsWith(QString("hgetall res:info:%1").arg(gRID))) {

        QList<QVariant> item = value.value().toList();

        if(item.length() == 0) {
            QMessageBox::warning(nullptr, "错误", "当前店家没有设置安装数据");
            return;
        }

        for(int i = 0; i < item.length(); i++) {
            if( item[i].toString() == "rid") {
                gRestaurant.insert("rid", item[i + 1].toInt());
                gRestaurant.insert("id", item[i + 1].toInt());

            } else if(item[i].toString() == "account") {
                gRestaurant.insert("account", item[i + 1].toString());

            } else if(item[i].toString() == "name") {
                gRestaurant.insert("name", item[i + 1].toString());

            } else if(item[i].toString() == "uuid") {
                gRestaurant.insert("uuid", item[i + 1].toString());

            } else if(item[i].toString() == "token") {
                gRestaurant.insert("token", item[i + 1].toString());

            } else if(item[i].toString() == "taxRate") {
                gRestaurant.insert("taxRate", item[i + 1].toFloat());

            } else if(item[i].toString() == "address") {
                gRestaurant.insert("address", item[i + 1].toString());

            } else if(item[i].toString() == "phoneNumbers") {
                gRestaurant.insert("phoneNumbers", item[i + 1].toString());

            } else if(item[i].toString() == "cloudIP") {
                gRestaurant.insert("cloudIP", item[i + 1].toString());
            }
        }

    } else if(key.startsWith(QString("hgetall res:printer:config:rid:%1").arg(gRID))) {

        _kitchPrinter.clear();
        _cashierPrinter = "";

        QList<QVariant> item = value.value().toList();

        for(int i = 0; i < item.length() - 1; i++) {
            if( item[i + 1].toString() == "cashier") {
                _cashierPrinter = item[i].toString();
            } else if( item[i + 1].toString().startsWith("kitchen-")) {
                _kitchPrinter.push_back( item[i].toString());
            }
        }

        if("" == _cashierPrinter) {
            QMessageBox::warning(nullptr, "错误", "前台打印机设置无效");
        }
    } else if(key.startsWith(QString("hget res:conf:%1 flavors").arg(gRID))) {
        QString flavors = value.value().toString();
        if("" == flavors) {
            setupSteps--;
        }
        else {
            QJsonParseError json_error;
            QJsonDocument jsonDoc(QJsonDocument::fromJson(QByteArray::fromPercentEncoding(flavors.toLocal8Bit()), &json_error));
            if(json_error.error != QJsonParseError::NoError)
            {
                //QMessageBox::warning(nullptr, "错误", "无法加载口味设置配置文件，配置文件格式不合法");
                qDebug() << "json error: " << json_error.errorString();
                gFlavors = QVariantMap();

            } else {
                gFlavors = jsonDoc.toVariant().toMap();
            }

        }
    } else if(key.startsWith(QString("hget res:conf:%1 tables").arg(gRID))) {

        QString tables = value.value().toString();
        if("" == tables) {

            setupSteps--;

        }
        else {

            QJsonParseError json_error;
            QJsonDocument jsonDoc(QJsonDocument::fromJson(QByteArray::fromPercentEncoding(tables.toLocal8Bit()), &json_error));
            if(json_error.error != QJsonParseError::NoError)
            {
                //QMessageBox::warning(nullptr, "错误", "无法加载桌台设置配置文件，配置文件格式不合法");
                qDebug() << "json error: " << json_error.errorString();
                gTables = QList<QVariant>();

                setupSteps--;

            } else {
                gTables = jsonDoc.toVariant().toList();
            }
        }
    }

    usleep(200 * 1000);

    switch(setupSteps) {
    case 1:
        this->_loading->setText("AMP: Connect to Server OK, Waiting for loding shop's information...");
        _rediscli->sendCommand(Command::HGETALL("res:info:" + QString::number(gRID) ));
        break;
    case 2:
        this->_loading->setText("AMP: loading shop's info OK, Waiting for loding shop's printers...");
        _rediscli->sendCommand(Command::HGETALL("printers:config:" + QString::number(gRID)));
        break;
    case 3:
        if( false == this->loadTables()) {
            this->_loading->setText("AMP: Connect to Server OK, Waiting for loding shop's tables info ...");
            _rediscli->sendCommand(Command::HGET("res:conf:" + QString::number(gRID), "tables"));
        }
        break;
    case 4:
        if(false == this->loadFlavors()) {
            this->_loading->setText("AMP: Connect to Server OK, Waiting for loding shop's flavors info ...");
            _rediscli->sendCommand(Command::HGET("res:conf:" + QString::number(gRID), "flavors"));
            break;
        }
        break;
    case 5:
        {
            this->hide();
            this->switchMainFrame();
            break;
        }
    }

    this->_loading->update();


}
