#include "shopcartview.h"

#include <QPushButton>
#include <QEvent>
#include <QPainter>
#include <QNetworkReply>

#include "datathread.h"

ShopcartItemBuddy::ShopcartItemBuddy(const FoodBean& fb, QWidget* parent)
    : QWidget(parent),
      _fb(fb) {
    _manager = new QNetworkAccessManager(this);

    connect(_manager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(replyFinished(QNetworkReply*)));

    _iconPath = _fb.icon;
    initUi();
}

void ShopcartItemBuddy::replyFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError)
    {
        //获取字节流构造 QPixmap 对象
        QPainter painter(_icon);
        _pixmap.loadFromData(reply->readAll());

        painter.drawImage(_icon->rect(), _pixmap.toImage());
        update();
    }
}

void ShopcartItemBuddy::initUi()
{
    //初始化
    _icon = new QWidget(this);
    _layout = new GridLayoutEx(20, this);
    _layout->setCellBord(5);

    _foodName = new QLabel(_layout);
    _priceInfo = new QLabel(_layout);
    _flavor = new QLabel(_layout);
    _amount = new QLabel(_layout);

    _layout->addWidget(_foodName, 12);
    _layout->addWidget(_amount, 2, 3);
    {
        QPushButton* add = new QPushButton(_layout);
        add->setText("Add\n(+1)");
        connect(add, SIGNAL(released()), this, SLOT(onAddItemCountOnSelected()));
        _layout->addWidget(add, 3, 3);
    }
    {
        QPushButton* sub = new QPushButton(_layout);
        sub->setText("Sub\n(-1)");
        connect(sub, SIGNAL(released()), this, SLOT(onSubItemCountOnSelected()));
        _layout->addWidget(sub, 3, 3);
    }

    _layout->addWidget(_priceInfo, 12);

    _layout->addWidget(_flavor, 12);


    _icon->setFixedSize(55,55);
    //设置个性签名字体为灰色
    QPalette color;

    color.setColor(QPalette::Text, {71, 64, 64});
    _flavor->setPalette(color);
    _flavor->setFont(QFont("仿宋", 7, QFont::Bold));

    color.setColor(QPalette::Text, {11, 33, 63});
    _priceInfo->setPalette( color);
    _priceInfo->setFont(QFont("仿宋", 7, QFont::Bold));

    color.setColor(QPalette::Text, {255, 32, 32});
    _amount->setPalette( color);
    _amount->setFont(QFont("仿宋", 7, QFont::Bold));
    _amount->setAlignment(Qt::AlignHCenter |Qt::AlignCenter);

    color.setColor(QPalette::Text, Qt::black);
    _foodName->setPalette(color);
    _foodName->setFont(QFont("仿宋", 10, QFont::Bold));

    //装载事件过滤器
    _icon->installEventFilter(this);

    qDebug() << "shop cart's image: " << _iconPath;
    _manager->get(QNetworkRequest(QUrl(QString("%1%2").arg(LOCALWEBSERVER).arg(_iconPath))));
}

bool ShopcartItemBuddy::eventFilter(QObject *obj, QEvent *event)
{
    if(obj == _icon )
    {
        if(event->type() == QEvent::Paint)
        {
            QPainter painter(_icon);
//            QString fpath = DataThread::inst().getMenuIconPath() +  _iconPath;

//            qDebug() << "food icon: " << fpath;
//            painter.drawImage(_icon->rect(), QImage(fpath));
            painter.drawImage(_icon->rect(), _pixmap.toImage());
        }
    }
    return QWidget::eventFilter(obj, event);
}

void ShopcartItemBuddy::resizeEvent(QResizeEvent *event)  {
    QWidget::resizeEvent(event);
/**
  [icon]  [food name ]              [total price for one item]
          [count]  [price ]
          [ flavor ]
*/
    QRect rect = this->geometry();
    _icon->setGeometry(2, 2, 55, 55);

    _layout->setRowHeight( rect.height()/3);
    _layout->setGeometry(60, 0, rect.width() - 60, rect.height());
}


void ShopcartItemBuddy::updateFoodItem() {

    _foodName->setText(_fb.name);
    _amount->setText( QString::number(_fb.count * _fb.unit_price * (1 - _fb.discount), 'f', 2) );

    QString price = QObject::tr("%1件 单价为 %2/件").arg(_fb.count).arg(_fb.unit_price);
    if(_fb.discount > 0) {
        price += QObject::tr(", 当前折扣: %1%").arg(_fb.discount * 100, 2);
    }

    _priceInfo->setText(price);
    _flavor->setText(_fb.flavor);
}

void ShopcartItemBuddy::activeBgColor() {
    this->_layout->setStyleSheet("background-color: rgb(145, 183, 233)");
}
void ShopcartItemBuddy::inActiveBgColor() {
    this->_layout->setStyleSheet("background-color: rgb(173, 173, 173)");
}



void ShopcartItemBuddy::onAddItemCountOnSelected() {
    emit( onAddItemCount(this)  );
}
void ShopcartItemBuddy::onSubItemCountOnSelected() {
    emit( onSubItemCount(this)  );
}

