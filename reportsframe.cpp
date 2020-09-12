#include "reportsframe.h"

#include "datathread.h"

ReportsFrame::ReportsFrame(QWidget *parent) :
    QFrame(parent)
{
    qDebug() << "ReportsFrame::init";

    init();
    qDebug() << "ReportsFrame::init end";
}

ReportsFrame::~ReportsFrame()
{
}

int ReportsFrame::selDays() {
    QString label = _daycb->currentText();
    if(tr("最近一周") == label) {
        return 7;
    } else if(tr("最近一月") == label ) {
        return 30;
    } else if(tr("最近一季") == label ) {
        return 30 * 3;
    } else if(tr("最近半年") == label ) {
        return 30 * 6 + 3;
    } else if (tr("最近一年") == label ) {
        return 365;
    }

    return 7;
}
void ReportsFrame::init() {
    {
        QLabel * lab = new QLabel(tr("选择统计区间"), this);
        lab->setGeometry(0, 0, 120, 50);

        _daycb = new QComboBox(this);
        _daycb->addItem(tr("最近一周"));
        _daycb->addItem(tr("最近一月"));
        _daycb->addItem(tr("最近一季"));
        _daycb->addItem(tr("最近半年"));
        _daycb->addItem(tr("最近一年"));
        _daycb->setGeometry(125, 0, 100, 50);
    }
    {
        _vkbSalesTrend = new VKeyItemButton("SalesTrend", this, this);
        _vkbSalesTrend->setText(QObject::tr("销售趋势"));
    }  {
        _vkbIncomeRatio = new VKeyItemButton("IncomeRatio", this, this);
        _vkbIncomeRatio->setText(QObject::tr("收入占比"));
    }  {
        _vkbTopFoodSales = new VKeyItemButton("TopFoodSales", this, this);
        _vkbTopFoodSales->setText(QObject::tr("热销菜品"));
    }

    _chartView = new QChartView(this);
}

void ReportsFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->frameGeometry();

    int w = 100;
    int x = 230;
    _vkbSalesTrend->setGeometry(x, 0, w, 50);
    _vkbIncomeRatio->setGeometry(x + 1 * w , 0, w, 50);
    _vkbTopFoodSales->setGeometry(x + 2 * w , 0, w, 50);

    _chartView->setGeometry(0, 60, rect.width(), rect.height() - 65);
}
void ReportsFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    showSalesTrend();
}

void ReportsFrame::showSalesTrend() {

    QVariantMap data = DataThread::inst().loadReportSalesTrend( this->selDays() );
    //!
    QBarSeries *series = new QBarSeries();

    QStringList categories;

    QList<QVariant> days = data["days"].toList();
    QList<QVariant> values = data["data"].toList();

    if(days.length() != values.length() ) {
        return;
    }

    QBarSet *set0 = new QBarSet(tr("销售额"));
    for(int i = 0 ; i< days.length(); i++) {
        QString day = days.at(i).toString();
        QString value = values.at(i).toString();

        categories << day;

        *set0 << value.toDouble();

        series->append(set0);
    }

    //!
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(tr("最近销售趋势"));
        chart->setAnimationOptions(QChart::SeriesAnimations);
    //![3]

    //![4]
        QBarCategoryAxis *axis = new QBarCategoryAxis();
        axis->append(categories);
        chart->createDefaultAxes();//创建默认的左侧的坐标轴（根据 QBarSet 设置的值）
        chart->setAxisX(axis, series);//设置坐标轴
    //![4]

    //![5]
        chart->legend()->setVisible(true); //设置图例为显示状态
        chart->legend()->setAlignment(Qt::AlignBottom);//设置图例的显示位置在底部
    //![5]

    //![6]
        _chartView->setRenderHint(QPainter::Antialiasing);

        if(_chartView->chart() != nullptr){
            delete _chartView->chart();
        }
        _chartView->setChart(chart);
    //![6]
}
void ReportsFrame::showIncomeRatio() {
    //

    QList<QVariant> data = DataThread::inst().loadReportSalesIncome( this->selDays() );

    QPieSeries *series = new QPieSeries();

    foreach(QVariant var, data) {
        QVariantMap dmap = var.toMap();
        QString name = dmap["name"].toString();
        double value = dmap["value"].toDouble();

        series->append(name, value);
    }

    series->setLabelsVisible();


    _chartView->setRenderHint(QPainter::Antialiasing);

    QChart *chart = new QChart();
    chart->setTitle(tr("收入类别占比"));

    chart->removeAllSeries();
    chart->addSeries(series);

    chart->legend()->setAlignment(Qt::AlignBottom);

    chart->setTheme(QChart::ChartThemeBlueCerulean);

    chart->legend()->setFont(QFont("Arial",7));

    if(_chartView->chart() != nullptr){
        delete _chartView->chart();
    }
    _chartView->setChart(chart);
}

void ReportsFrame::showTopFoodSales() {


    QVariantMap data = DataThread::inst().loadReportTopFoodsSaled( this->selDays() );
    //!
    QBarSeries *series = new QBarSeries();

    QStringList categories;

    QList<QVariant> names = data["name"].toList();
    QList<QVariant> values = data["value"].toList();

    if(names.length() != values.length() ) {
        return;
    }

    QBarSet *set0 = new QBarSet("销量");
    for(int i = 0 ; i< names.length(); i++) {
        QString day = names.at(i).toString();
        QString value = values.at(i).toString();

        categories << day;

        *set0 << value.toDouble();

        series->insert(0, set0);
    }

    //!
        QChart *chart = new QChart();
        chart->addSeries(series);
        chart->setTitle(tr("最近热销菜品TOP10"));
        chart->setAnimationOptions(QChart::SeriesAnimations);
    //![3]

    //![4]
        QBarCategoryAxis *axis = new QBarCategoryAxis();
        axis->append(categories);
        chart->createDefaultAxes();//创建默认的左侧的坐标轴（根据 QBarSet 设置的值）
        chart->setAxisX(axis, series);//设置坐标轴
    //![4]

    //![5]
        chart->legend()->setVisible(true); //设置图例为显示状态
        chart->legend()->setAlignment(Qt::AlignBottom);//设置图例的显示位置在底部
    //![5]

    //![6]
        _chartView->setRenderHint(QPainter::Antialiasing);

        if(_chartView->chart() != nullptr){
            delete _chartView->chart();
        }
        _chartView->setChart(chart);
    //![6]
}


void ReportsFrame::onKeyDown(const QString& value) {
    if("SalesTrend" == value) {
        showSalesTrend();

    } else if("IncomeRatio" == value) {
        showIncomeRatio();

    } else if("TopFoodSales" == value) {
        showTopFoodSales();
    }
}
