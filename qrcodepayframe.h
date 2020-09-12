#ifndef QRCODEPAYFRAME_H
#define QRCODEPAYFRAME_H

#include <QFrame>
#include <QNetworkReply>
#include <QThread>

#include "OrderData.h"

namespace Ui {
class QRCodePayFrame;
}

class QRCodePayFrame;
class OrderPaiedStatusCheck : public QThread
{
public:
    OrderPaiedStatusCheck(QString payOrderId, POSDATA::PAYTYPE type, QRCodePayFrame* parent, int oid = 0);
public:
    void run() override;

private:
    QString _payOrderId;
    int _oid;

    QRCodePayFrame* _parent;
    POSDATA::PAYTYPE _type;
};

class QRCodePayFrame : public QFrame
{
    Q_OBJECT

public:
    explicit QRCodePayFrame(QWidget *parent = nullptr);
    ~QRCodePayFrame();

public:
    void updateQRCode(QUrl qrcode, QString payOrderId, POSDATA::PAYTYPE type, QString label);
    void showEvent(QShowEvent* e) override;
    void hideEvent(QHideEvent *event) override;

public slots:
    void replyFinished(QNetworkReply *reply);

signals:
    void onlinePayFinished(int oid, int type);

private:
    Ui::QRCodePayFrame *ui;

    QNetworkAccessManager *manager;
    QPixmap *currentPicture;

    OrderPaiedStatusCheck* _curCheck;
};

#endif // QRCODEPAYFRAME_H
