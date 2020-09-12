#ifndef RECOMMENDATIONFRAME_H
#define RECOMMENDATIONFRAME_H
/**
  店长推荐 设置
  */

#include <QFrame>
#include "gridlayoutex.h"

class RecommendationFrame : public QFrame
{
    Q_OBJECT

public:
    explicit RecommendationFrame(QWidget *parent = nullptr);
    ~RecommendationFrame();

public:
    void init();
    void updateCategories();

    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

private:
    void changeCategory(int cid, const QString& cname);
    void updateFoodsBycid(int cid, const QString& cname);

    //更新特价区
    void updateRecommendationMenus();
    QWidget* menuObjInCategories(int cid, int menuid);

private slots:
    void onKeyDown(const QString& value);
private:

    GridLayoutEx* _categories;
    GridLayoutEx* _menuItems;

    GridLayoutEx* _optFrm;


    //////
    //分类按钮列表
    QMap<int, QList<QWidget*> > m_goodsIncategory; //各分类中的菜品，按 category id
    QMap<int, QVariant> _foods; // 菜品 信息，int = fid;
};

#endif // RECOMMENDATIONFRAME_H
