#ifndef COOKINGTIMEFRAME_H
#define COOKINGTIMEFRAME_H

#include <QFrame>
#include <QLineEdit>
#include <QLabel>

#include "gridlayoutex.h"
#include "posbuttonex.h"


class HourItemButton : public POSToolButtonEx
{
    Q_OBJECT
public:
    HourItemButton(const QString& label, QWidget* pWnd, QWidget* parent = nullptr);
    ~HourItemButton() override;
public slots:
    void onClicked();
signals:
    void hourKeyReder(const QString&);


};

class MinuteItemButton : public POSToolButtonEx
{
    Q_OBJECT
public:
    MinuteItemButton(const QString& label, QWidget* pWnd, QWidget* parent = nullptr);
    ~MinuteItemButton() override;
public slots:
    void onClicked();
signals:
    void minuteKeyReder(const QString&);


};



class CookingTimeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit CookingTimeFrame(QWidget *parent = nullptr);
    ~CookingTimeFrame() override;

public:
    void resizeEvent(QResizeEvent *) override;
    void showEvent(QShowEvent *e) override;
public slots:
    void hourKeyReder(const QString&);
    void minuteKeyReder(const QString&);
    void onAmpmSwitch();

    void onDone();
    void onCancel();

signals:
    void onCookingTimeSet(const QString&);
private:
    void init();

private:
    QLineEdit* _hour;
    QLineEdit* _minute;
    QLabel* _split;

    POSToolButtonEx* _apm;
    POSToolButtonEx* _doneBtn;
    POSToolButtonEx* _cancelBtn;

    GridLayoutEx* _hoursLayout;
    GridLayoutEx* _minutesLayout;
};

#endif // COOKINGTIMEFRAME_H
