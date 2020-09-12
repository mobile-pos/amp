#include "moresettingframe.h"
#include <QDebug>
#include <QListWidget>
#include <QComboBox>
#include <QGroupBox>
#include <QVBoxLayout>
#include <QWidgetAction>
#include <QMessageBox>

#include "mainwindow.h"
#include "vkeyboardex.h"
#include "datathread.h"

extern MainWindow* gWnd;

#define IDSPLITORDER 10000

SplitOrderFrame::SplitOrderFrame(QWidget* target, QWidget* parent) : QFrame(parent), _target(target) {

//    this->setStyleSheet("background-color: rgb(199, 199, 171)");

     _layout = new GridLayoutEx(5, this);
     _layout->setStyleSheet("background-color: rgb(199, 199, 171)");

     _vkb = new VKeyboardEx(_layout);
     _vkb->init(2);

     init();
}
SplitOrderFrame::~SplitOrderFrame() {}

void SplitOrderFrame::init() {
    //
    {
        QLabel* _l = new POSLabelEx(QObject::tr("输入分单的份数："), _layout);
        _l->setAlignment(Qt::AlignRight | Qt::AlignCenter);
        _layout->addWidget(_l, 2);

        _number = new LineEditEx(_layout);
        _number->setText("");
        this->connect(_number, SIGNAL(focussed(QWidget*, bool)), this, SLOT(focussed(QWidget*, bool)));
        _vkb->setTarget(_number);

        _layout->addWidget(_number, 3);
    } {
        _layout->addWidget(_vkb, 4, 4);
    } {
        //确定，取消
        VKeyItemButton* _done = new VKeyItemButton("OK", this, _layout);
        _layout->addWidget(_done, 1);

        VKeyItemButton* _cancel = new VKeyItemButton("Cancel", this, _layout);
        _layout->addWidget(_cancel, 1);
    }
}
void SplitOrderFrame::resizeEvent(QResizeEvent* e)  {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();
    _layout->setRowHeight( rect.height() / 5 );
    _layout->setGeometry(0, 0, rect.width(), rect.height());
}
void SplitOrderFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    _number->setText("");
}

void SplitOrderFrame::onKeyDown(const QString& k) {
    if( "OK" == k) {
        //完成分单操作
        if( _number->text().toInt() == 0) {
            QMessageBox::warning(this, tr("出错"), tr("拆单数不能为空"));
            return;
        }
        this->hide();
        emit(done( _number->text().toInt()  ));
    } else if("Cancel" == k) {
        //
        this->hide();
    }
}
void SplitOrderFrame::onVKeyDown(const QString& k) {

}


////////////////////////////////////////////////////////////////////
/// \brief DiscountFrame::DiscountFrame
/// \param target
/// \param parent
///
DiscountFrame::DiscountFrame(QWidget* target, QWidget* parent) : QFrame(parent), _target(target) {
    _layout = new GridLayoutEx(5, this);
    _layout->setStyleSheet("background-color: rgb(55, 55, 55)");

    init();
}
DiscountFrame::~DiscountFrame() {}

void DiscountFrame::resizeEvent(QResizeEvent *e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();
    _layout->setGeometry(0, 0, rect.width(), rect.height());

    qDebug() << " discount frame size: " << _layout->geometry();
}

void DiscountFrame::init() {
    _layout->clean();

    for(int i = 0; i <= 50; i+=5) {
        VKeyItemButton* btn = new VKeyItemButton(QString("discount:%1").arg(i), _target, _layout);
        btn->setText(QString::number(i) + "%");

        if( i % 2 == 0 ) {
            btn->setStyleSheet("background-color: rgb(110, 110, 110)");
        } else {
            btn->setStyleSheet("background-color: rgb(125, 125, 125)");
        }

        _layout->addWidget(btn);
    }
    //增加一个免单的处理方式
    VKeyItemButton* btn = new VKeyItemButton(QString("discount:%1").arg(100), _target, _layout);
    btn->setText(QString::number(100) + "%");
    _layout->addWidget(btn);
}


void DiscountFrame::retranslateUI() {
    this->init();
}

