#include "cookingtimeframe.h"

#include <QDateTime>
#include <QDebug>
//////////////////////////////////////////////////////////
/// \brief VKeyboardEx::VKeyboardEx
/// \param target
/// \param parent
///
HourItemButton::HourItemButton(const QString& label, QWidget* pWnd, QWidget* parent)
    : POSToolButtonEx(parent)
{
    this->setText(label);
    this->setObjectName(label);

    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
    this->connect(this, SIGNAL(hourKeyReder(const QString&)), pWnd, SLOT(hourKeyReder(const QString&)));
}
HourItemButton::~HourItemButton(){}

void HourItemButton::onClicked() {
    emit( hourKeyReder( this->objectName()) );
}

MinuteItemButton::MinuteItemButton(const QString& label, QWidget* pWnd, QWidget* parent)
    : POSToolButtonEx(parent)
{
    this->setText(label);
    this->setObjectName(label);

    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
    this->connect(this, SIGNAL(minuteKeyReder(const QString&)), pWnd, SLOT(minuteKeyReder(const QString&)));
}
MinuteItemButton::~MinuteItemButton(){}
void MinuteItemButton::onClicked() {
    emit( minuteKeyReder( this->objectName()) );
}
//////////////////////////////////////////////////////////
/// \brief CookingTimeFrame::CookingTimeFrame
/// \param parent
///
CookingTimeFrame::CookingTimeFrame(QWidget *parent) :
    QFrame(parent)
{
    _hour = new QLineEdit(this);
    _minute = new QLineEdit(this);

    _apm = new POSToolButtonEx(this);
    this->connect(_apm, SIGNAL(released()), this, SLOT(onAmpmSwitch()));

    _split = new QLabel(":", this);

    _hoursLayout = new GridLayoutEx(3, this);
    _hoursLayout->setObjectName("小时设置面板");

    _minutesLayout = new GridLayoutEx(3, this);
    _minutesLayout->setObjectName("分钟设置面板");

    _doneBtn = new POSToolButtonEx(this);
    _doneBtn->setText(QObject::tr("完成"));
    _doneBtn->setStyleSheet("background-color: rgb(120, 88, 120)");
    this->connect(_doneBtn, SIGNAL(released()), this, SLOT(onDone()));

    _cancelBtn = new POSToolButtonEx(this);
    _cancelBtn->setText(QObject::tr("放弃"));
    _cancelBtn->setStyleSheet("background-color: rgb(120, 88, 40)");
    this->connect(_cancelBtn, SIGNAL(released()), this, SLOT(onCancel()));
    init();
}

CookingTimeFrame::~CookingTimeFrame()
{
}

void CookingTimeFrame::init() {

    qDebug() << "CookingTimeFrame::init";
    {
        _hoursLayout->addWidget(new QLabel("", _hoursLayout), 1);
        _hoursLayout->addWidget(new POSLabelEx(QObject::tr("点击选择小时"), _hoursLayout), 2);
        for(int i = 0; i < 12; i++) {
            HourItemButton* btn = new HourItemButton(QString("%1").arg(i, 2, 10, QLatin1Char('0')), this, _hoursLayout);
            if( (i%3) % 2 == 0 ) {
                btn->setStyleSheet("background-color: rgb(110, 110, 110)");
            } else {
                btn->setStyleSheet("background-color: rgb(125, 125, 125)");
            }
            _hoursLayout->addWidget(btn);
        }
        {
            //1小时后
            HourItemButton* btn = new HourItemButton("1Hour", this, _hoursLayout);
            btn->setText(QObject::tr("1小时后"));
            btn->setStyleSheet("background-color: rgb(110, 110, 110)");
            _hoursLayout->addWidget(btn, 2);
            _hoursLayout->addWidget(new QLabel("", _hoursLayout), 1);
        }
        {
            //2小时后
            HourItemButton* btn = new HourItemButton("2Hours", this, _hoursLayout);
            btn->setText(QObject::tr("2小时后"));
            btn->setStyleSheet("background-color: rgb(125, 125, 125)");
            _hoursLayout->addWidget(btn, 2);
            _hoursLayout->addWidget(new QLabel("", _hoursLayout), 1);
        }
        {
            //3小时后
            HourItemButton* btn = new HourItemButton("3Hours", this, _hoursLayout);
            btn->setText(QObject::tr("3小时后"));
            btn->setStyleSheet("background-color: rgb(110, 110, 110)");
            _hoursLayout->addWidget(btn, 2);
            _hoursLayout->addWidget(new QLabel("", _hoursLayout), 1);
        }
    }
    {
        _minutesLayout->addWidget(new QLabel("", _minutesLayout), 1);
        _minutesLayout->addWidget(new QLabel(QObject::tr("点击选择分钟"), _minutesLayout), 2);
        for(int i = 0; i < 60; i+=5) {
            MinuteItemButton* btn = new MinuteItemButton(QString("%1").arg(i, 2, 10, QLatin1Char('0')), this, _minutesLayout);
            if( (i%3) % 2 == 0 ) {
                btn->setStyleSheet("background-color: rgb(110, 110, 110)");
            } else {
                btn->setStyleSheet("background-color: rgb(125, 125, 125)");
            }
            _minutesLayout->addWidget(btn);
        }
        {
            //15分钟后
            MinuteItemButton* btn = new MinuteItemButton("15Minutes", this, _minutesLayout);
            btn->setText(QObject::tr("15分钟后"));
            btn->setStyleSheet("background-color: rgb(125, 125, 125)");
            _minutesLayout->addWidget(btn, 2);
            _minutesLayout->addWidget(new QLabel("", _minutesLayout), 1);
        }
        {
            //30分钟后
            MinuteItemButton* btn = new MinuteItemButton("30Minutes", this, _minutesLayout);
            btn->setText(QObject::tr("30分钟后"));
            btn->setStyleSheet("background-color: rgb(110, 110, 110)");
            _minutesLayout->addWidget(btn, 2);
            _minutesLayout->addWidget(new QLabel("", _minutesLayout), 1);
        }
        {
            //45分钟后
            MinuteItemButton* btn = new MinuteItemButton("45Minutes", this, _minutesLayout);
            btn->setText(QObject::tr("45分钟后"));
            btn->setStyleSheet("background-color: rgb(125, 125, 125)");
            _minutesLayout->addWidget(btn, 2);
            _minutesLayout->addWidget(new QLabel("", _minutesLayout), 1);
        }
    }

    qDebug() << "CookingTimeFrame::init end";
}
void CookingTimeFrame::resizeEvent(QResizeEvent *e) {
    QFrame::resizeEvent(e);
/**
+-----------------------+
|   hour : minute   done+
+-----------------------+
| 1 2 3    5 10 15 20   |
| 4 5 6    25 30 35 40  |
| 7 8 9    45 50 55 0   |
*/
        QRect rect = this->frameGeometry();

        _hour->setGeometry(20, 10, 30, 30);
        _split->setGeometry(55, 10, 5, 30);
        _minute->setGeometry(65, 10, 30, 30);

        _apm->setGeometry(100, 10, 50, 30);

        _doneBtn->setGeometry(rect.width() - 190, 10, 80, 50);
        _cancelBtn->setGeometry(rect.width() - 100, 10, 80, 50);

        _hoursLayout->setGeometry(0, 80, rect.width() / 2 - 5, rect.height() - 80);
        _minutesLayout->setGeometry(rect.width() / 2 + 5, 80, rect.width() / 2, rect.height() - 80);

}

