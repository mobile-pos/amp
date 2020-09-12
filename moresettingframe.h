#ifndef MORESETTINGFRAME_H
#define MORESETTINGFRAME_H

#include <QFrame>
#include <QComboBox>

#include "OrderData.h"
#include "gridlayoutex.h"
#include "cookingtimeframe.h"
#include "vkeyboardex.h"
#include "flavorsettingframe.h"

class MoreSettingFrame;

/**
 拆单 输入窗口
 */
class SplitOrderFrame : public QFrame {
    Q_OBJECT
public:
    explicit SplitOrderFrame(QWidget* target, QWidget* parent) ;
    ~SplitOrderFrame() override;
public:
    void resizeEvent(QResizeEvent* e) override;
    void showEvent(QShowEvent* e) override;
public slots:
    void onKeyDown(const QString& k);
    void onVKeyDown(const QString& k);
signals:
    void done(int count);
private:
    void init();
private:
    GridLayoutEx* _layout;
    VKeyboardEx* _vkb;
    LineEditEx* _number;
    QWidget* _target;
};

/**********
 * 折扣下拉扩展
 * */

class DiscountFrame : public QFrame {
    Q_OBJECT
public:
    explicit DiscountFrame(QWidget* target, QWidget* parent) ;
    ~DiscountFrame() override;
public:
    void resizeEvent(QResizeEvent* e) override;

    void retranslateUI();
private:
    void init();
private:
    GridLayoutEx* _layout;
    QWidget* _target;
};
class DiscountComboBox : public QComboBox
{
    Q_OBJECT

public:
    explicit DiscountComboBox(MoreSettingFrame* pWnd, QWidget* parent) ;
    ~DiscountComboBox() override ;
public:
    void showPopup() override;
signals:
    void showDiscountFrame();
private:
    MoreSettingFrame* _pWnd;
};


class MoreSettingFrame : public QFrame
{
    Q_OBJECT

public:
    explicit MoreSettingFrame(OrderBean* order, QWidget *parent = nullptr);
    ~MoreSettingFrame() override ;

public:
    void resizeEvent(QResizeEvent *) override;

    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent* e) override;

    void updateOrder(OrderBean* ob);

    void retranslateUI();
public slots:
    void onCookTimeClicked();
    void onKeyDown(const QString&);
    void showDiscountFrame();
    void onCookingTimeSet(const QString&);

    void onSaveConsumerAddrClicked();
    //分单完成
    void onDoneSplit(int);

    void flavorSettingDone(const QString& flavor );
signals:
    void onMoreDone();

private:
    void init();
private:
    OrderBean* _order;
    VKeyItemButton* _done;
    VKeyItemButton* _cancel;

    CookingTimeFrame* _cookingTimeFrm; //预约下厨时间
    DiscountFrame* _discountFrm;   //折扣
    DiscountComboBox *_discount;

    FlavorSettingFrame* _flavorFrm;


    SplitOrderFrame* _splitOrderFrm; //分单

    GridLayoutEx* _layout;
};

#endif // MORESETTINGFRAME_H
