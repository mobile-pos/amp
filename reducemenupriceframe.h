#ifndef REDUCEMENUPRICEFRAME_H
#define REDUCEMENUPRICEFRAME_H

/**
  降价调整
*/

#include <QFrame>

#include "gridlayoutex.h"
#include "vkeyboardex.h"
#include "posbuttonex.h"

class ReduceMenuPriceFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ReduceMenuPriceFrame(QWidget *parent = nullptr);
    ~ReduceMenuPriceFrame();

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

    //更新特价区
    void updateReduceMenus();
    QWidget* menuObjInCategories(int cid, int menuid);
private:

    GridLayoutEx* _categories;
    GridLayoutEx* _menuItems;

    GridLayoutEx* _optFrm;
    VKeyboardEx* _vkb;

    LineEditEx* _oldPriceEdit;
    LineEditEx* _newPriceEdit;

    //////
    //分类按钮列表
    QMap<int, QList<QWidget*> > m_goodsIncategory; //各分类中的菜品，按 category id
    QMap<int, QVariant> _foods; // 菜品 信息，int = fid;

};

#endif // REDUCEMENUPRICEFRAME_H
