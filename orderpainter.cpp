#include "orderpainter.h"
#include <QObject>
#include <QByteArray>
#include <QTextCodec>
#include <QDebug>
#include <QRect>
#include <QPen>
#include <QBrush>
#include <QFont>
#include <QPrinterInfo>
#include <exception>
#include <QJsonDocument>
#include <QUrl>
#include <QTextDocument>

#include "datathread.h"
#include "qrcode/QrCode.hpp"
static QPixmap printQr(const QString& url);
/////////////////////////////////////////////////////////////
/// \brief OrderPainter::OrderPainter
/// \param printer
/// \param od
///
OrderPainter::OrderPainter(QPrinter* printer,
                               const OrderBean& od,
                           bool isSplitOrder)
    : m_order(od)
    ,_isSplitOrder(isSplitOrder)
{
    this->_fontSize = 18;
    this->_titleFontSize = 24;
    this->_printer = printer;
//    doCustomerPrint();
}


QString OrderPainter::formatOrder() {
    QString txt = "<div class='print_container' style='width: 250px;' >";

    QString takeType = "PickUp";
    if(m_order.takeType == 2) {
        takeType = "Deliver";
    }
    QString payTypeDesc = "Cash";
    if(m_order.payType == 10) {
        payTypeDesc = "Credit";
    }

    if(true){
        txt += "        <table border='0' cellspacing='0' style='width: 100%;'>";
        txt += "                <tbody width='100%'>";
        txt += "                        <tr  width='100%'><td  width='100%' style='font-weight: bolder; font-size: " +  QString::number(this->_titleFontSize) + "px;' align='center'>" + gRestaurant["rname"].toString() + "</td></tr>";
        txt += "                        <tr  width='100%'><td width='100%' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='left'>" + gRestaurant["addr"].toString() + "</td></tr>";
        txt += "                        <tr  width='100%'><td width='100%' style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;' align='left'>" + gRestaurant["phone"].toString() + "</td></tr>";
        txt += "                </tbody>";
        txt += "        </table>";
    }

    txt += "        <table border='0' cellspacing='0' style='width: 100%;'>";
    txt += "                <tbody width='100%'>";
    //#显示 order id
    txt += "                        <tr >";
    txt += "                                     <td  width='6%' style='border-bottom: 1px dashed rgb(0, 0, 0); font-size: " + QString::number(_fontSize) + "px;' align='left'>" + "Order #: " + "</td>";
    txt += "                                     <td  width='94%' style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_titleFontSize - 4) + "px;' align='left'>"  +  m_order.orderid + "</td>";
    txt += "                        </tr>";
    txt += "                        <tr  > <td colspan='2' width='100%' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_titleFontSize - 2) + "px; ' align='center'>" + m_order.takeType + "</td></tr>";
    txt += "                        <tr  >";
    txt += "                                    <td  width='6%' style='border-top: 1px dashed rgb(0, 0, 0); font-size: " + QString::number(_fontSize) + "px; ' align='left'>" + "PayType: " + "</td>";
    txt += "                                    <td  width='94%' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='left'>" + payTypeDesc + "</td>";
    txt += "                        </tr>";
    if (m_order.payType == 10) {
        txt += "                        <tr  > ";
        txt += "                                <td  width='6%' style='border-top: 1px dashed rgb(0, 0, 0); font-size: " + QString::number(_fontSize) + "px; ' align='left'>" + "Card's ID: "  + "</td>";
        txt += "                                <td  width='94%' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='left'>" + m_order.creditID + "</td>";
        txt += "                        </tr>";
        txt += "                        <tr  > ";
        txt += "                                <td  width='6%' style='border-bottom: 1px dashed rgb(0, 0, 0); font-size: " + QString::number(_fontSize) + "px; ' align='left'>" + "Expiration: " + "</td>";
        txt += "                                <td  width='94%' style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='left'>" + m_order.creditValidity + "</td>";
        txt += "                        </tr>";
    }
    txt += "                </tbody>";
    txt += "        </table>";

    txt += "        <table border='0' cellspacing='0' style='width: 100%;'>";
    txt += "                <tbody width='100%'>";
    txt += "                       <tr >";
    txt += "                                     <td  width='6%' style='border-top: 1px dashed rgb(0, 0, 0); font-size: " + QString::number(_fontSize) + "px;' align='left'>" + (m_order.sid.startsWith("TC-") ? "Table: ": "Phone: ")  + "</td>";
    txt += "                                     <td  width='94%' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;' align='left'>" + m_order.sid.right(3) + "</td>";
    txt += "                        </tr>";

    if ("Deliver" == takeType) {
        txt += "                       <tr >";
        txt += "                                <td width='6%' style='font-size: " + QString::number(_fontSize) + "px; ' align='left'>"  + "Address: " + "</td>";
        txt += "                                <td width='94%' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='left'>"  + m_order.addr + "</td>";
        txt += "                       </tr>";
    }

    txt += "                        <tr >";
    txt += "                                     <td  width='6%' style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;' align='left'>" + "Time: " + "</td>";
    txt += "                                     <td  width='94%' style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;' align='left'>" +  m_order.time + "</td>";
    txt += "                        </tr>";
    txt += "                </tbody>";
    txt += "        </table>";

    txt += "        <table border='0' cellspacing='0' style='width: 100%;'>";
    //# txt += "                <thead  width='100%'><tr  height='0px' >"
    //# txt += "                        <td width='6%' align='left' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;'>" + "Count" + "</td>"
    //# txt += "                        <td width='74%' align='left' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;'>" + "Food Name" + "</td>"
    //# txt += "                        <td width='20%' align='right' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;'>" + "Price" + "</td>"
    //# txt += "                </tr></thead>"
    txt += "                <tbody width='100%'>";

    foreach(FoodBean food, m_order.foods) {
        txt += "                        <tr >";
        txt += "                                <td width='3%' align='left' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;'>" + QString::number(food.count) + "</td>";
        txt += "                                <td width='77%' align='left' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;'>" + food.cnName + "</td>";
        txt += "                                <td width='20%' align='right' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px;'>" + QString().sprintf("%.2f", food.unit_price * food.count) + "</td>";
        txt += "                        </tr>";
        txt += "                        <tr><td/><td align='left' colspan='2' style='font-size: " + QString::number(_fontSize - 2) + "px; '>(" + food.enName + ")</td></tr>";
        if (food.flavor != "")
            txt += "                    <tr><td style='border-bottom: 1px dashed rgb(0, 0, 0)' /><td align='left' colspan='2' style='border-bottom: 1px dashed rgb(0, 0, 0);  font-weight: bolder; font-size: " + QString::number(_fontSize - 2)  + "px; '>" + "[mark: " + food.flavor + "]</td></tr>";
    }
    txt += "                </tbody>";
    txt += "                <tfoot>";
    txt += "                        <tr >";
    txt += "                                    <td colspan='2' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + "Total: " + "</td>";
    txt += "                                    <td colspan='1' style='border-top: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + QString().sprintf("%-5.3f", m_order.total) + "</td>";
    txt += "                        </tr>";
    txt += "                        <tr >";
    txt += "                                    <td colspan='2' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + "Tax: " + "</td>";
    txt += "                                    <td colspan='1' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + QString().sprintf("%-5.3f", m_order.total * m_order.taxRate) + "</td>";
    txt += "                        </tr>";
    //# txt += "                        <tr >"
    //# txt += "                                    <td colspan='2' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + "Tips: " + "</td>"
    //# txt += "                                    <td colspan='1' style='font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + "{:_>10s}".format('') + "</td>"
    //# txt += "                        </tr>"
    txt += "                        <tr >";
    txt += "                                    <td colspan='2' style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + "Total Amount: " + "</td>";
    txt += "                                    <td colspan='1' style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; ' align='right'>" + QString().sprintf("%-5.3f", m_order.total * (1+ m_order.taxRate)) + "</td>";
    txt += "                        </tr>";
    txt += "                </tfoot>";
    txt += "        </table>";
    txt += "        <table border='0' cellspacing='0' style='width: 100%;'>";
    txt += "            <tbody width='100%'>";
    if (m_order.mark != "")
        txt += "                    <tr width='100%'><td width='100%' align='left'   style='border-bottom: 1px dashed rgb(0, 0, 0); font-weight: bolder; font-size: " + QString::number(_fontSize) + "px; '>&nbsp;&nbsp;Order Mark:  " + m_order.mark + "</td></tr>";

    if(true) {
        txt += "                    <tr width='100%'><td width='50%' align='center' ><img src='qrcode-gzh.png' style='width: 100%;'><p/><div style='text-align:center;font-size: 10px;'>扫码关注公众号</div> </td>";
        txt += "                                     <td width='50%' align='center' ><img src='qrcode-shop-" + QString::number(gRestaurant["rid"].toInt()) + ".png' style='width: 100%;'><p/><div style='text-align:center;font-size: 10px;'>Scan To Order</div> </td>";
        txt += "                    </tr>";
    }

    txt += "                    <tr><td height='30px'/></tr>";

    txt += "                </tbody>";
    txt += "        </table>";


    txt += "</div>";

    qDebug() << " order: \n" << txt;
    return txt;
}

