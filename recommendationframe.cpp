#include "recommendationframe.h"
#include <QDebug>
#include <QMessageBox>

#include "vkeyboardex.h"
#include "datathread.h"
#include "menuitemexframe.h"

RecommendationFrame::RecommendationFrame(QWidget *parent) :
    QFrame(parent)
{
    _categories = new GridLayoutEx(5, this);
    _menuItems = new GridLayoutEx(4, this);

    _optFrm = new GridLayoutEx(4, this);
    _optFrm->setObjectName("估清操作面板");

    init();
}

RecommendationFrame::~RecommendationFrame()
{
}


void RecommendationFrame::init() {
    qDebug() << "ReduceMenuPriceFrame::init";

    {
        int id = -1;
        VKeyItemButton* cb = new VKeyItemButton("s:" + QString::number(id), this, _categories);

        cb->setText( tr("特价类"));
        cb->setStyleSheet("background-color: rgb(167, 119, 89)");

        _categories->addWidget(cb, 1, 1, id);
    }
    //opt frame
    {
        {
            POSLabelEx* _l = new POSLabelEx (QObject::tr("当前菜名："), _optFrm);
            _optFrm->addWidget(_l, 2);
            QLineEdit* _e = new QLineEdit("", _optFrm);
            _e->setEnabled(false);
            _optFrm->addWidget(_e, 2, 1, 10000);
        } {
            //是否为店长推荐
            POSCheckBoxEx* cb = new POSCheckBoxEx(QObject::tr("是否推荐"), _optFrm);
            _optFrm->addWidget(cb, 2, 1, 10001);

            QLabel* _l = new QLabel("", _optFrm);
            _optFrm->addWidget(_l, 2);
        }{
            QLabel* _l = new QLabel("", _optFrm);
            _optFrm->addWidget(_l, 4, 1, 1);
        } {
            VKeyItemButton* _btn = new VKeyItemButton("submit", this, _optFrm);
            _btn->setText(QObject::tr("提交"));
            _optFrm->addWidget(_btn, 4);
        }
    }

    qDebug() << "ReduceMenuPriceFrame::init end";
}

void RecommendationFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->frameGeometry();

    int lw = rect.width() * 0.6;
    int ch = rect.height() * 0.4;
    int mh = rect.height() - ch;

    _categories->setRowHeight( ch / 4);
    _categories->setGeometry(0, 0, lw, ch );

    _menuItems->setRowHeight(mh / 4);
    _menuItems->setGeometry(0, ch, lw, mh);

    _optFrm->setRowHeight( rect.height() / 20);
    _optFrm->setGeometry(lw, 0, rect.width() - lw, rect.height());
//    qDebug() << " estimate clearly size: " << _layout->geometry();
}

void RecommendationFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    updateCategories();
    changeCategory(-1, "");
}


void RecommendationFrame::onKeyDown(const QString& value) {
    qDebug() << "onKeyDown: " << value;
    if(value.startsWith("c:")) {
        //更改了类别
        this->changeCategory(value.mid(2).toInt(), "");
    } else if(value.startsWith("m:")) {
        //选择了菜品
        //获取菜品信息
        qDebug() <<"获取菜品信息: " << value.mid(2);

        QVariantMap menu = DataThread::inst().loadMenuByfoodId( value.mid(2).toInt() );
        if(menu.empty()) {
            QMessageBox::warning(this, "出错", "没找到指定的菜品" );
            return;
        }
        int fid = menu["id"].toInt();
        QString name = menu["name"].toString();

        QLineEdit* _e = dynamic_cast<QLineEdit*>( _optFrm->getItembyID(10000) );

        _e->setObjectName( QString::number(fid) );
        _e->setText(name);


        POSCheckBoxEx* _cb = dynamic_cast<POSCheckBoxEx*>( _optFrm->getItembyID(10001) );
        _cb->setChecked( menu["recommendation"].toBool());
        //更新右侧数据
    } else if(value.startsWith("s:")) {
        //已售磬列表
        qDebug() <<"已售磬列表: " << value.mid(2);

        updateRecommendationMenus();
    } else if("submit" == value) {
        //提交
        QLineEdit* _e = dynamic_cast<QLineEdit*>( _optFrm->getItembyID(10000) );
        POSCheckBoxEx* _cb = dynamic_cast<POSCheckBoxEx*>( _optFrm->getItembyID(10001) );

        int fid = _e->objectName().toInt();

        int ret = DataThread::inst().updateMenuRecommendation(fid, _cb->isChecked());
        if( 0 != ret) {
            QMessageBox::warning(this, "出错", "更新菜品售罄状态失败");
        }
    }
}

