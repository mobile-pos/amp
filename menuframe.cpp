#include "menuframe.h"

#include <iterator>

#include <QLineEdit>
#include <QMessageBox>
#include <QDateTime>
#include <QDebug>

#include "mainwindow.h"
#include "datathread.h"
#include "menuitemexframe.h"

extern MainWindow* gWnd;

//进程刚启动 检查是否需要升级
#define APPNOTIFYISSTARTED "app:notify:event:process:start"

/////////////////////////////////////////////////
CategoryButton::CategoryButton(int cid,
                               const QString& cname,
                               MenuFrame* pwnd,
                               QWidget* parent)
    : POSToolButtonEx(parent)
    , m_cid( cid )
    , m_cname(cname)
    , m_pWnd(pwnd)
{
    this->connect(this, SIGNAL(changeCategory(int, const QString&)), m_pWnd, SLOT(changeCategory(int, const QString&)));
}
void CategoryButton::onClicked() {
    emit(changeCategory(m_cid, m_cname));
}


///================================================
MenuFrame::MenuFrame(QWidget *parent) :
    QFrame(parent),
    _cdFrm(nullptr)
{
    this->setObjectName("点菜主页面");
    this->setStyleSheet("background-color: rgb(224, 224, 224)");
//    this->setStyleSheet("background-color: rgb(88, 88, 88)");

    _leftBottomPane = new GridLayoutEx(10, this);
    _leftBottomPane->setObjectName(QString::fromUtf8("订单操作面板"));

//    _leftBottomPane->setStyleSheet("background-color: rgb(88, 88, 88)");

    _leftBottomPane->setRowHeight(45);
    _leftBottomPane->setCellBord(0);

    _shopcart = new ShopcartView(_leftBottomPane);
    _shopcart->setObjectName(QString::fromUtf8("购物车"));
    {
        connect(_shopcart, SIGNAL(onFoodItemAddCount()), this, SLOT(on_m_ItemCountAdd_clicked()));
        connect(_shopcart, SIGNAL(onFoodItemSubCount()), this, SLOT(on_m_itemCountSub_clicked()));
    }

//    _shopcart->setStyleSheet("background-color: rgb(160, 160, 154)");

    _catePane = new GridLayoutEx(6, this);
    _catePane->setObjectName(QString::fromUtf8("菜品类别操作面板"));

//    _cateCodePane = new GridLayoutEx(2, this);
//    _cateCodePane->setObjectName(QString::fromUtf8("菜类编码"));
//    _catePane->setStyleSheet("background-color: rgb(100, 80, 100)");

    _searchCnt = new LineEditEx(this, true);
    _searchCnt->clear();
    _searchCnt->setPlaceholderText("请输入菜码或英文菜名");

    _searchBtn = new VKeyItemButton("Search", this, this);
    _searchBtn->setText(QObject::tr("搜索"));
    _searchBtn->setFocus();

    _menuPane = new GridLayoutEx(4, this);
    _menuPane->setObjectName(QString::fromUtf8("菜品操作面板"));
//    _menuPane->setStyleSheet("background-color: rgb(222, 223, 219)");
    _menuPane->setCellBord(3);

    _flavorFrm = new FlavorSettingFrame(this);
    _flavorFrm->hide();

    this->connect(_flavorFrm, SIGNAL(done(const QString&)), this, SLOT(flavorSettingDone(const QString&)));
    this->connect(_flavorFrm, SIGNAL(cancel()), this, SLOT(flavorSettingCancel()));

    _moreFrm = new MoreSettingFrame(&this->m_ob, this);
    _moreFrm->setObjectName("更多设置项");
    this->connect(_moreFrm, SIGNAL(onMoreDone()), this, SLOT(onMoreDone()));
    _moreFrm->hide();

    _payFrm = new OrderPayFrame(&this->m_ob, this);
    _payFrm->setObjectName("付款");
    _payFrm->hide();
    this->connect(_payFrm, SIGNAL(onFinishPay()), this, SLOT(onFinishPay()));

    _consumerFrm = new ConsumerTypeFrame(this);
    _consumerFrm->setObjectName("类型变更");
    _consumerFrm->hide();
    this->connect(_consumerFrm, SIGNAL(done(const QString&, const QString&, const QString&)), this, SLOT(consumerTypeChangedDone(const QString&, const QString&, const QString&)));

    _disFrm = new DiscountFrame(this, this);
    _disFrm->setObjectName("单项菜品打折");

    _disFrm->hide();

    init();

    _rediscli = new RedisClient(this);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);
    qDebug() << "MenuFrame init end";
}

MenuFrame::~MenuFrame()
{
}


