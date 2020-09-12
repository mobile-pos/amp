#include "qrcodepayframe.h"
#include "ui_qrcodepayframe.h"

#include <QUrl>
#include <QGraphicsPixmapItem>
#include <QNetworkAccessManager>
#include <QMessageBox>

#include "datathread.h"
#include "OrderData.h"



OrderPaiedStatusCheck::OrderPaiedStatusCheck(QString payOrderId, POSDATA::PAYTYPE type, QRCodePayFrame* parent, int oid) :
    QThread(parent),
    _payOrderId(payOrderId),
    _oid(oid),
    _parent(parent),
    _type(type)
{

}

void OrderPaiedStatusCheck::run() {
    while(true) {
        QVariantMap ret = DataThread::inst().getPayStatus4LocalOrder(_oid, _payOrderId);
        if(ret.isEmpty()) {
            msleep(5000);
            continue;
        }

        if(ret["payStatus"].toInt() != 2) {
            msleep(2000);
            continue;
        }

        QMessageBox::information(_parent, tr("通知"), tr("支付完成"));
        this->_parent->onlinePayFinished(ret["oid"].toInt(), (int)POSDATA::WEIPAY);
        this->_parent->hide();
        //支付完成
        break;
    }
}

///////////////////////////////////////////////////
/// \brief QRCodePayFrame::QRCodePayFrame
/// \param parent
///
QRCodePayFrame::QRCodePayFrame(QWidget *parent) :
    QFrame(parent),
    ui(new Ui::QRCodePayFrame),
    currentPicture(nullptr)
{
    ui->setupUi(this);

    manager = new QNetworkAccessManager(this);
    connect(manager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(replyFinished(QNetworkReply*)));
}

QRCodePayFrame::~QRCodePayFrame()
{
    delete ui;
}


void QRCodePayFrame::replyFinished(QNetworkReply *reply) {
    if (reply->error() == QNetworkReply::NoError)
    {
        //获取字节流构造 QPixmap 对象
        currentPicture = new QPixmap();
        currentPicture->loadFromData(reply->readAll());
        QDateTime now;
        QString filename = now.currentDateTime().toString("yyMMddhhmmss.jpg");
        bool ret = currentPicture->save(filename);//保存图片
        if(ret == false) {
            qDebug() << "save qrcode failed";
        }
        ui->qrcodeView->setPixmap(*currentPicture);

        //启动支付状态查询
    }
}
void QRCodePayFrame::updateQRCode(QUrl qrcode, QString payOrderId, POSDATA::PAYTYPE type, QString label) {

    if(nullptr == currentPicture) {
        delete currentPicture;
        currentPicture = nullptr;
    }
    if(type == POSDATA::ALIPAY) {
        ui->sidLabel->setText(tr("支付宝支付：") + label);
    } else {
        ui->sidLabel->setText(tr("微信支付：") + label);
    }
    //获取网络图片
    //http://pay.priusis.com/ppay-api/api/qrcode_img_get?url=weixin://wxpay/bizpayurl?pr=brxRzMK&widht=200&height=200
    manager->get(QNetworkRequest(qrcode));
    //管理好内存
    _curCheck = new OrderPaiedStatusCheck(payOrderId, type, this);
    _curCheck->start();
}

void QRCodePayFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

}
void QRCodePayFrame::hideEvent(QHideEvent *event) {
    QFrame::hideEvent(event);

    _curCheck->terminate();
}