void OrderPainter::doCustomerPrint(){

    QTextDocument doc; //  # 使用QTextDcument对html进行解析
    //#d.setDocumentMargin(0)  # 将打印的边距设为0
    doc.setPageSize(QSizeF(this->_printer->pageRect().size()));


    QString content = formatOrder();
    doc.setHtml(content);
    doc.print(this->_printer);

    qDebug() << "print item OK";
}

OrderPainter::~OrderPainter()
{

}


void OrderPainter::Print(const OrderBean& order, int copies, const QString& localPtName, bool isSplitOrder) {
    QString pname = localPtName;
    if( "" == localPtName) {
        pname = DataThread::inst().getCashierPrinter();
    }
    if("" == pname) {
        pname = QPrinterInfo::defaultPrinterName();
    }
    qDebug() << "printer name: " << pname;

    QPrinterInfo targetPrinter = QPrinterInfo::printerInfo(pname);
    QPrinter printer(targetPrinter);

    printer.setOutputFormat(QPrinter::PdfFormat);
    printer.setOutputFileName("~/a.pdf");


    printer.setNumCopies(copies);

    OrderPainter rp(&printer, order, isSplitOrder);
    rp.doCustomerPrint();
}


// Prints the given QrCode object to the console.
static QPixmap printQr(const QString& url) {
    int border = 4;


    std::vector<qrcodegen::QrSegment> segs0 = qrcodegen::QrSegment::makeSegments(url.toStdString().c_str());
    qrcodegen::QrCode qr = qrcodegen::QrCode::encodeSegments(segs0, qrcodegen::QrCode::Ecc::HIGH,
                                qrcodegen::QrCode::MIN_VERSION,
                                qrcodegen::QrCode::MAX_VERSION, -1, true);  // Automatic mask

    QPixmap map(qr.getSize(), qr.getSize());
    map.fill();

    QPainter painter(&map); //创建一直画笔
    painter.setPen({0, 0, 0});

    for (int y = -border; y < qr.getSize() + border; y++) {
        for (int x = -border; x < qr.getSize() + border; x++) {

            if(true == qr.getModule(x, y)) {
                painter.drawPoint(x, y);
            }

        }
    }

    return map.scaled(200, 200);
}