void MenuFrame::init() {

    qDebug() << "MenuFrame init ...";
    _leftBottomPane->addWidget(_shopcart, 10, 18);
    {

        QLabel* _lname = new QLabel(_leftBottomPane);
        _leftBottomPane->addWidget(_lname, 10, 1);
    }
    //上下翻页, 一行，10个单元
    QString bgcolor = "background-color: rgb(110, 110, 110)";
    {
//        _leftBottomPane->addWidget(new QLabel(_leftBottomPane), 3, 2);
//        _leftBottomPane->addWidget(new QLabel(_leftBottomPane), 3, 2);

        QLabel* mark = new QLabel(tr("整单备注:"), _leftBottomPane);
        mark->setObjectName("OrderMark");
        _leftBottomPane->addWidget(mark, 6, 2);
    }
    //右则是小计
    {
        GridLayoutEx* _xiaoji = new GridLayoutEx(2, _leftBottomPane);
        _xiaoji->setStyleSheet(bgcolor);
        _xiaoji->setRowHeight(20);
        {
            POSLabelEx* _xj = new POSLabelEx(_xiaoji);
            _xj->setText(QObject::tr("小计："));
            _xj->setAlignment(Qt::AlignRight);
            _xiaoji->addWidget(_xj);
        } {
            m_subamountTE = new QLineEdit(_xiaoji);
            m_subamountTE->setStyleSheet("background-color: rgb(238, 238, 233)");
            m_subamountTE->setText("");
            m_subamountTE->setReadOnly(true);
            _xiaoji->addWidget(m_subamountTE);
        } {
            POSLabelEx* _xj = new POSLabelEx(_xiaoji);
            _xj->setText(QObject::tr("税费："));
            _xj->setAlignment(Qt::AlignRight);
            _xiaoji->addWidget(_xj);
        } {
            m_taxTE = new QLineEdit(_xiaoji);
            m_taxTE->setStyleSheet("background-color: rgb(238, 238, 233)");
            m_taxTE->setText("");
            m_taxTE->setReadOnly(true);
            _xiaoji->addWidget(m_taxTE);
        } {
            POSLabelEx* _xj = new POSLabelEx(_xiaoji);
            _xj->setText(QObject::tr("折扣："));
            _xj->setAlignment(Qt::AlignRight);
            _xiaoji->addWidget(_xj);
        } {
            m_discountTE = new QLineEdit(_xiaoji);
            m_discountTE->setStyleSheet("background-color: rgb(238, 238, 233)");
            m_discountTE->setText("");
            m_discountTE->setReadOnly(true);
            _xiaoji->addWidget(m_discountTE);
        } {
            POSLabelEx* _xj = new POSLabelEx(_xiaoji);
            _xj->setText(QObject::tr("数量："));
            _xj->setAlignment(Qt::AlignRight);
            _xiaoji->addWidget(_xj);
        } {
            m_totalCount = new QLineEdit(_xiaoji);
            m_totalCount->setStyleSheet("background-color: rgb(238, 238, 233)");
            m_totalCount->setText("0");
            m_totalCount->setReadOnly(true);
            _xiaoji->addWidget(m_totalCount);
        } {
            POSLabelEx* _xj = new POSLabelEx(_xiaoji);
            _xj->setText(QObject::tr("合计："));
            _xj->setAlignment(Qt::AlignRight);
            _xiaoji->addWidget(_xj);
        } {
            m_amountTE = new QLineEdit(_xiaoji);
            m_amountTE->setStyleSheet("background-color: rgb(238, 238, 233)");
            m_amountTE->setText("");
            m_amountTE->setReadOnly(true);
            _xiaoji->addWidget(m_amountTE);
        }
        _leftBottomPane->addWidget(_xiaoji, 4, 5);
    }
    //左侧的上下翻页
    {
        POSToolButtonEx* _up = new POSToolButtonEx(_leftBottomPane);
        _up->setText(QObject::tr("往上"));
        _up->setStyleSheet(bgcolor);
        this->connect(_up, SIGNAL(released()), this, SLOT(onShopcartPageUpClicked()));
        _leftBottomPane->addWidget(_up, 3, 3);
    }
    {

        POSToolButtonEx* _down = new POSToolButtonEx(_leftBottomPane);
        _down->setText(QObject::tr("往下"));
        _down->setStyleSheet(bgcolor);
        this->connect(_down, SIGNAL(released()), this, SLOT(onShopcartPageDownClicked()));
        _leftBottomPane->addWidget(_down, 3, 3);
    }
    //一行，10个单元
    bgcolor = "background-color: rgb(125, 125, 125)";
    {
        POSToolButtonEx* _change = new POSToolButtonEx(_leftBottomPane);
        _change->setObjectName("changeType");
        _change->setText(QObject::tr("变更"));
        _change->setStyleSheet(bgcolor);
        this->connect(_change, SIGNAL(released()), this, SLOT(on_consumertype_change()));
        _leftBottomPane->addWidget(_change, 1, 2);
    }
    {
        QLabel* _lname = new POSLabelEx(_leftBottomPane);
        _lname->setText(QObject::tr("名称"));
        _lname->setStyleSheet(bgcolor);
        _leftBottomPane->addWidget(_lname, 1, 2);

        _name = new QLineEdit(_leftBottomPane);
        _name->setText(m_ob.sid);
        _name->setReadOnly(true);
        _name->setStyleSheet(bgcolor);
        _leftBottomPane->addWidget(_name, 3, 2);
    }


    {
        POSLabelEx* _laddr = new POSLabelEx(_leftBottomPane);
        _laddr->setText(QObject::tr("地址"));
        _laddr->setStyleSheet(bgcolor);
        _leftBottomPane->addWidget(_laddr, 1, 2);

        _addr = new QTextEdit(_leftBottomPane);
        _addr->setText(m_ob.addr);
        _addr->setReadOnly(true);
        _addr->setStyleSheet(bgcolor);
        _leftBottomPane->addWidget(_addr, 4, 2);
    }

    //增加菜品操作按钮, 一行，8个单元，另两个用于小计窗口
    bgcolor = "background-color: rgb(110, 110, 110)";
//    {
//        POSToolButtonEx* _del = new POSToolButtonEx(_leftBottomPane);
//        _del->setText(QObject::tr("删除"));
//        _del->setObjectName("deletItem");
//        _del->setStyleSheet(bgcolor);
//        this->connect(_del, SIGNAL(released()), this, SLOT(on_m_deleteItemInOrder_clicked()));
//        _leftBottomPane->addWidget(_del, 2, 2);
//    }
//    {
//        POSToolButtonEx* _add = new POSToolButtonEx(_leftBottomPane);
//        _add->setText(QObject::tr("份量(+1)"));
//        _add->setStyleSheet(bgcolor);
//        _add->setObjectName("addItemCount");
//        this->connect(_add, SIGNAL(released()), this, SLOT(on_m_ItemCountAdd_clicked()));
//        _leftBottomPane->addWidget(_add, 2, 2);
//    }
//    {
//        POSToolButtonEx* _sub = new POSToolButtonEx(_leftBottomPane);
//        _sub->setText(QObject::tr("份量(-1)"));
//        _sub->setStyleSheet(bgcolor);
//        _sub->setObjectName("subItemCount");
//        this->connect(_sub, SIGNAL(released()), this, SLOT(on_m_itemCountSub_clicked()));
//        _leftBottomPane->addWidget(_sub, 2, 2);
//    }
    {
        POSToolButtonEx* _flavor = new POSToolButtonEx(_leftBottomPane);
        _flavor->setText(QObject::tr("跟随项"));
        _flavor->setStyleSheet(bgcolor);
        _flavor->setObjectName("flavor");
        this->connect(_flavor, SIGNAL(released()), this, SLOT(on_m_flavor4Food_clicked()));
        _leftBottomPane->addWidget(_flavor, 2, 2);
    }
    {
        POSToolButtonEx* _discount = new POSToolButtonEx(_leftBottomPane);
        _discount->setText(QObject::tr("单项折扣"));
        _discount->setObjectName("discountItem");
        _discount->setEnabled(false);
        this->connect(_discount, SIGNAL(released()), this, SLOT(on_discount4Food_clicked()));
        _discount->setStyleSheet(bgcolor);
        _leftBottomPane->addWidget(_discount, 2, 2);
    }

    //增加订单操作按钮, 一行，8个单元，另两个用于小计窗口
    bgcolor = "background-color: rgb(125, 125, 125)";
    {
        POSToolButtonEx* _print = new POSToolButtonEx(_leftBottomPane);
        _print->setText(QObject::tr("打印"));
        _print->setStyleSheet(bgcolor);
        this->connect(_print, SIGNAL(released()), this, SLOT(on_m_printOrder_clicked()));
        _leftBottomPane->addWidget(_print, 3, 2);
    }
    {
        //预约下厨时间
        POSToolButtonEx* _more = new POSToolButtonEx(_leftBottomPane);
        _more->setText(QObject::tr("更多"));
        _more->setStyleSheet(bgcolor);
        _more->setObjectName("more");
        this->connect(_more, SIGNAL(released()), this, SLOT(on_moreSetting_clicked()));
        _leftBottomPane->addWidget(_more, 3, 2);
    }
//    {
//        POSToolButtonEx* _discount = new POSToolButtonEx(_leftBottomPane);
//        _discount->setText(QObject::tr("保留"));
//        _discount->setStyleSheet(bgcolor);
////        this->connect(_discount, SIGNAL(released()), this, SLOT(on_m_printOrder_clicked()));
//        _leftBottomPane->addWidget(_discount, 2, 2);
//    }
    {
        POSToolButtonEx* _back = new POSToolButtonEx(_leftBottomPane);
        _back->setText(QObject::tr("返回"));
        _back->setStyleSheet(bgcolor);
        this->connect(_back, SIGNAL(released()), this, SLOT(on_m_gotoHomeFrame_clicked()));
        _leftBottomPane->addWidget(_back, 4, 2);
    }
    {
        POSToolButtonEx* _pay = new POSToolButtonEx(_leftBottomPane);
        _pay->setText(QObject::tr("付款"));
        _pay->setObjectName("pay");
        _pay->setStyleSheet(bgcolor);
        this->connect(_pay, SIGNAL(released()), this, SLOT(on_m_orderPay_clicked()));
        _leftBottomPane->addWidget(_pay, 3, 2);
    }
    {
        POSToolButtonEx* _l = new POSToolButtonEx(_leftBottomPane);
        _l->setText(QObject::tr("下厨"));
        _l->setStyleSheet(bgcolor);
        _l->setObjectName("toCooking");
        this->connect(_l, SIGNAL(released()), this, SLOT(on_m_toCooking_clicked()));
        _leftBottomPane->addWidget(_l, 3, 2);
    }

}
void MenuFrame::setOrderBean(const OrderBean& ob) {
    m_ob = ob;

    if(m_ob.status == POSDATA::OD_REJECTOD
            || m_ob.status < POSDATA::OD_ACCESS
            || m_ob.status >= POSDATA::OD_ODPAIED) {
        updateTableStatus(false);
    } else {
        updateTableStatus(true);
    }

    _moreFrm->hide();
    _consumerFrm->hide();
    _flavorFrm->hide();
    _payFrm->hide();

    //只对2~14的进行在线编辑，15的只支持 支付小费录入
    _leftBottomPane->getItembyObjectName("changeType")->setEnabled(false);
//    _leftBottomPane->getItembyObjectName("deletItem")->setEnabled(false);
//    _leftBottomPane->getItembyObjectName("addItemCount")->setEnabled(false);
//    _leftBottomPane->getItembyObjectName("subItemCount")->setEnabled(false);
    _leftBottomPane->getItembyObjectName("flavor")->setEnabled(false);
    _leftBottomPane->getItembyObjectName("discountItem")->setEnabled(false);

    _leftBottomPane->getItembyObjectName("more")->setEnabled(false);
    _leftBottomPane->getItembyObjectName("pay")->setEnabled(false);
    _leftBottomPane->getItembyObjectName("toCooking")->setEnabled(false);

    if( ob.status == 1) {
        //网上订单，在接受前，只能预览
    } else if(ob.status >= 15 ){
        _leftBottomPane->getItembyObjectName("pay")->setEnabled(true);
    } else if( ob.status != POSDATA::OD_REJECTOD ) {
        _leftBottomPane->getItembyObjectName("changeType")->setEnabled(true);
//        _leftBottomPane->getItembyObjectName("deletItem")->setEnabled(true);
//        _leftBottomPane->getItembyObjectName("addItemCount")->setEnabled(true);
//        _leftBottomPane->getItembyObjectName("subItemCount")->setEnabled(true);
        _leftBottomPane->getItembyObjectName("flavor")->setEnabled(true);
        _leftBottomPane->getItembyObjectName("discountItem")->setEnabled(true);

        _leftBottomPane->getItembyObjectName("more")->setEnabled(true);
        _leftBottomPane->getItembyObjectName("pay")->setEnabled(true);
        _leftBottomPane->getItembyObjectName("toCooking")->setEnabled(true);
    }

    _name->setText(m_ob.sid);
    _addr->setPlainText(m_ob.addr);

    {
        QLabel* mark = dynamic_cast<QLabel*>(_leftBottomPane->getItembyObjectName("OrderMark"));
        mark->setText(tr("备注:") + ob.mark );
    }

    //清空小计区间的显示
    this->m_subamountTE->setText( "$0");
    this->m_taxTE->setText( "0%");
    this->m_discountTE->setText( "0%");
    this->m_amountTE->setText( "$0");
}
void MenuFrame::rowSelected(const QModelIndex &) {
    qDebug() << " good row selected ";
}

