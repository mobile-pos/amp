#ifndef ESTIMATECLEARLYFRAME_H
#define ESTIMATECLEARLYFRAME_H
/**
  商品估清设置
  */
#include <QFrame>
#include "gridlayoutex.h"
#include "vkeyboardex.h"

class EstimateClearlyFrame : public QFrame
{
    Q_OBJECT

public:
    explicit EstimateClearlyFrame(QWidget *parent = nullptr);
    ~EstimateClearlyFrame() override;

public:
    void init();
    void updateCategories();

    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
public slots:
    void onKeyDown(const QString&);
    void onVKeyDown(const QString&);
    void focussed(QWidget* _this, bool hasFocus);

private:
    void changeCategory(int cid, const QString& cname);
    void updateFoodsBycid(int cid, const QString& cname);

    void updateSoldoutMenus();

    QWidget* menuObjInCategories(int cid, int menuid);
private:

    GridLayoutEx* _categories;
    GridLayoutEx* _menuItems;

    GridLayoutEx* _optFrm;
    VKeyboardEx* _vkb;

    //////
    //分类按钮列表
    QMap<int, QList<QWidget*> > m_goodsIncategory; //各分类中的菜品，按 category id
    QMap<int, QVariant> _foods; // 菜品 信息，int = fid;
};

#endif // ESTIMATECLEARLYFRAME_H