void CookingTimeFrame::showEvent(QShowEvent *event) {
    QFrame::showEvent(event);


    QDateTime current_date_time = QDateTime::currentDateTime();

    _hour->setText( QString("%1").arg(current_date_time.time().hour() % 12, 2, 10,QLatin1Char('0')));

    _minute->setText( QString("%1").arg(current_date_time.time().minute(), 2, 10,QLatin1Char('0')));

    _apm->setText( current_date_time.time().hour()  < 12 ? "AM": "PM");

}

void CookingTimeFrame::hourKeyReder(const QString& hour) {
    qDebug() << "hour key down: " << hour;
    //获取当前时间
    QDateTime curhour = QDateTime::currentDateTime();
    int add = 0;
    if(hour == "1Hour") {
        add += 1;
    } else if(hour == "2Hours") {
        add += 2;
    } else if(hour == "3Hours") {
        add += 3;
    }
    if(add > 0) {
        QTime time = curhour.time();
        time = time.addSecs(3600 * add);
        int h = time.hour();
        if(h < 12) {
            _apm->setText("AM");
        } else {
            _apm->setText("PM");
            h -= 12;
        }

        _hour->setText( QString("%1").arg( h, 2, 10, QLatin1Char('0')));
    } else {
        _hour->setText(hour);
    }
}
void CookingTimeFrame::minuteKeyReder(const QString& minute) {
    qDebug() << "minute key down: " << minute;
    //获取当前时间
    QDateTime curhour = QDateTime::currentDateTime();
    int add = 0;
    if(minute == "15Minutes") {
        add += 15;
    } else if(minute == "30Minutes") {
        add += 30;
    } else if(minute == "45Minutes") {
        add += 45;
    }
    if(add > 0) {
        QTime time = curhour.time();
        time = time.addSecs( 60 * add);
        int h = time.hour();
        if(h < 12) {
            _apm->setText("AM");
        } else {
            _apm->setText("PM");
            h -= 12;
        }

        _hour->setText( QString("%1").arg( h, 2, 10, QLatin1Char('0')));
        _minute->setText(QString("%1").arg( time.minute(), 2, 10, QLatin1Char('0')));
    } else {
        _minute->setText(minute);
    }
}
void CookingTimeFrame::onAmpmSwitch() {
    QString cur = _apm->text();
    if( cur == "AM") {
        _apm->setText("PM");
    } else {
        _apm->setText("AM");
    }
}

void CookingTimeFrame::onDone() {
    qDebug() <<" 需要更新 订单预约时间 数据";
    this->hide();

    //设置定时间和备注
    int h = _hour->text().toInt();
    int m = _minute->text().toInt();

    if( _apm->text() == "PM" ) {
        h += 12;
    }

    //获取当前日日土已
    QDateTime dt = QDateTime::currentDateTime();
    dt.setTime( QTime(h, m, 0) );
    qDebug() << "预约时间：" << dt.toString("yyyy-MM-dd hh:mm:ss");

    emit( onCookingTimeSet( dt.toString("yyyy-MM-dd hh:mm:ss")) );
}
void CookingTimeFrame::onCancel() {

    this->hide();
}