void MenuFrame::on_m_gotoHomeFrame_clicked()
{
    gWnd->Home();
}

void MenuFrame::resizeEvent(QResizeEvent *event) {
    QFrame::resizeEvent(event);

    /**
+------------------------------------------------+
|
|   order tree view |     category gridlayout ex |
|
|-------------------|     -----------------------|
|                   |      [search content] [ok] |
|+-----left pane----|     food menu gridlayout ex|
||base info     +   |                            |
||food item opt +   |                            |
||order opt     +   |                            |
+-------------------------------------------------+
     */


    QRect frame = this->geometry();

    int lw = frame.width() * 0.4;
    int rw = frame.width() - lw;
    int rch = (int)(frame.height() * 0.3);

    qDebug() << "MenuFrame resize event be called ...";

    _leftBottomPane->setGeometry(0, 0, lw, frame.height());
    _leftBottomPane->setRowHeight( frame.height() / 30 );

    qDebug() << "left bottom pane " << _leftBottomPane->geometry();

    QRect left = _leftBottomPane->geometry();

    int ccpw = 0 ; //100;
//    _cateCodePane->setRowHeight( frame.height() / 20 );
//    _cateCodePane->setGeometry( lw, 0, ccpw, frame.height() );

    lw += ccpw;
    rw -= ccpw;

    _catePane->setRowHeight( rch / 4 ); //只有三行
    _catePane->setGeometry(lw, 0, rw, rch);

    int hsearch = 30;
    _searchCnt->setGeometry(frame.right() - 230, rch, 160, hsearch);
    _searchBtn->setGeometry(frame.right() - 65, rch , 60, hsearch);

    int rmt = rch + 10 + hsearch;
    _menuPane->setRowHeight( (frame.height() - rmt) / 4 ); // 5行
    _menuPane->setGeometry(lw, rmt, rw, frame.height() - rmt);

    QRect frect(left.right(), left.top(), frame.width() - left.width(), frame.height());

    _flavorFrm->setGeometry(frect);

    _payFrm->setGeometry(frect);

    _disFrm->setGeometry(frect);

    _consumerFrm->setGeometry(left.left(), left.top(), left.width(), left.height() * 0.7);

    qDebug() << "category pane: " << _catePane->geometry().height() << ", width: " << frame.height() * 0.4;
}
void MenuFrame::showEvent(QShowEvent *e) {
    QFrame::showEvent(e);

    //最后是后台线程来处理该数据
    updateCategories();

    //更新购物车
    updateReceipts();

    //如果是外卖，需要弹出变更类型，以便输入电话和地址
    if(this->m_ob.sid.startsWith("QD-")
            && this->m_ob.oid == 0) {
        this->_consumerFrm->show();

        if(this->m_ob.sid.endsWith("null")) {
            this->_consumerFrm->updateCurName(this->m_ob.sid, "", "");
        } else {
            this->_consumerFrm->updateCurName(this->m_ob.sid,
                                              this->m_ob.sid.mid(3),
                                              "");
        }
    }
    //预约订单
    if(this->m_ob.status == POSDATA::OD_BOOKING_SHARE && this->m_ob.oid == 0) {
        this->on_moreSetting_clicked();
    }

    _searchBtn->setFocus();
}
void MenuFrame::hideEvent(QHideEvent *e) {
    QFrame::hideEvent(e);

    _shopcart->clear();


}