///////////////////////////////////////////////////////////
/// \brief DiscountComboBox::DiscountComboBox
/// \param pWnd
/// \param parent
///

DiscountComboBox::DiscountComboBox(MoreSettingFrame* pWnd, QWidget* parent) : QComboBox(parent), _pWnd(pWnd)
{

}
DiscountComboBox::~DiscountComboBox(){}


void DiscountComboBox::showPopup() {
    qDebug() << "show popup ...";
    //显示比率窗口
    _pWnd->showDiscountFrame();
}

///////////////////////////////////////////////////////////
/// \brief MoreSettingFrame::MoreSettingFrame
/// \param order
/// \param parent
///
MoreSettingFrame::MoreSettingFrame(OrderBean* order, QWidget *parent) :
    QFrame(parent)
  , _order(order)
{
    qDebug() << "MoreSettingFrame::init";
//    this->setStyleSheet("background-color: rgb(90, 126, 133)");

    _layout = new GridLayoutEx(2, this);
    _layout->setObjectName("更多设置");
//    _layout->setStyleSheet("background-color: rgb(66, 66, 66)");
    _layout->setStyleSheet("background-color: rgb(90, 126, 133)");
    _layout->setRowHeight(80);

    _done = new VKeyItemButton("OK", this, this);
    _done->setText(QObject::tr("完成"));
    _done->setStyleSheet("background-color: rgb(120, 88, 120)");

    _cancel = new VKeyItemButton("Cancel", this, this);
    _cancel->setText(QObject::tr("放弃"));
    _cancel->setStyleSheet("background-color: rgb(120, 88, 120)");

    //只能是 menuframe 作为其父窗口
    _cookingTimeFrm = new CookingTimeFrame(this->parentWidget());
    this->connect(_cookingTimeFrm, SIGNAL(onCookingTimeSet(const QString&)), this, SLOT(onCookingTimeSet(const QString&)));

    _cookingTimeFrm->hide();

    _discountFrm = new DiscountFrame(this, this->parentWidget());
    _discountFrm->hide();

    _flavorFrm = new FlavorSettingFrame(this->parentWidget());

    this->connect(_flavorFrm, SIGNAL(done(const QString&)), this, SLOT(flavorSettingDone(const QString&)));
    _flavorFrm->hide();

    _splitOrderFrm = new SplitOrderFrame(this, this->parentWidget());
    this->connect(_splitOrderFrm, SIGNAL(done(int)), this, SLOT(onDoneSplit(int)));
    _splitOrderFrm->hide();

    init();
    qDebug() << "MoreSettingFrame::init end";
}

MoreSettingFrame::~MoreSettingFrame()
{
}
void MoreSettingFrame::init() {
    _layout->clean();

    // -行2个单元
    {
        POSToolButtonEx* _cooktime = new POSToolButtonEx(_layout);
        _cooktime->setObjectName("CookingTime");
        this->connect(_cooktime, SIGNAL(released()), this, SLOT(onCookTimeClicked()));
        _cooktime->setText(QObject::tr("预约时间"));

        _layout->addWidget(_cooktime);
    } {
        POSToolButtonEx* _save = new POSToolButtonEx(_layout);
        _save->setText(QObject::tr("保存用户地址"));
        _save->setObjectName("SaveConsumerAddr");
        this->connect(_save, SIGNAL(released()), this, SLOT(onSaveConsumerAddrClicked()));

        _layout->addWidget(_save);
    }

    {
        POSToolButtonEx* _print = new POSToolButtonEx(_layout);
        _print->setText(QObject::tr("本地打印"));

        _layout->addWidget(_print);
    } {
//        //
//        QLabel* _placeholder = new QLabel(_layout);

//        _layout->addWidget(_placeholder);

        VKeyItemButton* om = new VKeyItemButton("OrderMark", this, _layout);
        om->setText(QObject::tr("整单备注"));
        _layout->addWidget(om);
    }

    {
        VKeyItemButton* _splitorder = new VKeyItemButton("SplitOrder", this, _layout);
        _splitorder->setText(QObject::tr("折单"));
        _layout->addWidget(_splitorder, 1, 1, IDSPLITORDER);
    } {
        QLabel* _placeholder = new QLabel(_layout);

        _layout->addWidget(_placeholder);
    }


    {
        _discount = new DiscountComboBox(this, _layout);
        _discount->addItem("0%");
        _discount->setMinimumHeight(50);
        //当点击下拉事件时，显示比率窗口
//        QGroupBox *_gb = new QGroupBox(this);
//        _gb->setTitle(QObject::tr("折扣"));
//        QVBoxLayout *vbox = new QVBoxLayout;
//        vbox->addWidget(_discount);
//        _gb->setLayout(vbox);

        _layout->addWidget(_discount);

    } {
        QLabel* _placeholder = new QLabel(_layout);

        _layout->addWidget(_placeholder);
    }
}

