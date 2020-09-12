#include "menuitemexframe.h"

#include <QPainter>
#include <QNetworkReply>
#include <QImageWriter>
#include <QDir>

#include "datathread.h"

static const int IMAGE_WIDTH = 160;
static const int IMAGE_HEIGHT = 120;
static const QSize IMAGE_SIZE = QSize(IMAGE_WIDTH, IMAGE_HEIGHT);

MenuItemExFrame::MenuItemExFrame(const QString& value , QWidget* pWnd, QWidget* parent) :
    VKeyItemButton(value, pWnd, parent),
    _oldPrice(0.0)
{
    _brush = QBrush(Qt::white);
    _pixmap = QPixmap(IMAGE_SIZE);
    _scale = 1.0;
    _mode = AUTO_ZOOM;
    _isSoldout = false;

    manager = new QNetworkAccessManager(this);

    connect(manager, SIGNAL(finished(QNetworkReply*)),
        this, SLOT(replyFinished(QNetworkReply*)));
}

MenuItemExFrame::~MenuItemExFrame()
{
}

void MenuItemExFrame::init() {
}


void MenuItemExFrame::replyFinished(QNetworkReply *reply) {

    QVariant redirect = reply->attribute(QNetworkRequest::RedirectionTargetAttribute);
    if( ! redirect.isNull()) {
        qDebug() << "redirect: " << redirect.toUrl();

        manager->get( QNetworkRequest(redirect.toUrl()));
        return;
    }

    if (reply->error() == QNetworkReply::NoError)
    {
        //获取字节流构造 QPixmap 对象

        _pixmap.loadFromData(reply->readAll() );
        _scale = qBound(0.01, 1.0, 100.0);
        if(_mode == AUTO_SIZE)
        {
            setFixedSize(_pixmap.size() * _scale);
        }

        update();
    } else {
        qDebug() << "network reply failed: " << reply->errorString();
    }
}

void MenuItemExFrame::resizeEvent(QResizeEvent* e) {
    VKeyItemButton::resizeEvent(e);
}

bool MenuItemExFrame::setMenuItemIconbyQUrl(const QUrl& image) {
    qDebug() << "menu icon: " << image.url();

    QStringList items = image.url().split("/");
    _fileName = items[ items.length() - 1];

    manager->get(QNetworkRequest(image));

    return true;
}

bool MenuItemExFrame::setMenuItemIcon(const QString &imagepath, double scale) {

    QImageReader reader( QDir::toNativeSeparators(imagepath) );
    reader.setFormat("jpeg");

    foreach(QByteArray ba, reader.supportedImageFormats()) {
        qDebug() << "supported image format: " << ba << "\n";
    }

    reader.setDecideFormatFromContent(true);
    if(!reader.canRead()) {
        qDebug() << " iamge path: " << QDir::toNativeSeparators(imagepath) <<  ", can not read: " << reader.errorString();
        return false;
    }

    _pixmap = QPixmap::fromImage(reader.read());
    _scale = qBound(0.01, scale, 100.0);
    if(_mode == AUTO_SIZE)
    {
        setFixedSize(_pixmap.size() * _scale);
    }
    update();
    return true;
}

void MenuItemExFrame::drawText(QPainter & painter, qreal x, qreal y, Qt::Alignment flags,
              const QString & text, QRectF * boundingRect, bool isdeltext)
{
   const qreal size = 32767.0;
   QPointF corner(x, y - size);
   if (flags & Qt::AlignHCenter) corner.rx() -= size/2.0;
   else if (flags & Qt::AlignRight) corner.rx() -= size;
   if (flags & Qt::AlignVCenter) corner.ry() += size/2.0;
   else if (flags & Qt::AlignTop) corner.ry() += size;
   else flags |= Qt::AlignBottom;
   QRectF rect{corner.x(), corner.y(), size, size};
   painter.drawText(rect, flags, text, boundingRect);
   if( true == isdeltext) {
       painter.drawLine(rect.left(), rect.top() + rect.height() / 2,
                        rect.right(), rect.top() + rect.height() /2);
   }
}

