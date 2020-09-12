#include "loginframe.h"
#include <QLabel>
#include <QLineEdit>
#include <QDebug>
#include <QCryptographicHash>
#include <QMessageBox>

#include "vkeyboardex.h"
#include "datathread.h"

#include "mainwindow.h"

extern MainWindow* gWnd;
///////////////////////////////////////////////////////////
/// \brief LoginFrame::LoginFrame
/// \param parent
///

LoginFrame::LoginFrame(QWidget *parent) :
    QFrame(parent)
{
    this->setObjectName("loginWnd");
    this->setStyleSheet("LoginFrame#loginWnd{ border-image: url(:/res/img/grass.jpg);}");

    _layout = new GridLayoutEx(15, this);
    _layout->setObjectName("loginArea");
    _layout->setStyleSheet("#loginArea{ background: gray;}");
    _layout->setCellBord(0);
    //半透明设置
//    _layout->setWindowFlags(Qt::FramelessWindowHint);
//    _layout->setAttribute(Qt::WA_TranslucentBackground);
    _layout->setWindowOpacity(0.5);

    _vkb = new VKeyboardEx(this);
    _vkb->setObjectName("virtualKB");
    _vkb->setCellBord(0);
    _vkb->setStyleSheet("#virtualKB{ background: rgb(116, 122, 131);}");

    init();
}

LoginFrame::~LoginFrame()
{
}
void LoginFrame::init() {
/**
+------------------------+
|       title            |
|   name:                |
|   pwd :                |
|------------------------|
|       OK      cancel   |
|------------------------|
|   virtual keyboard     |
+------------------------+
*/
    qDebug() << "LoginFrame::init";
    //1行15个单元
    {
        //title
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 3, 2);
        }
        {
            QLabel* _l = new POSLabelEx(QObject::tr("请登录"), _layout);
//            _l->setStyleSheet("background-color: rgb(80, 114, 165)");
            _l->setAlignment(Qt::AlignCenter);
            _l->setObjectName("LoginTitle");
            _layout->addWidget(_l, 10, 2);
        }
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 2, 2);
        }
    } {
        //输入用户名
        {
            QLabel* _l = new POSLabelEx(QObject::tr("账号："), _layout);
            _l->setAlignment(Qt::AlignRight |Qt::AlignCenter);
            _layout->addWidget(_l, 4, 2);
        } {
            LineEditEx* _e = new LineEditEx(_layout);
            this->connect(_e, SIGNAL(focussed(QWidget*, bool)), this, SLOT(focussed(QWidget*, bool)));

            _e->setPlaceholderText(QObject::tr("请输入账号"));
            _e->setObjectName("LoginAccout");
            _e->setText("demoroot");

            _layout->addWidget(_e, 10, 2, 1000);
        }
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 1, 2);
        }
    }{
        //输入密码
        {
            QLabel* _l = new POSLabelEx(QObject::tr("密码："), _layout);
            _l->setAlignment(Qt::AlignRight |Qt::AlignCenter);
            _layout->addWidget(_l, 4, 2);
        } {
            LineEditEx* _e = new LineEditEx(_layout);
            this->connect(_e, SIGNAL(focussed(QWidget*, bool)), this, SLOT(focussed(QWidget*, bool)));
            _e->setEchoMode(QLineEdit::Password);

            _e->setPlaceholderText(QObject::tr("请输入密码"));
            _e->setObjectName("LoginPasswd");
            _e->setText("1356@aiwaiter");

            _layout->addWidget(_e, 10, 2, 1001);
        }

        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 1, 2);
        }
    } {
        //空行
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 15, 1);
        }
    } {
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 3, 2);
        }
        {
            VKeyItemButton* _btn = new VKeyItemButton("OK", this, _layout);
            _btn->setText(QObject::tr("点击登录"));
            _btn->setStyleSheet("background-color: rgb(80, 114, 165)");
            _btn->setFocus();
            _layout->addWidget(_btn, 3, 2);
        }
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 3, 2);
        }
        {
            VKeyItemButton* _btn = new VKeyItemButton("Cancel", this, _layout);
            _btn->setText(QObject::tr("重置"));
            _btn->setStyleSheet("background-color: rgb(80, 114, 165)");
            _layout->addWidget(_btn, 3, 2);
        }
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 3, 2);
        }
    } {
        //空一行
        {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 15, 1);
        }
    } {
        _vkb->init(0);
        _vkb->hide();
    }

    qDebug() << "LoginFrame::init end";
}

