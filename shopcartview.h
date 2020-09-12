#ifndef SHOPCARTITEM_H
#define SHOPCARTITEM_H

#include <QListWidget>
#include <QLabel>
#include <QNetworkAccessManager>

#include "OrderData.h"
#include "gridlayoutex.h"

class ShopcartView;
class ShopcartItemBuddy : public QWidget
{
    Q_OBJECT
public:
    explicit ShopcartItemBuddy(const FoodBean& fb, QWidget *parent = nullptr);
    void initUi();//初始化Ui
    bool eventFilter(QObject *obj, QEvent *event) override;//事件过滤器

    void updateFoodItem() ;
    void activeBgColor();
    void inActiveBgColor();
signals:

    void onAddItemCount(ShopcartItemBuddy* sel);
    void onSubItemCount(ShopcartItemBuddy* sel);

public slots:
    void resizeEvent(QResizeEvent *event) override;
    void replyFinished(QNetworkReply *reply);


    void onAddItemCountOnSelected();
    void onSubItemCountOnSelected();
public:
    friend ShopcartView;
private:
    QWidget* _icon;  // 菜样图
    QLabel* _foodName;  //菜品
    QLabel* _priceInfo;  //
    QLabel* _flavor;  //
    QLabel* _amount; //当前 food  的总价

    QString _iconPath;
    GridLayoutEx* _layout;

    FoodBean _fb;
    QNetworkAccessManager* _manager;
    QPixmap _pixmap;
};

class ShopcartView : public QListWidget
{
    Q_OBJECT
public:
    explicit ShopcartView(QWidget *parent = nullptr);
    ~ShopcartView();

signals:
    void onFoodItemAddCount();
    void onFoodItemSubCount();

public slots:
    void itemSelectionChanged();

    void onAddItemCount(ShopcartItemBuddy* sel);
    void onSubItemCount(ShopcartItemBuddy* sel);
public:

    void clear();

    void addFoodItem(const FoodBean& fb);
    /**
     * @brief removeFoodItemSelected
     * @return 返回删除的菜品
     */
    QList<FoodBean*> removeFoodItemSelected();

    QList<FoodBean*> getSelectedFood();
    void updateFoodItemSelected();

    QList<FoodBean> allFoods();

    //翻页
    void pageUp();
    void pageDown();
private:
    QListWidgetItem *currentItem;//当前的项
    QMap<QListWidgetItem*,QListWidgetItem*> groupMap;   // 组容器 - key:项 value:组
};

#endif // SHOPCARTITEM_H