void MenuFrame::hide() {
    if(this->m_ob.status == POSDATA::OD_BOOKING_SHARE && this->m_ob.cookingTime.length() == 0) {
        QMessageBox::warning(this, tr("提示"), tr("预约的订单，需要指定预约时间"));

        return;
    }
    //
    GoodsChanged();
    if(this->m_ob.foods.length() > 0 && this->m_ob.status < POSDATA::OD_ODPAIED) {
        newOrder();
    }


    QFrame::hide();
}


void MenuFrame::retranslateUI() {
    this->updateCategories();

    if(this->_flavorFrm->isHidden() == false) {
        this->_flavorFrm->hide();
    }
    if(this->_moreFrm->isHidden() == false) {
        this->_moreFrm->hide();
    }
    if(this->_disFrm->isHidden() == false) {
        this->_disFrm->hide();
    }
    if(this->_consumerFrm->isHidden() == false) {
        this->_consumerFrm->hide();
    }

    this->_flavorFrm->retranslateUI();
    this->_moreFrm->retranslateUI();

    this->_disFrm->retranslateUI();
    this->_consumerFrm->retranslateUI();
}

void MenuFrame::updateReceipts() {
    if( m_ob.oid == 0 ) {
        return;
    }

    foreach (FoodBean fb, m_ob.getGoods()) {
        _shopcart->addFoodItem(fb);
    }

    GoodsChanged();
}