void MoreSettingFrame::retranslateUI() {
    this->init();

    this->_discountFrm->retranslateUI();
}

void MoreSettingFrame::resizeEvent(QResizeEvent *e) {
    QFrame::resizeEvent(e);

    QRect frame = gWnd->geometry();
    QRect rect = this->frameGeometry();

    _layout->setRowHeight( rect.height() / 6);
    _layout->setGeometry(0, 0, rect.width(), rect.height() - 60);
    _done->setGeometry(rect.width() - 160, rect.height() - 55, 60, 50);
    _cancel->setGeometry(rect.width() - 80, rect.height() - 55, 60, 50);

    _cookingTimeFrm->setGeometry(rect.right() + 1, rect.top(), frame.width() - rect.right() -1 , frame.height());
    _discountFrm->setGeometry(rect.right()+ 1, rect.top(), frame.width() - rect.right() -1, frame.height());
    _splitOrderFrm->setGeometry(rect.right()+ 1, rect.top(), frame.width() - rect.right() -1, 300);
    _flavorFrm->setGeometry(rect.right()+ 1, rect.top(), frame.width() - rect.right() -1, 600);
}

void MoreSettingFrame::onCookTimeClicked() {
    if( _cookingTimeFrm->isHidden() == false) {
        _cookingTimeFrm->onCancel();
        return;
    }
    _discountFrm->hide();
    _splitOrderFrm->hide();

    QRect rect = this->geometry();
    qDebug() << "more setting frame rect: " << rect;

    _cookingTimeFrm->show();
}

void MoreSettingFrame::onSaveConsumerAddrClicked() {
    if(this->_order->sid.startsWith("QD-") == false) {
        return;
    }

    QString phone = this->_order->sid.mid(3);
    QString addr = this->_order->addr;

    int ret = DataThread::inst().saveConsumerAddress( phone, addr );
    if(0 == ret){
        QMessageBox::information(this, tr("提示"), tr("客户信息保存成功"));
    } else {
        QMessageBox::warning(this, tr("出错"), tr("客户信息保存失败, code: ") + QString::number(ret));
    }
}
void MoreSettingFrame::onCookingTimeSet(const QString& time) {
    qDebug() << "MoreSettingFrame::onCookingTimeSet: " << time;

    _order->cookingTime = time;
    //
    POSToolButtonEx* btn = dynamic_cast<POSToolButtonEx*>(_layout->getItembyObjectName("CookingTime"));

    btn->setText( QObject::tr("预约时间") + ":\n" + time.mid(11).mid(0, 5) );

}

void MoreSettingFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    _layout->getItembyObjectName("SaveConsumerAddr")->setEnabled(
                this->_order->sid.startsWith("QD-"));
}
void MoreSettingFrame::hideEvent(QHideEvent* e) {
    QFrame::hideEvent(e);

    _cookingTimeFrm->onCancel();

    _discountFrm->hide();
}


void MoreSettingFrame::showDiscountFrame() {
    if(_discountFrm->isHidden() == false) {
        _discountFrm->hide();
        return;
    }
    _cookingTimeFrm->hide();
    _splitOrderFrm->hide();

    _discountFrm->show();
}

void MoreSettingFrame::flavorSettingDone(const QString& flavor ) {
    this->_order->mark = flavor;
    this->_flavorFrm->hide();
}