void LoginFrame::login(bool isAdmin) {
    QLabel* lab = dynamic_cast<QLabel*>( _layout->getItembyObjectName("LoginTitle") );
    if(true == isAdmin) {
        lab->setText(tr("请管理员登陆"));
    } else {
        lab->setText(tr("请登陆"));
    }
    this->showMaximized();

}
void LoginFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();

    int w = 400;
    int h = 200;
    QRect center( rect.width()/2 - w/2, rect.height() * 0.3 - h/2, w, h);
    _layout->setRowHeight(h/10);
    _layout->setGeometry(center);

    QRect lrect = _layout->geometry();
    _vkb->setGeometry(lrect.left() - 100, lrect.bottom() + 5, lrect.width() + 200, rect.height() -lrect.bottom() - 30);
}

void LoginFrame::onKeyDown(const QString& value) {
    qDebug() << "onKeyDown: " << value;
    if( "Cancel" == value) {
        LineEditEx* _edit = dynamic_cast<LineEditEx*>(_layout->getItembyID(1000));
        _edit->setText("");
        _edit = dynamic_cast<LineEditEx*>(_layout->getItembyID(1001));
        _edit->setText("");
    } else if("OK" == value) {
        //调用登录接口
        LineEditEx* _en = dynamic_cast<LineEditEx*>(dynamic_cast<LineEditEx*>(_layout->getItembyID(1000)));
        LineEditEx* _ep = dynamic_cast<LineEditEx*>(dynamic_cast<LineEditEx*>(_layout->getItembyID(1001)));
        QString name = _en->text();
        QString pwd = _ep->text();

        if(name == "") {
            _en->setPlaceholderText(QObject::tr("账号不能为空"));
            return;
        }
        if(pwd == "") {
            _en->setPlaceholderText(QObject::tr("密码不能为空"));
            return;
        }

        QString key = name + pwd;
        QString md5 = QString(QCryptographicHash::hash( key.toUtf8(),QCryptographicHash::Md5).toHex());
        QVariantMap ret = DataThread::inst().login(name, md5);

//        //@Task Todo for test
//        ret["account"] = "r002";
//        ret["id"] = 2;

        if(ret.empty() || 0 != ret["code"].toInt()) {
            QMessageBox::warning(this, "登录失败", QString("用户名或密码不对，请重试 或是网络异常: %1").arg(ret["msg"].toString()));

            gWnd->recordPOSEvent("login", "login failed, 用户名或密码不对，请重试 或是网络异常");
            return;
        }

        if( ret["id"].toInt() != DataThread::inst().getRestaurantInfo()["rid"].toInt()) {
            QMessageBox::warning(this, "登录失败", "非本店账号不能登录");

            gWnd->recordPOSEvent("login", "login failed, 非本店账号不能登录");
            return;
        }

        DataThread::inst().setLogin(ret["account"].toString(), ret);

        _ep->setText("");
        this->hide();

        gWnd->Home();

        gWnd->updateLoginInfo();

        //
        if(0 != DataThread::inst().updateTables2Cloud()) {
            QMessageBox::warning(this, "出错", "更新桌台信息到云端失败，可能影响到扫码点餐");
        }

        gWnd->recordPOSEvent("login", "login OK");
    }
}

void LoginFrame::onVKeyDown(const QString& value) {
    qDebug() << "onVKeyDown: " << value;

    if("OK" == value) {
    }
}
void LoginFrame::focussed(QWidget* _this, bool hasFocus) {
    if(true == hasFocus) {
        LineEditEx* _edit = dynamic_cast<LineEditEx*>(_this);
//        _edit->setStyleSheet("background-color: red(255, 255, 255)");
        _vkb->setTarget( _edit );
        _vkb->show();
    } else {
        _vkb->hide();
    }
}