QWidget* MenuFrame::menuObjInCategories(int cid, int menuid) {
    if( this->m_goodsIncategory.count(cid) == 0) {
        return nullptr;
    }

    QString _mobjname = "m:" + QString::number(menuid);

    QList<QWidget*> clist = this->m_goodsIncategory[cid];
    foreach(QWidget* obj, clist) {
        if(obj->objectName() == _mobjname) {
            return obj;
        }
    }

    return nullptr;
}


void MenuFrame::changeCategory(int cid, const QString& cname) {
    _searchCnt->clear();
    _searchCnt->setPlaceholderText(QObject::tr("请输入菜码或英文菜名"));

    if(cid == -1) {
        updateRecommendationMenus();

    } else if( cid == -2 ) {
        updateReduceMenus();

    } else {
        updateFoodsBycid(cid, cname);
    }
}

void MenuFrame::onKeyDown(const QString& value) {

    qDebug() << "MenuFrame::onKeyDown: " << value;
    if(value.startsWith("c:")) {
        //选择了菜类
        int cid = value.mid(2).toInt();
        changeCategory(cid, "");

    } else if( value.startsWith("cc:")) {
        //直接选择菜码
        int cid = value.mid(3).toInt();
        changeCategory(cid, "");

    } else if(value.startsWith("m:")) {
        int id = value.mid(2).toInt();

        QVariantMap item = _foods[id].toMap();
        FoodBean fb;
        fb.id = item["id"].toInt();
        fb.name = item["name"].toString();
        fb.cid = item["cid"].toInt();
        fb.rid = item["rid"].toInt();
        fb.code = item["code"].toString();
        fb.unit_price = item["price"].toDouble();
        fb.maxCount = item["maxCount"].toInt();
        fb.count = 1;
        fb.icon = item["icon"].toString();

        addFoodItem(fb);

    } else if( "Search" == value) {
        _menuPane->clean();

        QList<QVariant> foods = DataThread::inst().search4Foods( _searchCnt->text() );
        foreach (QVariant menu, foods) {
            QVariantMap item = menu.toMap();

            addMenuItemtoMenuPanel(item);
        }

        _menuPane->resetPages();

        _searchBtn->setFocus();
    } else if(value.startsWith("discount:")) {
        //单项打折
        QList<FoodBean*> fbs = _shopcart->getSelectedFood();
        foreach(FoodBean* fb, fbs) {
            fb->discount = value.mid( QString("discount:").length() ).toInt() * 1.0 / 100;
        }
        _shopcart->updateFoodItemSelected();
        GoodsChanged();

        _disFrm->hide();
    }
}


void MenuFrame::updateCategories() {
    _catePane->clean();

//    _cateCodePane->clean();

    {
        VKeyItemButton* cb = new VKeyItemButton("c:" + QString::number(-1), this, _catePane);
        cb->setText(tr("店长推荐"));
        _catePane->addWidget(cb, 1, 1, -1);
    } {
        VKeyItemButton* cb = new VKeyItemButton("c:" + QString::number(-2), this, _catePane);
        cb->setText(tr("特价类"));
        _catePane->addWidget(cb, 1, 1, -1);
    }

    QVariantMap categories = DataThread::inst().loadCategories();

    int cateCount = categories.size();

    QVariantMap::iterator it = categories.begin();

    for(int i = 0; i< cateCount && it != categories.end(); i++, it++ ) {

        int id = it.key().toInt();
        QString name = it.value().toString();

        if( _catePane->getItembyID(id) == nullptr) {
            VKeyItemButton* cb = new VKeyItemButton("c:" + QString::number(id), this, _catePane);
            cb->setText(name);

            if( i % 5 % 2 == 0) {
                cb->setStyleSheet("background-color: rgb(112, 201, 228)");
            } else {
                cb->setStyleSheet("background-color: rgb(92, 164, 185)");
            }

            _catePane->addWidget(cb, 1, 1, id);
        }

//        {
//            VKeyItemButton* cb = new VKeyItemButton("cc:" + QString::number(id), this, _cateCodePane);
//            cb->setText(QString::number(id));
//            _cateCodePane->addWidget( cb );
//        }
    }

    if( cateCount > 0) {
        it = categories.begin();
//        updateFoodsBycid(it.key().toInt(), it.value().toString());
        changeCategory(-1, "");
    }

    _catePane->resetPages();
//    _cateCodePane->resetPages();
}

void MenuFrame::updateRecommendationMenus() {
    _menuPane->clean();

    QList<QVariant> menus = DataThread::inst().loadRecommendationMenus();
    foreach (QVariant menu, menus) {
        QVariantMap item = menu.toMap();

        addMenuItemtoMenuPanel(item);
    }

    _menuPane->resetPages();

}
void MenuFrame::updateReduceMenus() {
    _menuPane->clean();

    QList<QVariant> menus = DataThread::inst().loadSpecialMenus();
    foreach (QVariant menu, menus) {
        QVariantMap item = menu.toMap();

        addMenuItemtoMenuPanel(item);
    }

    _menuPane->resetPages();

}

