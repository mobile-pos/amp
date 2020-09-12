#include "consumerdisplayframe.h"

#include <QDesktopWidget>
#include <QScreen>
#include <QApplication>
#include <QLineEdit>
#include <QMessageBox>

ConsumerDisplayFrame::ConsumerDisplayFrame(QWidget *parent) :
    QFrame(parent),
    _ob(nullptr),
    _shopcart(nullptr)
{
    _layout = new GridLayoutEx(16, this);
    init();
}

ConsumerDisplayFrame::~ConsumerDisplayFrame()
{
}

void ConsumerDisplayFrame::init() {
    _layout->clean();

    {
        _shopcart = new ShopcartView(_layout);
        _shopcart->setObjectName(QString::fromUtf8("购物车"));
        _shopcart->setStyleSheet("background-color: rgb(160, 160, 154)");

        _layout->addWidget(_shopcart, 16, 9);
    }
    //显示价格
    {
        //10列
        {
            QLabel* lab = new QLabel(tr("小计"), _layout);
            lab->setAlignment(Qt::AlignRight);
            _layout->addWidget(lab, 2);

            QLineEdit* subtotal = new QLineEdit(_layout);
            subtotal->setObjectName("SubTotal");
            _layout->addWidget(subtotal, 2);
        }
        {
            QLabel* lab = new QLabel(tr("税率"), _layout);
            lab->setAlignment(Qt::AlignRight);
            _layout->addWidget(lab, 2);

            QLineEdit* tax = new QLineEdit(_layout);
            tax->setObjectName("TaxRate");
            _layout->addWidget(tax, 2);
        }
        {
            QLabel* lab = new QLabel(tr("折扣"), _layout);
            lab->setAlignment(Qt::AlignRight);
            _layout->addWidget(lab, 2);

            QLineEdit* discount = new QLineEdit(_layout);
            discount->setObjectName("Discount");
            _layout->addWidget(discount, 2);
        }
        {
            QLabel* lab = new QLabel(tr("合计"), _layout);
            lab->setAlignment(Qt::AlignRight);
            _layout->addWidget(lab, 2);

            QLineEdit* discount = new QLineEdit(_layout);
            discount->setObjectName("Amount");
            _layout->addWidget(discount, 2);
        }
    }
}


void ConsumerDisplayFrame::updateOrder(OrderBean& ob) {
    //显示前，清空购物车
    _shopcart->clear();

    double subamount = 0.0;
    double amount = 0.0;

    foreach(FoodBean fb, ob.foods) {
        subamount += (fb.count * fb.unit_price * (1 - fb.discount));
        _shopcart->addFoodItem(fb);
    }

    //折后加税费
    amount = subamount * (1 - ob.discount)  + subamount * ob.taxRate; //这里不计算小费 + m_ob.gratuity;

    qDebug() << "sub amount: " << subamount << ", amount: " << amount;

    {
        QLineEdit* le = dynamic_cast<QLineEdit*>(_layout->getItembyObjectName("SubTotal"));
        le->setText("$" + QString::number(subamount, 10, 2) );
    } {
        QLineEdit* le = dynamic_cast<QLineEdit*>(_layout->getItembyObjectName("TaxRate"));
        le->setText(QString::number(ob.taxRate * 100) + "%");
    } {
        QLineEdit* le = dynamic_cast<QLineEdit*>(_layout->getItembyObjectName("Discount"));
        le->setText(QString::number(ob.discount * 100) + "%");
    } {
        QLineEdit* le = dynamic_cast<QLineEdit*>(_layout->getItembyObjectName("Amount"));
        le->setText("$" + QString::number(amount, 10, 2));
    }

}

void ConsumerDisplayFrame::resizeEvent(QResizeEvent *e) {
    QFrame::resizeEvent(e);

    //如果 有两个显示屏，则客显 show
    if(QApplication::screens().length() > 1) {

        QScreen* screen = QApplication::screens().at(1);
        QRect rect = screen->geometry();

        _layout->setRowHeight( rect.height() / 10  );

        this->setGeometry( rect );
    } else {

    }

    qDebug() << QString("screen size: %1").arg(QApplication::screens().length());
}

void ConsumerDisplayFrame::showEvent(QShowEvent *e ) {
    QFrame::showEvent(e);

    if(QApplication::screens().length() <= 1) {
        this->hide();
    }
}

