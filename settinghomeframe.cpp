#include "settinghomeframe.h"
#include "vkeyboardex.h"

#include <QDebug>

#include "datathread.h"
#include "mainwindow.h"

extern MainWindow* gWnd;


SetPanel::SetPanel(QWidget* parent) : QFrame(parent) {

    qDebug() << "SetPanel::init";

    qDebug() << "SetPanel::init end";
}

void SetPanel::addFrame(QFrame *nfrm) {
    if( nfrm->objectName() == "" ) {
        //不能为空，无法查找
        qDebug() << "frame's object name must not be null";
        return;
    }

    QString obname = nfrm->objectName();

    _frames.insert(obname, nfrm);

    nfrm->hide();
}

QFrame* SetPanel::showFrame(const QString& objname) {
    if( _frames.count(objname) == 0) {
        qDebug() << "not found object name: " << objname;
        return nullptr;
    }
    QFrame* obj = nullptr;

    foreach (QString key, _frames.keys()) {
        QFrame* item = _frames[key];

        if( key != objname) {
            item->hide();
        } else {
            item->show();

            qDebug() << " show frame: " << key;
            obj = item;
        }
    }
    return obj;
}

void SetPanel::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->frameGeometry();

    foreach (QString key, _frames.keys()) {
        QFrame* item = _frames[key];

        item->setGeometry(0, 0, rect.width(), rect.height());
    }

}
void SetPanel::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    QRect rect = this->frameGeometry();

    foreach (QString key, _frames.keys()) {
        QFrame* item = _frames[key];

        item->setGeometry(0, 0, rect.width(), rect.height());
    }
}

//////////////////////////////////////////////////////
/// \brief SettingHomeFrame::SettingHomeFrame
/// \param parent
///
SettingHomeFrame::SettingHomeFrame(QWidget *parent) :
    QFrame(parent)
{
    this->setObjectName("setHomeFrm");
//    this->setStyleSheet("background: rgb(128, 138, 139)");
    this->setStyleSheet("background-color: rgb(224, 224, 224)");

    _layout = new GridLayoutEx(10, this);
    _layout->setObjectName("设置面板");


    _nav = new GridLayoutEx(1, _layout);
    _nav->setObjectName("设置导航面板");
    _nav->setStyleSheet("#设置导航面板{ background: rgb(51, 131, 138);}");
    _nav->setRowHeight(50);

    _panel = new SetPanel(_layout);

    _estiFrm = new EstimateClearlyFrame(_panel);
    _estiFrm->setObjectName("Estimate");
    _estiFrm->setStyleSheet("#Estimate{ background: rgb(128, 138, 139);}");

    _reportFrm = new ReportsFrame(_panel);
    _reportFrm->setObjectName("Reports");

    _ordersFrm = new OrdersManagerFrame(_panel);
    _ordersFrm->setObjectName("Orders");

    _cashFrm = new CashBoxSetFrame(_panel);
    _cashFrm->setObjectName("CashBox");

    _reduceFrm = new ReduceMenuPriceFrame(_panel);
    _reduceFrm->setObjectName("ReducePrice");

    _recomFrm = new RecommendationFrame(_panel);
    _recomFrm->setObjectName("Recommendation");

    _printFrm = new PrintersFrame(_panel);
    _printFrm->setObjectName("Printers");

    _sysFrm = new SysOptFrame(_panel);
    _sysFrm->setObjectName("SysOpt");
    //首页为空
    QFrame* _nullFrm = new QFrame(_panel);
    _nullFrm->setObjectName("Welcome");

    _panel->addFrame(_nullFrm);
    _panel->addFrame(_estiFrm);
    _panel->addFrame(_reduceFrm);
    _panel->addFrame(_recomFrm);
    _panel->addFrame(_reportFrm);
    _panel->addFrame(_ordersFrm);
    _panel->addFrame(_cashFrm);
    _panel->addFrame(_printFrm);
    _panel->addFrame(_sysFrm);

    init();
}

SettingHomeFrame::~SettingHomeFrame()
{
}

void SettingHomeFrame::init() {
    _nav->clean();
    _layout->clean();
    //1行10个单元，左边的导航1/10
    {
        //设置面板布局
        //左边的导航栏
        _layout->addWidget(_nav, 1);
        _layout->addWidget(_panel, 9);
    }
    //具体子控件布局
    {
        {
            VKeyItemButton* _btn = new VKeyItemButton("Estimate", this, _nav);
            _btn->setText(QObject::tr("估清"));
            _nav->addWidget(_btn);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("ReducePrice", this, _nav);
            _btn->setText(QObject::tr("特价类"));
            _nav->addWidget(_btn);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("Recommendation", this, _nav);
            _btn->setText(QObject::tr("店长推荐"));
            _nav->addWidget(_btn);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("Printers", this, _nav);
            _btn->setText(QObject::tr("打印机管理"));
            _nav->addWidget(_btn);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("CashBox", this, _nav);
            _btn->setText(QObject::tr("钱箱设置"));
            _nav->addWidget(_btn);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("Reports", this, _nav);
            _btn->setText(QObject::tr("日常报表"));
            _nav->addWidget(_btn);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("Orders", this, _nav);
            _btn->setText(QObject::tr("订单管理"));
            _nav->addWidget(_btn);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("SysOpt", this, _nav);
            _btn->setText(QObject::tr("系统设置"));
            _nav->addWidget(_btn);
        }
    }

    _nav->resetPages();
    _layout->resetPages();

}

void SettingHomeFrame::retranslateUI() {
    this->init();
}

void SettingHomeFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->frameGeometry();
    _layout->setRowHeight(rect.height());
    _layout->setGeometry(0, 0, rect.width(), rect.height());

    qDebug() << " setting home frame size: " << _layout->geometry();
}
void SettingHomeFrame::showEvent(QShowEvent* e)  {
    QFrame::showEvent(e);

    _panel->showFrame("Welcome");
}

void SettingHomeFrame::onKeyDown(const QString& value) {
    qDebug() << "SettingHomeFrame::onKeyDown: " << value;

    if("Estimate" == value) {
        _panel->showFrame("Estimate");

        gWnd->recordPOSEvent("setting", "沽清设置");
    } else if("Printers" == value) {
        _panel->showFrame("Printers");

    } else if("Reports" == value) {
        if(true == DataThread::inst().isAdminLoggedin()) {
            _panel->showFrame("Reports");

            gWnd->recordPOSEvent("setting", "查看报表");
        } else {
            gWnd->Login(true);
        }

    } else if("Orders" == value) {
        if(true == DataThread::inst().isAdminLoggedin()) {
            _panel->showFrame("Orders");

            gWnd->recordPOSEvent("setting", "订单管理");
        } else {
            gWnd->Login(true);
        }
    } else if("CashBox" == value) {
        _panel->showFrame("CashBox");

    } else if( "ReducePrice" == value) {
        if(true == DataThread::inst().isAdminLoggedin()) {
            _panel->showFrame("ReducePrice");

            gWnd->recordPOSEvent("setting", "特价设置");
        } else {
            gWnd->Login(true);
        }
    } else if( "Recommendation" == value) {
        if(true == DataThread::inst().isAdminLoggedin()) {
            _panel->showFrame("Recommendation");

            gWnd->recordPOSEvent("setting", "店长推荐");
        } else {
            gWnd->Login(true);
        }
    } else if( "SysOpt" == value) {
        _panel->showFrame("SysOpt");

        gWnd->recordPOSEvent("setting", "系统设置");
    } else {
        qDebug() << "没找到指定的面板: " << value;
    }
}