///////////////////////////////////////////
/// \brief ShopcartView::ShopcartView
/// \param parent
///
ShopcartView::ShopcartView(QWidget *parent) : QListWidget(parent)
{
    setFocusPolicy(Qt::NoFocus);       // 去除item选中时的虚线边框
    this->setHorizontalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//水平滚动条关闭
    this->setVerticalScrollBarPolicy(Qt::ScrollBarAlwaysOff);//水平滚动条关闭
    setSelectionMode(QAbstractItemView::SingleSelection);

    this->connect(this, SIGNAL(itemSelectionChanged()), this, SLOT(itemSelectionChanged()));
}
ShopcartView::~ShopcartView(){

}
void ShopcartView::clear() {
    //只要隐藏，即清空
    int count = this->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem* item = this->item(i);
        ShopcartItemBuddy* buddy = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(item));
        delete buddy;
        delete item;
    }

    QListWidget::clear();
}

QList<FoodBean*> ShopcartView::removeFoodItemSelected() {
    QList<FoodBean*> fbs;

    QList<QListWidgetItem*> sels = this->selectedItems();
    foreach (QListWidgetItem* sel, sels) {
        ShopcartItemBuddy* item = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(sel));

        fbs.push_back(& item->_fb);
        delete item;
        delete sel;
    }
    return fbs;
}

void ShopcartView::addFoodItem(const FoodBean& fb) {
    ShopcartItemBuddy* item = new ShopcartItemBuddy(fb);   //创建一个自己定义的信息类
    connect(item, SIGNAL(onAddItemCount(ShopcartItemBuddy*)), this, SLOT(onAddItemCount(ShopcartItemBuddy*)));
    connect(item, SIGNAL(onSubItemCount(ShopcartItemBuddy*)), this, SLOT(onSubItemCount(ShopcartItemBuddy*)));

    item->updateFoodItem();

    QListWidgetItem *newItem = new QListWidgetItem();
    newItem->setSizeHint( {this->width(), 80});

    this->insertItem(0, newItem); //将该newItem插入到后面
    this->setItemWidget(newItem, item); //将buddy赋给该newItem
    newItem->setHidden(false);

    this->setCurrentRow( 0 );
}

QList<FoodBean*> ShopcartView::getSelectedFood() {
    QList<FoodBean*> fbs;

    QList<QListWidgetItem*> sels = this->selectedItems();
    foreach (QListWidgetItem* sel, sels) {
        ShopcartItemBuddy* item = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(sel));
        fbs.push_back(& item->_fb);
    }

    return fbs;
}
void ShopcartView::updateFoodItemSelected() {
    QList<QListWidgetItem*> sels = this->selectedItems();
    foreach (QListWidgetItem* sel, sels) {
        ShopcartItemBuddy* item = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(sel));
        item->updateFoodItem();
    }
}

QList<FoodBean> ShopcartView::allFoods() {
    QList<FoodBean> fbs;
    int count = this->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem* item = this->item(i);
        ShopcartItemBuddy* buddy = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(item));
        fbs.push_back( buddy->_fb);
    }

    return fbs;
}

void ShopcartView::itemSelectionChanged() {

    int count = this->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem* item = this->item(i);
        ShopcartItemBuddy* buddy = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(item));
        if(nullptr == buddy) {
            continue;
        }
        buddy->inActiveBgColor();
    }

    QList<QListWidgetItem*> sels = this->selectedItems();
    foreach (QListWidgetItem* sel, sels) {
        ShopcartItemBuddy* item = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(sel));
        if(nullptr == item) {
            continue;
        }
        item->activeBgColor();
    }
}

void ShopcartView::pageUp() {

    int cur = this-> currentRow();

    if( cur < 5) {
        this->setCurrentRow( 0 );
        return;
    }

    this->setCurrentRow( cur - 5 );
}

void ShopcartView::pageDown() {
    int count = this->count();
    int cur = this-> currentRow();

    if( cur + 5 > count) {
        this->setCurrentRow( count - 1 );
        return;
    }

    this->setCurrentRow( cur + 5 );
}

void ShopcartView::onAddItemCount(ShopcartItemBuddy* item) {

    int count = this->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem* _it = this->item(i);
        ShopcartItemBuddy* buddy = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(_it));
        if(nullptr == buddy) {
            continue;
        }

        if(item == buddy) {
            _it->setSelected(true);
        } else {
            buddy->inActiveBgColor();
            _it->setSelected(false);
        }
    }

    emit(onFoodItemAddCount());
}

void ShopcartView::onSubItemCount(ShopcartItemBuddy* item) {
    int count = this->count();
    for(int i = 0; i < count; i++) {
        QListWidgetItem* _it = this->item(i);
        ShopcartItemBuddy* buddy = dynamic_cast<ShopcartItemBuddy*>(this->itemWidget(_it));
        if(nullptr == buddy) {
            continue;
        }

        if(item == buddy) {
            buddy->activeBgColor();
            _it->setSelected(true);
        } else {
            buddy->inActiveBgColor();
            _it->setSelected(false);
        }
    }

    emit(onFoodItemSubCount());
}
