#ifndef REPORTSFRAME_H
#define REPORTSFRAME_H

#include <QFrame>

#include <QtCharts>
#include <QtWidgets>

#include <QtCharts>

using namespace QtCharts;
#include "vkeyboardex.h"

class ReportsFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ReportsFrame(QWidget *parent = nullptr);
    ~ReportsFrame() override;

public:
    void init();
    void refreshWeb();

private:
    void showSalesTrend();
    void showIncomeRatio();
    void showTopFoodSales();
    void showSalesDaily();

    int selDays();
public slots:
    void onKeyDown(const QString&);
public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;
private:

    VKeyItemButton* _vkbSalesDaily;
    VKeyItemButton* _vkbSalesTrend;
    VKeyItemButton* _vkbIncomeRatio;
    VKeyItemButton* _vkbTopFoodSales;

    QComboBox* _daycb;
    QChartView * _chartView;
};

#endif // REPORTSFRAME_H