void MenuFrame::addMenuItemtoMenuPanel(QVariantMap item) {
    int cid = -1; //没有分类

    int mid = item["id"].toInt();
    QString img = item["icon"].toString();

    if( _foods.count(mid) == 0) {
        _foods.insert(mid, item);
    }

    MenuItemExFrame* gib = dynamic_cast<MenuItemExFrame*>(menuObjInCategories(cid, mid) );
    if(nullptr != gib) {
        _menuPane->addWidget(gib, 1, 1, mid);
        return;
    }

    if( _menuPane->getItembyID( mid ) == nullptr ) {
        gib = new MenuItemExFrame( "m:" + QString::number(mid), this, _menuPane);

        qDebug() << "menu icon: " << DataThread::inst().getMenuIconPath() + img;

        gib->setRecommendation(item["recommendation"].toBool());
        gib->setPrice(item["price"].toDouble() );
        gib->setOldPrice(item["oldPrice"].toDouble());

        gib->setMenuItemName( item["name"].toString());
//        gib->setMenuItemIcon(DataThread::inst().getMenuIconPath() + img);
        gib->setMenuItemIconbyQUrl(QUrl( QString("%1%2").arg(LOCALWEBSERVER).arg(img)));
//        if( (++i % 5) % 2 == 0) {
//            gib->setBgColor(QColor(230, 235, 226));
//        } else {
//            gib->setBgColor( QColor(213, 214, 211) );
//        }

        gib->setSoldout( item["maxCount"].toInt() == 0);

        _menuPane->addWidget(gib, 1, 1, mid);

    } else {
        gib = dynamic_cast<MenuItemExFrame*>(_menuPane->getItembyID( mid ));
    }


    m_goodsIncategory[cid].push_back( gib) ;
}
void MenuFrame::updateFoodsBycid(int cid, const QString& cname) {

    _menuPane->clean();

    QList<QVariant> menus = DataThread::inst().loadMenuItemsByCategoryId(cid);
    foreach (QVariant menu, menus) {
        QVariantMap item = menu.toMap();

        addMenuItemtoMenuPanel(item);
    }

    _menuPane->resetPages();
}


void MenuFrame::addFoodItem(const FoodBean& food) {


    if(this->m_ob.sid.endsWith("-null")) {
        // invalidate sid
        QMessageBox::warning(this, "出错", "请确认: 无效客户订单[ " + this->m_ob.sid + "]");
        return;
    }

    if( food.maxCount == 0) {
        //已售完
        return;
    }
    //如果是已结算的订单，不能加菜品了
    if(m_ob.status >= POSDATA::OD_ODPAIED || m_ob.status == POSDATA::OD_REJECTOD) {
        return;
    }
    //如果已保存过的订单，要直接更新数据库
    else if(m_ob.status >= POSDATA::OD_ACCESS && m_ob.status < POSDATA::OD_ODPAIED) {
        FoodBean fb = food;
        fb.oid = m_ob.oid;

        fb.reid = DataThread::inst().addReceiptToOrder(m_ob.oid, fb.toMap());

         _shopcart->addFoodItem(fb);
    } else  {
        //订单都没生成，先生成订单
        m_ob.foods.push_back(food);
        newOrder();

         _shopcart->addFoodItem(*m_ob.foods.begin());
    }

    GoodsChanged();

}

void MenuFrame::onShopcartPageUpClicked() {
    _shopcart->pageUp();
}

void MenuFrame::onShopcartPageDownClicked() {
    _shopcart->pageDown();
}

void MenuFrame::on_discount4Food_clicked() {
    //单项打折

    showFrme(_disFrm);

}

void MenuFrame::on_m_printOrder_clicked()
{
    //先更新下再打印
    this->m_ob.status = POSDATA::OD_SUBMIT;

    newOrder();
    //打印订单及小票
    OrderPainter::Print(m_ob, 1, DataThread::inst().getCashierPrinter());
}

void MenuFrame::on_m_deleteItemInOrder_clicked()
{

    QList<FoodBean*> removed = _shopcart->removeFoodItemSelected();
    foreach(FoodBean* fb, removed) {
        if(fb->oid > 0 && fb->reid > 0) {
            if(0 != DataThread::inst().removeReceiptfromOrder(fb->oid, fb->reid)) {
                QMessageBox::warning(this, "出错", "删除当前菜品失败");
                continue;
            }
        }
    }

    GoodsChanged();
}

void MenuFrame::on_m_ItemCountAdd_clicked()
{

    QList<FoodBean*> fbs = _shopcart->getSelectedFood();
    foreach(FoodBean* fb, fbs) {
        fb->count += 1;
    }
    _shopcart->updateFoodItemSelected();

    GoodsChanged();
}

void MenuFrame::on_m_itemCountSub_clicked()
{
    QList<FoodBean*> fbs = _shopcart->getSelectedFood();
    foreach(FoodBean* fb, fbs) {
        fb->count -= 1;
        if( fb->count <= 0) {
            on_m_deleteItemInOrder_clicked();
        }
    }
    _shopcart->updateFoodItemSelected();

    GoodsChanged();
}

void MenuFrame::GoodsChanged() {

    double subamount = 0.0;
    double amount = 0.0;
    int count = 0;

    QList<FoodBean> fbs = _shopcart->allFoods();
    foreach(FoodBean fb, fbs) {
        subamount += (fb.count * fb.unit_price * (1 - fb.discount));
        count += fb.count;
    }
    //折后价格
//    subamount = subamount * (1 - this->m_ob.discount);

    //折后加税费
    amount = subamount * (1 - this->m_ob.discount)  + subamount * m_ob.taxRate; //这里不计算小费 + m_ob.gratuity;

//    amount -= amount * this->m_ob.discount;
    qDebug() << "sub amount: " << subamount << ", amount: " << amount;

    this->m_subamountTE->setText( "$" + QString::number(subamount, 10, 2));

    this->m_taxTE->setText( QString::number(m_ob.taxRate * 100) + "%");

    this->m_discountTE->setText( QString::number(this->m_ob.discount * 100) + "%");

    this->m_amountTE->setText( "$" + QString::number(amount, 10, 2));

    this->m_totalCount->setText(QString::number(count));

    m_ob.total = subamount;
    m_ob.foods = fbs;
    m_ob.amount = amount;

    if(nullptr != _cdFrm) {
        _cdFrm->updateOrder(m_ob);
    }
}

