#ifndef INPUTGRATUITYFRAME_H
#define INPUTGRATUITYFRAME_H
/**
  信用卡小费录入
*/

#include <QFrame>
#include <QTableView>

#include "gridlayoutex.h"
#include "vkeyboardex.h"
#include "orderpainter.h"

#include "posbuttonex.h"

class GratuityModel;

class GratuityItem
{
public:
    GratuityItem() ;
public:
//    int _creditType; //信用卡类型
    QString _creditName; //信用卡银行名
    QString _tips;
    double _amount; //总费用
};

class GratuityFrame : public QFrame
{
    Q_OBJECT

public:
    explicit GratuityFrame(QWidget *parent = nullptr);
    ~GratuityFrame() override;

private:
    void init();

    void updateSubOrdersGratuity();
public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

    void updateOrder(OrderBean* ob);
public slots:
    void onKeyDown(const QString&);
    void onVKeyDown(const QString&);

    void rowSelected(const QModelIndex &);

signals:
    void onGratuityDone(QList<GratuityItem>& tips);
private:
    GridLayoutEx* _layout;
    VKeyboardEx* _vkb;

    QTableView* _tipsTb;
    GratuityModel* _tipsModel;

    GridLayoutEx* _banks;

    OrderBean* _ob;
};

#endif // INPUTGRATUITYFRAME_H
