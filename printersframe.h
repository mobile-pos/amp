#ifndef PRINTERSFRAME_H
#define PRINTERSFRAME_H
/**
  打印机管理
  */
#include <QFrame>
#include <QMap>
#include <QComboBox>

//#include <QPrinterInfo>

#include "gridlayoutex.h"
//#include "redis-cli/redisclient.h"
//using namespace Redis;

class PrintersFrame : public QFrame
{
    Q_OBJECT

public:
    explicit PrintersFrame(QWidget *parent = nullptr);
    ~PrintersFrame() override;

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

public slots:
    void onSubmitClick();
    void onCancelClick();

    void printControl();
    void modifyPrinter();
private:
    void init();

private:

    //当前系统所有的打印机
    //分配好的前台打印机和厨房打印机
    QList<QComboBox*> _cookingPrinters;
    QComboBox* _cashierPrinter;

    GridLayoutEx* _layout;
};

#endif // PRINTERSFRAME_H
