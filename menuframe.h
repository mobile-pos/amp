#ifndef MENUFRAME_H
#define MENUFRAME_H

#include <QFrame>
#include <QToolButton>
#include <QTableView>

#include "orderpainter.h"
#include "flavorsettingframe.h"
#include "moresettingframe.h"
#include "orderpayframe.h"
#include "consumertypeframe.h"
#include "shopcartview.h"
#include "posbuttonex.h"

#include "redis-cli/redisclient.h"
#include "consumerdisplayframe.h"

using namespace Redis;


class FoodsModel;
class MenuFrame;

//菜品类别 按钮
class CategoryButton : public POSToolButtonEx
{
    Q_OBJECT
public:
    CategoryButton(int cid, const QString& cname, MenuFrame* pwnd, QWidget* parent = nullptr);
public:
    inline int getCategoryId() { return m_cid;}

public slots:
    void onClicked();
signals:
    void changeCategory(int cid, const QString& cname);
private:
    int m_cid;
    QString m_cname;
    MenuFrame* m_pWnd;
};


class MenuFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MenuFrame(QWidget *parent = nullptr);
    ~MenuFrame() override;

public:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *e) override;
    void hideEvent(QHideEvent *e) override;

    inline void setConsumerDisplay(ConsumerDisplayFrame* cd) {
        _cdFrm = cd;
    }
public slots:
    void hide();
public:
    //加载订单的菜品详情
    void updateReceipts();
    //加载菜谱的类别信息
    void updateCategories();
    //根据类别id, 加载菜谱
    void updateFoodsBycid(int cid, const QString& cname);

    //店长推荐类
    void updateRecommendationMenus();
    //物价类
    void updateReduceMenus();

    void init();

    void showFrme(QFrame* frm);

    void newOrder();
    void updateTableStatus(bool satdown);

    QWidget* menuObjInCategories(int cid, int menuid);

    void addMenuItemtoMenuPanel(QVariantMap item);

    void retranslateUI(); //更新资源
public slots:
    void on_m_gotoHomeFrame_clicked();
    void rowSelected(const QModelIndex &);

    void changeCategory(int cid, const QString& cname);

    void addFoodItem(const FoodBean& goods);

    //订单的菜品发生更新
    void GoodsChanged();

    //跟随项设置完成
    void flavorSettingDone(const QString& flavor );
    void flavorSettingCancel();
    //变更类型
    void consumerTypeChangedDone(const QString& type, const QString& name, const QString& addr);

    //增加菜品的事件
    void onKeyDown(const QString&);
public:
    void setOrderBean(const OrderBean& ob);
private slots:
    void on_m_printOrder_clicked();

    void on_m_deleteItemInOrder_clicked();

    void on_m_ItemCountAdd_clicked();

    void on_m_itemCountSub_clicked();

    void on_m_flavor4Food_clicked();

    void on_moreSetting_clicked();

    void on_m_orderPay_clicked();

    void on_m_toCooking_clicked();

    void on_consumertype_change();

    void on_discount4Food_clicked();

    void onShopcartPageUpClicked();
    void onShopcartPageDownClicked();
public slots:
    void connected();
    void disconnected();
    void onReply(const QString& cmd, Reply value);

    void onMoreDone();
    void onFinishPay();
private:
    OrderBean m_ob;

    //小计
    QLineEdit* m_subamountTE;
    QLineEdit* m_taxTE;
    QLineEdit* m_gratuityTE;
    QLineEdit* m_amountTE;
    QLineEdit* m_discountTE;
    QLineEdit* m_totalCount;

    //订单基本信息，如果是电话订单，且是外卖，则需要输入地址
    QLineEdit* _name;
    QTextEdit* _addr;

    //购物车

    ShopcartView* _shopcart;

    GridLayoutEx* _leftBottomPane; //左下则操作面板， 包括：订单基本信息，菜口操作，和订单操作
    //右则操作面板
    GridLayoutEx* _catePane;
    GridLayoutEx* _menuPane;
    //显示菜类编码
    GridLayoutEx* _cateCodePane;
    //

    //跟随项设置
    FlavorSettingFrame* _flavorFrm;
    //更多设置
    MoreSettingFrame* _moreFrm;
    //付款窗口
    OrderPayFrame* _payFrm;
    //类型变更
    ConsumerTypeFrame* _consumerFrm;
    //单项折扣
    DiscountFrame* _disFrm;

    //同步客显
    ConsumerDisplayFrame* _cdFrm;

    //分类按钮列表
//    QMap<int, CategoryButton*> m_catebtn; // 所有类别，按  category id
//    QMap<int, GoodsItem> m_goods; //所有菜品，按 food id
    QMap<int, QList<QWidget*> > m_goodsIncategory; //各分类中的菜品，按 category id
    QMap<int, QVariant> _foods; // 菜品 信息，int = fid;

    //缓存
    RedisClient* _rediscli;

    //搜索菜品
    LineEditEx* _searchCnt;
    VKeyItemButton* _searchBtn;
};

#endif // MENUFRAME_H