void MenuFrame::updateTableStatus(bool satdown) {
    QString type = this->m_ob.sid.mid(0, 3);
    if( type.startsWith("TC-") == false) {
        return;
    }
    //根据 sid ，看是不是堂吃
    //更改台桌标识
    QString sid = m_ob.sid.mid(3);
    QStringList tabs = sid.split("-");

    //设置桌子过期时间，即每天凌晨4点过期
    QDateTime today = QDateTime::currentDateTime();
    QDateTime tomorrow;
    tomorrow.setDate(today.date().addDays(1));
    tomorrow.setTime( QTime(4, 0, 0));

    foreach(QString tname, tabs) {

        int issatdown = satdown ? 1 : 0;
        if(0 == m_ob.oid) {
            //无效订单
            issatdown = 0;
        }

         _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + tname ,
                                              "name",
                                              tname));

         _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":"+ tname ,
                                              "satdown",
                                              QString::number(issatdown)));

         _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":"+ tname ,
                                              "time",
                                              QDateTime::currentDateTime().toString("yyyy-MM-dd hh:mm:ss")));

         _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":"+ tname ,
                                              "oid",
                                              satdown ? QString::number(this->m_ob.oid) : "0"));

         _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":"+ tname,
                                              "sid",
                                              satdown ? this->m_ob.sid : ""));

         //只有预约时间晚于当前订单时间半小以后，才更新
         QDateTime cookingTime = QDateTime::fromString(this->m_ob.cookingTime, "yyyy-MM-dd hh:mm:ss");
         QDateTime curtime = QDateTime::currentDateTime();
         //如果是预约时间，则计算预约时间未到时，可以让其它人占用
         if( cookingTime.isValid() && curtime.secsTo(cookingTime) > 0 &&
                 ( this->m_ob.status == POSDATA::OD_BOOKING_SHARE || this->m_ob.status == POSDATA::OD_BOOKING_TAKEUP )
            )
         {
             _rediscli->sendCommand(Command::HSET("res:info:tables:" + QString::number(gRID) + ":" + tname,
                                                  "cookingTime",
                                                  this->m_ob.cookingTime));
         }


         _rediscli->sendCommand(Command::EXPIRE("res:info:tables:" + QString::number(gRID) + ":" + tname , today.secsTo(tomorrow)));
    }
}

void MenuFrame::newOrder() {

    if(this->m_ob.sid.endsWith("-null")) {
        // invalidate sid
        QMessageBox::warning(this, "出错", "无效客户订单, 请确认: " + this->m_ob.sid);
        return;
    }

    if( this->m_ob.status >= POSDATA::OD_ODPAIED
            || this->m_ob.status == POSDATA::OD_REJECTOD ) { //已完成的订单或是 已是退单 不需要再更新
        updateTableStatus(false);
        return;
    }
    else if (this->m_ob.status == POSDATA::OD_RECVREQ) {
        //刚才接受，没审核，不能保存
        return;
    }
    else if( this->m_ob.status < POSDATA::OD_ACCESS) {
        this->m_ob.status = POSDATA::OD_ACCESS; //新增订单，直接下单，不像app，那样要确认
    }

    QVariantMap ret = DataThread::inst().newOrder( this->m_ob.toMap() );
    if( ret.empty() ) {
//        QMessageBox::warning(this, "出错", "订单保存失败，如果经常出错，请联系客服");
        return;
    }
    //同时更新小票 ID
    this->m_ob.oid = ret["oid"].toInt();

    QList<QVariant> reids = ret["reids"].toList();
    if( reids.length() != this->m_ob.foods.length() ) {
        //返回数据异常
        qDebug() <<"新增订单中返回的数据与请求的数据不一致";
        QMessageBox::warning(this, "出错", "新增订单中返回的数据与请求的数据不一致");
        return;
    }
    //更新小票的id值（非商品id)
    int fcount = this->m_ob.foods.length();
    for(int i = 0; i < fcount && i < reids.length(); i++ ) {
        //
        this->m_ob.foods[i].oid = this->m_ob.oid;
        this->m_ob.foods[i].reid = reids[i].toInt();
    }
    //更新桌台信息
    if(this->m_ob.status == POSDATA::OD_BOOKING_SHARE) {
        updateTableStatus(false);
    } else {
        updateTableStatus(true);
    }
}

void MenuFrame::on_m_flavor4Food_clicked()
{
    //显示跟随项

    showFrme(_flavorFrm);

    qDebug() << " show flavor dialog ..";
}

void MenuFrame::flavorSettingDone(const QString& flavor ){


    QList<FoodBean*> fbs = _shopcart->getSelectedFood();
    foreach(FoodBean* fb, fbs) {
        fb->flavor = flavor;
    }
    _shopcart->updateFoodItemSelected();
    GoodsChanged();
}

void MenuFrame::flavorSettingCancel( ){

}
void  MenuFrame::on_moreSetting_clicked() {

    QRect left = _leftBottomPane->geometry();

    _moreFrm->setGeometry(left.right() - 200, left.top(), 200, _shopcart->height());

    _moreFrm->updateOrder(&this->m_ob);
    showFrme(_moreFrm);

    qDebug() << " show more setting dialog ..";
}

void MenuFrame::on_m_toCooking_clicked() {
    //如果订单没有确认，则直接确认
    newOrder();
    //下发到厨房
    QList<QString> kitchprinter = DataThread::inst().getKitchPrinter();
    if( kitchprinter.length() == 0){
        QMessageBox::warning(nullptr, tr("提醒"), tr("当前没有配置厨房打印机，建议去设置：设置->打印机设置"));
        return;
    }
    if( kitchprinter.length() > 1 ) {
        //采用 菜品指定打印
        QList<FoodBean> fbs = this->m_ob.foods;
        foreach(FoodBean fb, fbs) {
            //不是打印订单
        }
    } else {
        OrderPainter::Print(m_ob, 1, kitchprinter.at(0));
    }

}

