#ifndef RECEIPTPAINTER_H
#define RECEIPTPAINTER_H

#include <QPaintEvent>
#include <QtPrintSupport/QPrinter>
#include <QtPrintSupport/QPrintDialog>
#include <QPainter>
#include <QWidget>
#include <QList>

#include <QString>
#include <QChar>
#include <QVector>
#include <QException>

#include "OrderData.h"
#include "datathread.h"

class OrderPainter : public QObject
{
private:
    OrderPainter(QPrinter*, const OrderBean&, bool isSplitOrder = false);
    ~OrderPainter();

private:
    void doCustomerPrint();
    QString formatOrder();

    //打印一行
    void drawLineItem(QRect rect);

public:
    static void Print(const OrderBean& receipt,
                      int copies = 1,
                      const QString& localPtName = "",
                      bool isSplitOrder = false);


private:
    //下面的数字单位是 像素
    // 字体大小
    enum {FONT_SIZE_NORMAL = 8,FONT_SIZE_MIDDLE = 10,FONT_SIZE_BIG = 15};  //正常、中等、大

    //预留尺寸
    enum {TOTAL_LENGTH = 140,GOODS_LENGTH = 40,GOODS_NUM_LENGTH = 40}; //小票每一行的可以打印的位数、商品名称的预留长度、商品数量的预留长度

    OrderBean m_order;  //订单的详细信息：时间、编号、总价、收款、找零，以及订单中每样商品的详细信息：名称、数量、单价
    QString shopName;

    bool _isSplitOrder;

    //
    int _titleFontSize;
    int _fontSize;

    QPrinter* _printer;
};

#endif // RECEIPTPAINTER_H
