#include "flavorsettingframe.h"
#include "datathread.h"
#include <QMessageBox>
#include <QDebug>

#include "mainwindow.h"
extern MainWindow* gWnd;

static FlavorOptButton* _gSelOptBtn = nullptr;
static QString _lastBtnStyle = "";
/////////////////////////////////////////////////////////
/// \brief FlavorOptButton::FlavorOptButton
/// \param opt
/// \param pWnd
/// \param parent
///
FlavorOptButton::FlavorOptButton(const QString& opt, FlavorSettingFrame* pWnd, QWidget* parent)
    : QToolButton(parent)
    , _opt(opt)
    , _pWnd(pWnd)
{

    this->setText(opt);
    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
    this->connect(this, SIGNAL(setOption(QString)), pWnd, SLOT(setOption(QString)));

//    this->setStyleSheet("background-color: rgb(26, 27, 25)");
    this->styleSheet();
}

void FlavorOptButton::onClicked() {
    qDebug() << "flavor opt button is clicked";
    if(nullptr != _gSelOptBtn) {
        _gSelOptBtn->setStyleSheet(_lastBtnStyle);
    }

    _gSelOptBtn = this;

    _lastBtnStyle = this->styleSheet();

    _gSelOptBtn->setStyleSheet("background-color: rgb(205, 226, 155)");

    emit(setOption(_opt));
}

/////////////////////////////////////////////////////////
/// \brief FlavorItemButton::FlavorItemButton
/// \param opt
/// \param pWnd
/// \param parent
///
FlavorItemButton::FlavorItemButton(const QString& item, FlavorSettingFrame* pWnd, QWidget* parent)
    : QToolButton(parent)
    , _item(item)
    , _pWnd(pWnd)
{
    this->setText(item);
    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
    this->connect(this, SIGNAL(setItem(QString)), pWnd, SLOT(setItem(QString)));
    this->setStyleSheet("background-color: rgb(59, 61, 58)");
}

void FlavorItemButton::onClicked() {
    emit(setItem(_item));
}

/////////////////////////////////////////////////////////
/// \brief FlavorSettingFrame::FlavorSettingFrame
/// \param parent
///
/// /////////////////////////////////////////////////////
FlavorSettingFrame::FlavorSettingFrame(QWidget *parent) :
    QFrame(parent)
{
    _flavorSel = new POSLabelEx("已选择：", this);

    //加载口味配置
    _optGL = new GridLayoutEx(5, this);
    _itemGL = new GridLayoutEx(5, this);

    _doneBtn = new POSButtonEx(QObject::tr("已选好"), this);
    _cancelBtn = new POSButtonEx(QObject::tr("取消"), this);
    _delLastBtn = new POSButtonEx(QObject::tr("撤销"), this);

    _doneBtn->setStyleSheet("background-color: rgb(120, 88, 120)");
    _cancelBtn->setStyleSheet("background-color: rgb(120, 88, 40)");
    _delLastBtn->setStyleSheet("background-color: rgb(168, 88, 120)");

    this->connect(_doneBtn, SIGNAL(released()), this, SLOT(onDoneClicked()));
    this->connect(_cancelBtn, SIGNAL(released()), this, SLOT(onCancelClicked()));
    this->connect(_delLastBtn, SIGNAL(released()), this, SLOT(onDeleteLastSel()));

    qDebug() << "FlavorSettingFrame::init";
    initFlavors();
    qDebug() << "FlavorSettingFrame::init end";
}

FlavorSettingFrame::~FlavorSettingFrame()
{
}

void FlavorSettingFrame::resizeEvent(QResizeEvent *event) {
     QFrame::resizeEvent(event);
     /*
+--------------+
| label info  -+
+--------------+
| option info -+
+--------------+
| flavor item -+
+--------------+
+ ok cancel ---+
*/
     QRect wndRect = gWnd->frameGeometry();
     //
     QRect rect = this->frameGeometry();
     _flavorSel->setGeometry(0, 0, rect.width(), 30);

     _optGL->setGeometry(0, 35, this->frameSize().width(), (int)(rect.height() * 0.3));

     int itemGLH = (int)(rect.height() * 0.7) - 95;
     _itemGL->setRowHeight( itemGLH / 5);
     _itemGL->setGeometry(0, (int)(rect.height() * 0.3) + 40,
                          this->frameSize().width(),
                          itemGLH);

     _doneBtn->setGeometry(10, rect.bottom() - 55, 80, 50);
     _cancelBtn->setGeometry(10 + 80 + 10, rect.bottom() - 55, 80, 50);
     _delLastBtn->setGeometry(10 + 80 + 10 + 80 + 30, rect.bottom() - 55, 80, 50);

}

void FlavorSettingFrame::initFlavors() {
    _optGL->clean();
    _itemGL->clean();

    QVariantMap flavors = DataThread::inst().loadFlavors();
    if(flavors.count("options") == 0 || flavors.count("items") == 0){
        //不是有效的口味配置文件
        QMessageBox::warning(nullptr, "错误", "无法加载口味设置，配置格式不合法");
        return;
    }
    QVariantMap opt = flavors["options"].toMap();
    QVariantList langOpt = opt[ DataThread::inst().getLanguage() ].toList();

    int i = 0;
    foreach( QVariant o, langOpt) {
        QString name = o.toString();
        FlavorOptButton* fob = new FlavorOptButton(name, this, _optGL);
        if( (++i%5) % 2 == 0 ) {
            fob->setStyleSheet("background-color: rgb(144, 180, 115)");
        } else {
            fob->setStyleSheet("background-color: rgb(96, 121, 76)");
        }
        fob->setObjectName("flavor opt:" + name);

        _optGL->addWidget(fob);
    }


    QVariantMap item = flavors["items"].toMap();
    QVariantList langItem = item[ DataThread::inst().getLanguage() ].toList();

    foreach( QVariant o, langItem) {
        QString name = o.toString();
        FlavorItemButton* fib = new FlavorItemButton(name, this, _itemGL);

        if( (++i%5) % 2 == 0 ) {
            fib->setStyleSheet("background-color: rgb(110, 110, 110)");
        } else {
            fib->setStyleSheet("background-color: rgb(125, 125, 125)");
        }
        fib->setObjectName("flavor item:" + name);

        _itemGL->addWidget(fib);
    }

}

void FlavorSettingFrame::retranslateUI() {
    this->initFlavors();
}


void FlavorSettingFrame::setOption(QString opt) {
    _optSel = opt;
}
void FlavorSettingFrame::setItem(QString item) {
    if( _optSel.length() == 0) {
        return;
    }

    _flavor.push_back(_optSel + ": " + item);

    _flavorSel->setText( _flavorSel->text() + "|" + _optSel + ": " + item);
    _flavorSel->update();
}

void FlavorSettingFrame::onDoneClicked()  {
    QString ret = "";
    foreach(QString f, _flavor) {
        ret += f;
        ret += "|";
    }
    emit(done(ret));
    this->hide();
    this->_flavor.clear();
    this->_flavorSel->setText( QObject::tr("已选择：") );
}
void FlavorSettingFrame::onCancelClicked()  {

    emit(cancel());
    this->hide();
    this->_flavor.clear();
    this->_flavorSel->setText( QObject::tr("已选择：") );
}

void FlavorSettingFrame::onDeleteLastSel() {
    if(_flavor.length() > 0) {
        _flavor.pop_back();
    }

    QString sel = QObject::tr("已选择：");
    foreach(QString item, _flavor) {
        sel += item;
        sel += "|";
    }
    _flavorSel->setText(sel);
}