void MenuFrame::on_m_orderPay_clicked() {
    newOrder();
    _payFrm->updateOrder(&this->m_ob);
    showFrme(_payFrm);
}

void MenuFrame::showFrme(QFrame* frm) {
    if( frm->isHidden() == false) {
        frm->hide();
        return;
    }

    this->_moreFrm->hide();
    this->_flavorFrm->hide();
    this->_payFrm->hide();
    this->_consumerFrm->hide();
    this->_disFrm->hide();

    frm->show();
}

void MenuFrame::on_consumertype_change() {

    showFrme(_consumerFrm);
    _consumerFrm->updateCurName(this->m_ob.sid.mid(0, 3), this->m_ob.sid.mid(3, this->m_ob.sid.length() - 3), "");
}

void MenuFrame::consumerTypeChangedDone(const QString& type, const QString& name, const QString& addr) {
    //如果是占了桌位，侧变更后需要退桌
    if(this->m_ob.sid.startsWith("TC-") &&
            this->m_ob.sid != type + name) {
        //
        updateTableStatus(false);
    }
    this->m_ob.sid = type + name;
    this->m_ob.addr = addr;

    this->_name->setText(m_ob.sid);
    this->_addr->setPlainText(addr);

    updateTableStatus(true);
}

void MenuFrame::connected() {
    qDebug() << " menu frame connect to redis OK";

    _rediscli->sendCommand(Command::Auth(REDISAUTH));
}
void MenuFrame::disconnected() {
    usleep(2 * 1000 * 1000);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);
}

void MenuFrame::onReply(const QString& cmd, Redis::Reply value) {
    qDebug() << "onReply cmd: " << cmd <<", value: " << value.value().toString();

    QString key = cmd.toLower().trimmed();
    if(key.startsWith("auth")) {
        _rediscli->sendCommand(Command::PUBLISH(QString("%1:%2").arg(APPNOTIFYISSTARTED).arg(gRID), "1"));

    } else if(key.startsWith("hget res:info:tables:") == true && key.endsWith("cookingtime") == true) {
        //
        //比较 缓存的cookingTime与当前订单的cookingTime是否一致
        //
        // key -> hget res:info:tables:<tname> cookingTime

        QString field = key.mid( QString("hget").length() ).trimmed();
        field = field.split(" ").at(0);

        QString tname =  field.replace(0, QString("res:info:tables:").length(), "");
        tname = tname.simplified().toUpper();

        QString cookingTime = value.value().toString();

        QDateTime cacheBookingTime = QDateTime::fromString(cookingTime, "yyyy-MM-dd hh:mm:ss");
        QDateTime bookingOfOrder = QDateTime::fromString(this->m_ob.cookingTime, "yyyy-MM-dd hh:mm:ss");

        if( cacheBookingTime.isValid() && bookingOfOrder.isValid() && abs( bookingOfOrder.secsTo( cacheBookingTime) ) < 60 ) {
            //同一个订单，可以清缓存
            _rediscli->sendCommand(Command::HDEL("res:info:tables:" + tname, "cookingTime"));
        }
    }
}

void MenuFrame::onMoreDone() {
    //如果设置了预约时间，则提交设置预约过期时间，采用redis的 expired 来触发事件

    QDateTime ct = QDateTime::fromString(this->m_ob.cookingTime, "yyyy-MM-dd hh:mm:ss");

    QDateTime now = QDateTime::currentDateTime();

    if(now.secsTo( ct ) > 5 * 60) {
        //预约不能小于 5 min

        if(m_ob.oid > 0 &&
                0 != DataThread::inst().updateOrderStatusbysid(m_ob.sid,
                                                  m_ob.status,
                                                  POSDATA::OD_BOOKING_SHARE) ) {
            QMessageBox::warning(this, tr("出错"), tr("变更状态失败, 或者先选好菜品，再变更"));
            return;
        }

        m_ob.status = POSDATA::OD_BOOKING_SHARE;

        _rediscli->sendCommand(Command::SET("order:cookingtime:timeout:" + QString::number(gRID) + ":" + this->m_ob.sid, "1"));
        _rediscli->sendCommand(Command::EXPIRE("order:cookingtime:timeout:" + QString::number(gRID) + ":" + this->m_ob.sid, now.secsTo(ct)));

        //预约的桌子，半小前不占用座位
    }

    {
        //更新显示
        QLabel* mark = dynamic_cast<QLabel*>( this->_leftBottomPane->getItembyObjectName("OrderMark"));
        mark->setText(tr("备注:") + m_ob.mark );
    }

    //更多设置结果
    GoodsChanged();
}

void MenuFrame::onFinishPay() {
    if(this->m_ob.status < POSDATA::OD_FINISHORDER) {
        this->m_ob.status = POSDATA::OD_ODPAIED;
    }
    if(0 != DataThread::inst().updateOrderInfo( m_ob.toMap() )) {
        QMessageBox::warning(this, tr("出错"), "结单失败");
        return;
    }
    updateTableStatus(false);

    //清除 预约相关记录, 不能直接删除，需要判断订单的预约时间与缓存的时间是否一致，

    QString tnames = this->m_ob.sid.mid(QString("TC-").length());
    QStringList names = tnames.split("-");

    foreach(QString tname, names) {
        _rediscli->sendCommand(Command::HGET("res:info:tables:" + QString::number(gRID) + ":" + tname, "cookingTime"));
    }

    gWnd->Home()->on_m_allPaiedOrders_clicked();
}
