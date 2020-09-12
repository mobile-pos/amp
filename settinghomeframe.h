#ifndef SETTINGHOMEFRAME_H
#define SETTINGHOMEFRAME_H

/**
  pos系统设置相关的主页面
  */
#include <QFrame>

#include "gridlayoutex.h"
#include "estimateclearlyframe.h"
#include "reportsframe.h"
#include "ordersmanagerframe.h"
#include "cashboxsetframe.h"
#include "reducemenupriceframe.h"
#include "recommendationframe.h"
#include "printersframe.h"
#include "sysoptframe.h"

class SetPanel : public QFrame
{
public:
    SetPanel(QWidget* parent = nullptr);
public:
    void addFrame(QFrame* nfrm);

    QFrame* showFrame(const QString& objName);

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
private:
    QMap<QString, QFrame*> _frames;
};

class SettingHomeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SettingHomeFrame(QWidget *parent = nullptr);
    ~SettingHomeFrame() override;

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

    void retranslateUI();
public slots:
    void onKeyDown(const QString&);

private:
    void init();
private:
    GridLayoutEx* _layout;

    GridLayoutEx* _nav;

    //右侧面板
    SetPanel* _panel;
    //估清设置
    EstimateClearlyFrame* _estiFrm;
    //打印机设置
    PrintersFrame* _printFrm;
    //报表页面
    ReportsFrame* _reportFrm;
    //订单管理页面
    OrdersManagerFrame* _ordersFrm;
    //钱箱设置
    CashBoxSetFrame* _cashFrm;
    //特价菜设置
    ReduceMenuPriceFrame* _reduceFrm;
    //店长推荐
    RecommendationFrame* _recomFrm;
    //系统设置
    SysOptFrame* _sysFrm;
};

#endif // SETTINGHOMEFRAME_H
