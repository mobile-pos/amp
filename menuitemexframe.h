#ifndef MENUITEMEXFRAME_H
#define MENUITEMEXFRAME_H
/**
  自定义菜品 按钮，显示 菜品图片，价格，和名称
  */
#include <QFrame>
#include <QImage>
#include <QPixmap>
#include <QImageReader>
#include <QNetworkAccessManager>

#include "vkeyboardex.h"

class MenuItemExFrame : public VKeyItemButton
{
    Q_OBJECT

public:
    enum PB_MODE {FIXED_SIZE, FIX_SIZE_CENTRED, AUTO_ZOOM, AUTO_SIZE};

    explicit MenuItemExFrame(const QString& value = "", QWidget* pWnd = nullptr, QWidget* parent = nullptr);
    ~MenuItemExFrame() override;

public:
    void resizeEvent(QResizeEvent*) override;
//    void showEvent(QShowEvent*) override;
    void paintEvent(QPaintEvent *event) override;

    inline void setRecommendation(bool isRecommendation) {
        _recommendation = isRecommendation;
    }
    inline void setPrice(double price) { _price = price;}
    inline void setOldPrice(double oldPrice) { _oldPrice = oldPrice; }
    inline void setMenuItemName(const QString& name) { _name = name;}

    bool setMenuItemIconbyQUrl(const QUrl& image);

    bool setMenuItemIcon(const QString &imgPath, double scale = 1.0) ;
    inline void setBgColor(QColor color) {
        _brush = QBrush(color);
    }

    inline void setSoldout(bool soldout) {
        _isSoldout = soldout;
    }

public slots:
    void replyFinished(QNetworkReply *reply);
private:
    void init();
    /**
      @isdeltext 删除文本线
      */
    void drawText(QPainter & painter, qreal x, qreal y, Qt::Alignment flags,
                  const QString & text, QRectF * boundingRect = {}, bool isdeltext = false);
    void drawText(QPainter & painter, const QPointF & point, Qt::Alignment flags,
                  const QString & text, QRectF * boundingRect = {}, bool isdeltext = false);
private:
    double _oldPrice;
    double _price;
    QString _name;
    QString _imgpath;

    bool _recommendation; //是否为店长推荐

    bool _isSoldout;
    //自绘工具
    QBrush _brush;
    QPixmap _pixmap;
    QString _fileName;

    double _scale;
    PB_MODE _mode;


    QNetworkAccessManager *manager;
    QPixmap *currentPicture;

};

#endif // MENUITEMEXFRAME_H