void MenuItemExFrame::drawText(QPainter & painter, const QPointF & point, Qt::Alignment flags,
              const QString & text, QRectF * boundingRect, bool isdeltext)
{
   drawText(painter, point.x(), point.y(), flags, text, boundingRect, isdeltext);
}


void MenuItemExFrame::paintEvent(QPaintEvent *event) {
/**
+-------------------+
|           [ price]|
| [     pic     ]   |
| [name]            |
+-------------------+
*/
    Q_UNUSED(event);
    QPainter painter(this);

    painter.setBackground(_brush);
    painter.eraseRect(rect());

    double window_width, window_height;
    double image_width, image_height;
    double r1, r2, r;
    int offset_x, offset_y;


    switch (_mode)
    {
    case FIXED_SIZE:
    case AUTO_SIZE:
     painter.scale(_scale, _scale);
     painter.drawPixmap(0, 0, _pixmap);
     break;
    case FIX_SIZE_CENTRED:
     window_width = width();
     window_height = height();
     image_width = _pixmap.width();
     image_height = _pixmap.height();
     offset_x = (window_width - _scale * image_width) / 2;
     offset_y = (window_height - _scale * image_height) / 2;
     painter.translate(offset_x, offset_y);
     painter.scale(_scale, _scale);
     painter.drawPixmap(0, 0, _pixmap);
     break;
    case AUTO_ZOOM:
     window_width = width();
     window_height = height();
     image_width = _pixmap.width();
     image_height = _pixmap.height();
     r1 = window_width / image_width;
     r2 = window_height / image_height;
     r = qMin(r1, r2);
     offset_x = (window_width - r * image_width) / 2;
     offset_y = (window_height - r * image_height) / 2;
     painter.translate(offset_x, offset_y);
     painter.scale(r, r);
     painter.drawPixmap(0, 0, _pixmap);
     break;
    }

    if(false == _isSoldout){
        //写价格
        QPainter painter2(this);
        painter2.setBackground(QBrush(Qt::white));
        painter2.eraseRect(QRect(width() - 40, 0, 40, 16));
        painter2.setPen({255, 0, 0, 255});
        painter2.setFont(QFont("Arial", 12, QFont::Bold));
        drawText(painter2, width(), 16, Qt::AlignRight, QString("$%1").arg(_price));

        if( _oldPrice > 0.0) {
            painter2.setBackground(QBrush(Qt::white));
            painter2.eraseRect(QRect(width() - 40, 16, 40, 16));
            painter2.setFont(QFont("Arial", 10, QFont::Bold));
            painter2.setPen(Qt::gray);
            drawText(painter2, width(), 32, Qt::AlignRight, QString("$%1").arg(_oldPrice), {}, true);
        }

        if( _recommendation ) {
            painter2.setBackground(QBrush(Qt::white));
            painter2.eraseRect(QRect(width() - 40, 32, 40, 16));
            painter2.setFont(QFont("Arial", 10, QFont::Bold));
            painter2.setPen({255, 64, 0});
            drawText(painter2, width(), 48, Qt::AlignRight, tr("店长推荐"), {}, true);
        }
    } else {
        QPainter painter2(this);
        painter2.setBackground(QBrush(Qt::white));
        painter2.eraseRect(QRect(width() - 60, 0, 60, 18));
        painter2.setFont(QFont("Arial", 12, QFont::Bold));
        painter2.setPen({220, 0, 0, 255});
        drawText(painter2, width() - 60, 18, Qt::AlignLeft, QObject::tr("已售完"));
    }

    {
        //写 菜名
        QPainter painter2(this);
        painter2.setBackground(QBrush(Qt::white));
        painter2.eraseRect(QRect(0, height() - 20, width(), 20));
        painter2.setFont(QFont("Arial", 12, QFont::Bold));
        painter2.setPen({0, 0, 0, 255});
        drawText(painter2, 0, height(), Qt::AlignLeft, _name);
    }
}