void RecommendationFrame::updateCategories() {
    //清下数据
    _categories->clean();
    VKeyItemButton* cb = new VKeyItemButton("c:" + QString::number(-1), this, _categories);
    cb->setText(tr("店长推荐类"));
    _categories->addWidget(cb, 1, 1, -1);

    QVariantMap categories = DataThread::inst().loadCategories();

    int cateCount = categories.size();

    QVariantMap::iterator it = categories.begin();

    for(int i = 0; i< cateCount && it != categories.end(); i++, it++ ) {

        int id = it.key().toInt();
        QString name = it.value().toString();

        if( _categories->getItembyID(id) == nullptr) {
            VKeyItemButton* cb = new VKeyItemButton("c:" + QString::number(id), this, _categories);
            cb->setText(name);

            if( i % 5 % 2 == 0) {
                cb->setStyleSheet("background-color: rgb(112, 201, 228)");
            } else {
                cb->setStyleSheet("background-color: rgb(92, 164, 185)");
            }

            _categories->addWidget(cb, 1, 1, id);
        }
    }

    _categories->resetPages();

    if( cateCount > 0) {
        it = categories.begin();
        updateFoodsBycid(it.key().toInt(), it.value().toString());
    }
}
QWidget* RecommendationFrame::menuObjInCategories(int cid, int menuid) {
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
void RecommendationFrame::updateFoodsBycid(int cid, const QString& cname) {

    _menuItems->clean();

    int i = 0;
    QList<QVariant> menus = DataThread::inst().loadMenuItemsByCategoryId(cid);
    foreach (QVariant menu, menus) {
        QVariantMap item = menu.toMap();

        int mid = item["id"].toInt();
        QString img = item["icon"].toString();

        if( _foods.count(mid) == 0) {
            _foods.insert(mid, item);
        }

        MenuItemExFrame* gib = dynamic_cast<MenuItemExFrame*>(menuObjInCategories(cid, mid) );
        if(nullptr != gib) {
            _menuItems->addWidget(gib, 1, 1, mid);
            continue;
        }

        if( _menuItems->getItembyID( mid ) == nullptr ) {
            gib = new MenuItemExFrame( "m:" + QString::number(mid), this, _menuItems);

            gib->setRecommendation( item["recommendation"].toBool());
            gib->setPrice(item["price"].toDouble() );
            gib->setOldPrice(item["oldPrice"].toDouble());

            gib->setMenuItemName( item["name"].toString());
//            gib->setMenuItemIcon(QImage(DataThread::inst().getMenuIconPath() + img));
            gib->setMenuItemIconbyQUrl(QUrl( QString("%1%2").arg(LOCALWEBSERVER).arg(img)));

            if( (++i % 5) % 2 == 0) {
                gib->setBgColor(QColor(230, 235, 226));
            } else {
                gib->setBgColor( QColor(213, 214, 211) );
            }

            gib->setSoldout( item["maxCount"].toInt() == 0);

            _menuItems->addWidget(gib, 1, 1, mid);

        } else {
            gib = dynamic_cast<MenuItemExFrame*>(_menuItems->getItembyID( mid ));
        }

        m_goodsIncategory[cid].push_back( gib) ;
    }

    _menuItems->resetPages();
}

void RecommendationFrame::changeCategory(int cid, const QString& cname) {
    if(-1 == cid) {
        updateRecommendationMenus();
    } else {
        updateFoodsBycid(cid, cname);
    }
}

void RecommendationFrame::updateRecommendationMenus() {
    QList<QVariant> menus = DataThread::inst().loadRecommendationMenus();

    int cid = -1;
    _menuItems->clean();
    m_goodsIncategory.clear();

    int i = 0;
    foreach (QVariant menu, menus) {
        QVariantMap item = menu.toMap();

        int mid = item["id"].toInt();
        QString img = item["icon"].toString();

        if( _foods.count(mid) == 0) {
            _foods.insert(mid, item);
        }

        MenuItemExFrame* gib = dynamic_cast<MenuItemExFrame*>(menuObjInCategories(cid, mid) );
        if(nullptr != gib) {
            _menuItems->addWidget(gib, 1, 1, mid);
            continue;
        }

        if( _menuItems->getItembyID( mid ) == nullptr ) {
            gib = new MenuItemExFrame( "m:" + QString::number(mid), this, _menuItems);

            gib->setRecommendation( item["recommendation"].toBool());
            gib->setPrice(item["price"].toDouble() );
            gib->setOldPrice(item["oldPrice"].toDouble());

            gib->setMenuItemName( item["name"].toString());
//            gib->setMenuItemIcon(QImage(DataThread::inst().getMenuIconPath() + img));
            gib->setMenuItemIconbyQUrl(QUrl( QString("%1%2").arg(LOCALWEBSERVER).arg(img)));

            if( (++i % 5) % 2 == 0) {
                gib->setBgColor(QColor(230, 235, 226));
            } else {
                gib->setBgColor( QColor(213, 214, 211) );
            }

            gib->setSoldout( item["maxCount"].toInt() == 0);

            _menuItems->addWidget(gib, 1, 1, mid);

        } else {
            gib = dynamic_cast<MenuItemExFrame*>(_menuItems->getItembyID( mid ));
        }

        m_goodsIncategory[cid].push_back( gib) ;
    }

    _menuItems->resetPages();
}