void MoreSettingFrame::onKeyDown(const QString& value) {
    if("OK" == value) {
        //完成
        _cookingTimeFrm->hide();
        _discountFrm->hide();
        _splitOrderFrm->hide();
        _flavorFrm->hide();

        this->hide();

        //打折
        QString dis = _discount->currentText();
        this->_order->discount = dis.mid(0, dis.length() - 1).toInt() / 100.0;

        emit(onMoreDone());

    } else if("Cancel" == value) {
        //放弃,需要重置当前的设置

        this->_order->cookingTime = "";

        QToolButton* btn = dynamic_cast<QToolButton*>(_layout->getItembyObjectName("CookingTime"));
        btn->setText( QObject::tr("预约时间" ));

        _cookingTimeFrm->onCancel();
        _discountFrm->hide();
        _splitOrderFrm->hide();
        _flavorFrm->hide();

        this->hide();
    } else if("OrderMark" == value) {
        //整单备注
        _cookingTimeFrm->hide();
        _discountFrm->hide();
        _splitOrderFrm->hide();

        _flavorFrm->show();

    } else if("SplitOrder" == value) {
        //折单操作
        _cookingTimeFrm->hide();
        _discountFrm->hide();
        _flavorFrm->hide();

        _splitOrderFrm->show();

    } else if(value.startsWith("discount:")) {
        _discount->setItemText(0, QString("%1%").arg(value.mid(QString("discount:").length()).toInt() ) );
        qDebug() << " update discount ok, discount: " << value.toInt();

    }
}

void MoreSettingFrame::updateOrder(OrderBean* ob) {
    this->_order = ob;
    //相当于清空一下环境
//    onKeyDown("Cancel");

    _cookingTimeFrm->onCancel();
    _discountFrm->hide();
    _splitOrderFrm->hide();
    _flavorFrm->hide();
    //如果之前有折扣，则显示当前折扣

    POSToolButtonEx* btn = dynamic_cast<POSToolButtonEx*>(_layout->getItembyObjectName("CookingTime"));
    if( (ob->status == POSDATA::OD_BOOKING_SHARE || ob->status == POSDATA::OD_BOOKING_TAKEUP)
          &&  ob->cookingTime.length() > 0) {
        btn->setText( QObject::tr("预约时间") + ":\n" + ob->cookingTime.mid(11).mid(0, 5) );
    } else {
        btn->setText( QObject::tr("预约时间") );
    }

    VKeyItemButton* kb = dynamic_cast<VKeyItemButton*>(_layout->getItembyID(IDSPLITORDER));
    if( ob->subOids.length() > 1) {
        kb->setText( tr("折单") + "\n" + QString::number(ob->subOids.length()));
    } else {
        kb->setText( tr("折单") );
    }

    qDebug() << "discount: " << ob->discount;
    _discount->setItemText(0, QString("%1%").arg(ob->discount * 100 ) );
}

void MoreSettingFrame::onDoneSplit(int count) {
    qDebug() << "split order to " << count;
    if( count < 0) {
        //1个拆单没意义，0无效
        qDebug() << "MoreSettingFrame::onDoneSplit 无效的拆单数量";
        return;
    } else if( count < this->_order->subOids.length()) {
        //删除多余的
        int i = count - 1;
        if( i < 0) i = 0;

        for( ; i < this->_order->subOids.length(); i++) {
            if(0 != DataThread::inst().deleteSubOrder(this->_order->oid, this->_order->subOids.at(i))) {
                QMessageBox::warning(this, tr("出错"), tr("删除多余拆单号失败"));
                continue;
            }
            this->_order->subOids.pop_back();
        }

    } else if( count > this->_order->subOids.length()) {
    //保存数据库
        QList<QVariant> soids = DataThread::inst().newSubOrder( this->_order->oid, count - this->_order->subOids.length() );
        foreach (QVariant soid, soids) {
            this->_order->subOids.push_back( soid.toInt());
        }
    }


    OrderPainter::Print(*this->_order, count, DataThread::inst().getCashierPrinter(), true);

    if(1 < this->_order->subOids.length()) {
        VKeyItemButton* kb = dynamic_cast<VKeyItemButton*>(_layout->getItembyID(IDSPLITORDER));
        kb->setText( tr("折单") + "\n" + QString::number(count));
    }
}
