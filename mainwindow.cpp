#include <QPushButton>
#include <QLabel>
#include <QHBoxLayout>
#include <QProcess>
#include <QMessageBox>
#include <QNetworkInterface>
#include <QApplication>

#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "datathread.h"
#include "aboutdialog.h"

#define TOOLBARHEIGHT 40
#define TOOLBARITEMHEIGHT TOOLBARHEIGHT

MainWindow* gWnd = nullptr;

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_home(nullptr)
    , _conDisp(nullptr)
{
    _rediscli = new RedisClient(this);
    _rediscli->connectToServer(REDISSERVER, REDISPORT);

    if (translator.load("zh", ":/languages")) { // 可以不带".qm"后缀名
        qApp->installTranslator(&translator);
    } else {
        QMessageBox::warning(this, "出错", "多语言(zh)资源加载失败");
    }


    gWnd = this;

    initUI();
}

void MainWindow::initUI() {

//    this->setStyleSheet("background-color: rgb(168, 168, 168)");
    ui->setupUi(this);


    _loginFrm = new LoginFrame(this);
    _loginFrm->hide();

    _setHome = new SettingHomeFrame(this);
    _setHome->hide();

    if(QApplication::screens().length() > 1) {
        _conDisp = new ConsumerDisplayFrame(nullptr);
    } else {
        qDebug() << "没有客显，不显示";
    }

    qDebug() << "MainWindow::init";
    initKeyboard();
    initToolbar();
//    initStatusBar();

    initHomeFrame();

    initMenuFrame();

    this->initStatusBar();
    qDebug() << "MainWindow::init end";

}

void MainWindow::initHomeFrame() {
    m_home = new HomeFrame(this);
    //
}

void MainWindow::initMenuFrame() {
    m_menu = new MenuFrame(this);
    m_menu->setConsumerDisplay(_conDisp);
    m_menu->hide();
}

MainWindow::~MainWindow()
{
    delete ui;

    _conDisp = nullptr;
}

void MainWindow::resizeEvent(QResizeEvent *event)  {

    QMainWindow::resizeEvent(event);

    if(nullptr != _conDisp) {
        _conDisp->resizeEvent(event);
    }

    qDebug() << " show event ok ";
    QRect geo = this->geometry();

    geo.setX(0);
    geo.setY( this->ui->toolBar->height() );
    geo.setBottom( this->ui->statusbar->geometry().top());

    m_home->setMaximumSize( this->frameSize());
    m_home->setGeometry(geo);

    m_menu->setMaximumSize( this->frameSize());
    m_menu->setGeometry(geo);

    _loginFrm->setMaximumSize(this->frameSize());
    _loginFrm->setGeometry(geo);

    _setHome->setMaximumSize(this->frameSize());
    _setHome->setGeometry(geo);

//    _toolbarSpace->setMinimumWidth(10 * (this->frameGeometry().width()/10 - 120) );

    this->initToolbar();
    this->ui->statusbar->show();

}
void MainWindow::showEvent(QShowEvent *e) {
    QMainWindow::showEvent(e);
    if(nullptr != _conDisp) {
        _conDisp->showFullScreen();
    }

    if(false == DataThread::inst().isLoggedIn()) {
        Login();
    }

    static bool first = true;
    if(true == first) {
        DataThread::inst().start();
        first = false;
    }
}

void MainWindow::ignoreClose() {
    QMessageBox::warning(this, tr("提醒"), tr("主窗口不能关闭"));
}
void MainWindow::closeEvent(QCloseEvent *event) {
    QMainWindow::closeEvent(event);
//    event->ignore();

//    QMessageBox::warning(this, tr("提醒"), tr("主窗口不能关闭"));

}
void MainWindow::hideEvent(QHideEvent* e) {
    QMainWindow::hideEvent(e);
//    e->ignore();

//    QMessageBox::warning(this, tr("提醒"), tr("主窗口不能隐藏"));
}

void MainWindow::initKeyboard() {

}

void MainWindow::initStatusBar() {
    QStatusBar* statusbar = ui->statusbar;

    statusbar->setGeometry(0, 0, this->width(), TOOLBARHEIGHT / 2);
    statusbar->setMinimumHeight(TOOLBARHEIGHT / 2);
    statusbar->setMaximumHeight(TOOLBARHEIGHT / 2);

    static bool isInited = false;
    if(true == isInited) {
        return;
    }
    isInited = true;
    {

        //获取本地IP片地址,
        QList<QHostAddress> addList = QNetworkInterface::allAddresses();

        foreach(QHostAddress address,addList)
        {
            //排除IPV6，排除回环地址
            if(address.protocol() == QAbstractSocket::IPv4Protocol
                    && address != QHostAddress(QHostAddress::LocalHost))
            {
                //输出，转换为字符串格式
//                qDebug() << address.toString();
                statusbar->addWidget( new QLabel(address.toString(), statusbar));
                break;
            }
        }
    } {
        //显示登录状态
        _isLogin = new POSLabelEx(QObject::tr("未登录"), statusbar);
        statusbar->addWidget( _isLogin);
    } {
        statusbar->addWidget( new QLabel(statusbar));
    } {
        //显示登录状态
        statusbar->addWidget( new POSLabelEx( QObject::tr("登录用户名:"), statusbar));
        _userName = new POSLabelEx(tr("未知"), statusbar);
        statusbar->addWidget( _userName);
    } {
        statusbar->addWidget( new QLabel(statusbar));
    } {
        //显示店家名
        statusbar->addWidget( new POSLabelEx(QObject::tr("店家："), statusbar));
        statusbar->addWidget( new QLabel(gRestaurant["name"].toString(), statusbar));
        statusbar->addWidget( new POSLabelEx(QObject::tr("地址："), statusbar));
        statusbar->addWidget( new QLabel(gRestaurant["address"].toString(), statusbar));
        statusbar->addWidget( new POSLabelEx(QObject::tr("电话："), statusbar));
        statusbar->addWidget( new QLabel(DataThread::inst().getRestaurantInfo().value("phoneNumbers").toString(), statusbar));
    } {
        //显示税率
        statusbar->addWidget( new POSLabelEx(QObject::tr("税率："), statusbar));
        double taxrate = DataThread::inst().getRestaurantInfo().value("taxRate").toDouble();
        statusbar->addWidget( new QLabel(QString::number(taxrate * 100) + "%", statusbar));
    }
}
void MainWindow::updateLoginInfo() {
    if( DataThread::inst().isLoggedIn() ) {
        _isLogin->setText(QObject::tr("已登录"));

        _userName->setText( DataThread::inst().activeUserName() );
    } else {
        _isLogin->setText(QObject::tr("登录失败"));
        _userName->setText(QObject::tr( "未知" ));
    }
}

void MainWindow::initToolbar() {

    QToolBar* toolbar = ui->toolBar;
    toolbar->clear();

    toolbar->setFloatable(false);
    toolbar->setMovable(false);

    toolbar->setGeometry(0, 0, this->width(), TOOLBARHEIGHT);
    toolbar->setMinimumHeight(TOOLBARHEIGHT + 10);
//    toolbar->setMaximumHeight(TOOLBARHEIGHT);

    int width = this->width();
    int bwidth = width / 18;

    {
        m_toolbarSetting = new POSButtonEx(QObject::tr("设置"));

        m_toolbarSetting->setMinimumSize(bwidth, TOOLBARITEMHEIGHT);
        this->connect(m_toolbarSetting, SIGNAL(released()), this, SLOT(onSetting()));
        toolbar->addWidget(m_toolbarSetting);
    }
    toolbar->addSeparator();

    {
        QLabel *lab = new QLabel(toolbar);
        lab->setMinimumWidth( bwidth * 3);
        toolbar->addWidget(lab);
    }
//    toolbar->addSeparator();s
    {
        //订单
//        POSButtonEx* _tc = new POSButtonEx(this);
//        _tc->setText(QObject::tr("堂食"));
//        _tc->setMinimumSize(bwidth, TOOLBARITEMHEIGHT);
//        this->connect(_tc, SIGNAL(released()), this, SLOT(onDineInClicked()));
//        toolbar -> addWidget(_tc);

        //订单
        POSButtonEx* _booking = new POSButtonEx(this);
        _booking->setText(QObject::tr("预约订单"));
        _booking->setMinimumSize(bwidth * 2, TOOLBARITEMHEIGHT);
        this->connect(_booking, SIGNAL(released()), this, SLOT(onBookingUnpaiedOrders()));
        toolbar -> addWidget(_booking);

//        //订单
//        POSButtonEx* _unpaied = new POSButtonEx(this);
//        _unpaied->setText(QObject::tr("未付订单"));
//        _unpaied->setMinimumSize(bwidth * 2, TOOLBARITEMHEIGHT);
//        this->connect(_unpaied, SIGNAL(released()), this, SLOT(onUnpaiedOrders()));
//        toolbar -> addWidget(_unpaied);


//        POSButtonEx* _paied = new POSButtonEx(this);
//        _paied->setText(QObject::tr("已付订单"));
//        _paied->setMinimumSize(bwidth * 2, TOOLBARITEMHEIGHT);
//        this->connect(_paied, SIGNAL(released()), this, SLOT(onPaiedOrders()));

//        toolbar -> addWidget(_paied);

        POSButtonEx* _ordering = new POSButtonEx(this);
        _ordering->setText(QObject::tr("订单草稿"));
        _ordering->setMinimumSize(bwidth * 2, TOOLBARITEMHEIGHT);
        this->connect(_ordering, SIGNAL(released()), this, SLOT(onOrderingOrders()));

        toolbar -> addWidget(_ordering);
    }
    toolbar->addSeparator();

    {
        QLabel *lab = new QLabel(toolbar);
        lab->setMinimumWidth( bwidth * 2);
        toolbar->addWidget(lab);
    }

//    toolbar->addSeparator();
    {
        POSButtonEx* _locker = new POSButtonEx(tr("钱箱"));
        _locker->setMinimumSize(bwidth, TOOLBARITEMHEIGHT);
        _locker->setMaximumHeight(TOOLBARITEMHEIGHT);
        this->connect(_locker, SIGNAL(released()), this, SLOT(onOpenCashBoxClick()));

        toolbar-> addWidget(_locker);

        m_toolbarMaintain = new POSButtonEx(tr("维护"));
        m_toolbarMaintain->setMinimumSize(bwidth, TOOLBARITEMHEIGHT);
        this->connect(m_toolbarMaintain, SIGNAL(released()), this, SLOT(onMaintain()));

        toolbar-> addWidget(m_toolbarMaintain);

        POSButtonEx* _logout = new POSButtonEx(tr("注销"));
        _logout->setMinimumSize(bwidth, TOOLBARITEMHEIGHT);
        this->connect(_logout, SIGNAL(released()), this, SLOT(onLoginOut()));

        toolbar-> addWidget(_logout);

    }
    toolbar->addSeparator();
    {
        QLabel *lab = new QLabel(toolbar);
        lab->setMinimumWidth( bwidth * 1.5);
        toolbar->addWidget(lab);
    }
    {
        //语言切换
        m_toolbarLang = new POSButtonEx("");
        m_toolbarLang->setMinimumSize(bwidth, TOOLBARITEMHEIGHT);
        m_toolbarLang->setIcon(QIcon(":/res/img/china.png"));

        DataThread::inst().setLanguage("zh_CN");
        this->connect(m_toolbarLang, SIGNAL(released()), this, SLOT(onSwithcLang()));
        toolbar->addWidget(m_toolbarLang);
    }

    {
        QLabel *lab = new QLabel(toolbar);
        lab->setMinimumWidth( bwidth * 0.5);
        toolbar->addWidget(lab);
    }
    {
        POSButtonEx* about = new POSButtonEx(tr("关于"));
        about->setMinimumSize(bwidth, TOOLBARITEMHEIGHT);
        this->connect(about, SIGNAL(released()), this, SLOT(onAboutClicked()));
        toolbar->addWidget(about);
    }
}

void MainWindow::retranslateUI() {
   POS::retranslateUI();

   m_home->retranslateUI();
   m_menu->retranslateUI();

   _setHome->retranslateUI();
}

void MainWindow::onSetting() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }

    static bool sys = false;
    if(false == sys) {

        SetHome();

        m_toolbarSetting->setText(QObject::tr("前台"));

        sys = true;
    } else {

        this->Home();

        m_toolbarSetting->setText(QObject::tr("设置"));

        sys = false;
    }
}

void MainWindow::onMaintain() {
    //取消最大化窗口，并启动x11vnc
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }

    static bool maintain = false;

    if(false == maintain) {
        QProcess::execute(QString("/bin/bash /opt/sbin/maintain.sh %1")
                          .arg(15900 + DataThread::inst().getRestaurantInfo().value("rid").toInt()));
        this->m_toolbarMaintain->setText(QObject::tr("运营"));
        this->showNormal();
        maintain = true;
    } else {
        QProcess::execute("killall ssh");
        this->m_toolbarMaintain->setText(QObject::tr("维护"));
        this->showFullScreen();
        //需要调用x11vnc

        maintain = false;
    }
}
void MainWindow::onOpenCashBoxClick() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }
    //开钱箱
#define OPENCASHBOXREQ "pos:cashbox:event:req:open"
//    _redis->sendCommand(Command::PUBLISH(OPENCASHBOXREQ, "1"));
    _rediscli->sendCommand(Command::PUBLISH(OPENCASHBOXREQ, "1"));
    this->recordPOSEvent("cashbox", "open");
//    _redis->sendCommand(Command::HSET( "pos:log:event:cashbox", )
}

void MainWindow::onAboutClicked() {
    AboutDialog about(this);
    about.exec();
}

void MainWindow::onSwithcLang() {
    static bool ch = true;
    if(true == ch) {
        DataThread::inst().setLanguage("en_US");

        m_toolbarLang->setIcon(QIcon(":/res/img/us.png"));

//        this->m_toolbarLang->setText(tr("English"));

        qApp->removeTranslator(&translator);

        if (translator.load("en", ":/languages")) {
            if(false == qApp->installTranslator(&translator)) {
                qDebug() << "切换语言失败(en)";
            }
        } else {
            QMessageBox::warning(this, "出错", "多语言(en)资源加载失败");
        }

        ch = false;
    } else {

        DataThread::inst().setLanguage("zh_CN");

        m_toolbarLang->setIcon(QIcon(":/res/img/china.png"));
//        this->m_toolbarLang->setText(tr("中文"));

        qApp->removeTranslator(&translator);

        if (translator.load("zh", ":/languages")) {
            if(false == qApp->installTranslator(&translator)) {
                qDebug() << "切换语言失败(zh)";
            }
        } else {
            QMessageBox::warning(this, "出错", "多语言(zh)资源加载失败");
        }

        ch = true;
    }

    this->retranslateUI();

}
void MainWindow::onGuQingSetting() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }
    SetHome();

    _setHome->onKeyDown("Estimate");
}
void MainWindow::onShowReport() {
    if(false == DataThread::inst().isAdminLoggedin()) {
        Login();
        return;
    }

}

void MainWindow::onDineInClicked() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }

    Home();
    this->m_home->on_m_tangshi_clicked();
}

void MainWindow::onBookingUnpaiedOrders() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }

    Home();
    this->m_home->onBookingUnpaiedOrdersClicked();
}

void MainWindow::onUnpaiedOrders() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }

    Home();
    this->m_home->on_m_allUnpaiedOrders_clicked();
}
void MainWindow::onPaiedOrders() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }

    Home();
    this->m_home->on_m_allPaiedOrders_clicked();
}

void MainWindow::onOrderingOrders() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return;
    }

    Home();
    this->m_home->on_m_allOrderingOrders_clicked();
}


void MainWindow::openMenuFrame(const OrderBean& order){
    this->m_menu->setOrderBean(order);
    this->FoodMenu();
}
HomeFrame* MainWindow::Home() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return nullptr;
    }

    _setHome->hide();
    this->m_menu->hide();
    this->m_home->showMaximized();

    return m_home;
}
MenuFrame* MainWindow::FoodMenu() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return nullptr;
    }

    this->m_home->hide();
    _setHome->hide();
    this->m_menu->showMaximized();

    return m_menu;
}
SettingHomeFrame* MainWindow::SetHome() {
    if(false == DataThread::inst().isLoggedIn()) {
        Login();
        return nullptr;
    }

    m_home->hide();
    m_menu->hide();

    _setHome->show();

    return _setHome;
}

void MainWindow::Login(bool isAdmin) {
    this->m_menu->hide();
    this->m_home->hide();
    _setHome->hide();

    _loginFrm->login(isAdmin);
}

void MainWindow::onLoginOut() {
    DataThread::inst().logout();

    this->updateLoginInfo();

    if(false == DataThread::inst().isLoggedIn()) {
        Login();
    }
}


void MainWindow::connected() {
    _rediscli->sendCommand(Command::Auth(REDISAUTH));

    qDebug() << "redis connect to local server OK";
    _rediscli->sendCommand(Command::Auth(REDISAUTH));
}
void MainWindow::disconnected() {
    usleep(2 * 1000 * 1000);
    _rediscli->connectToServer(REDISSERVER, REDISPORT); //重连
}
void MainWindow::onReply(const QString& cmd, Reply value) {
    //
    qDebug() << "MainWindow::onReply cmd: " << cmd << ", value: " << value.value().toString();
    QString key = cmd.toLower().trimmed();
    if(key.startsWith("auth")) {

    }
}
void MainWindow::recordPOSEvent(const QString& event, const QString& log) {
    QString dt = QDateTime::currentDateTime().toString("yyyy-MM-dd HH:mm:ss");
    QString key = QString("pos:log:event:%1:%2").arg(event).arg(gRID);
    _rediscli->sendCommand( Command::HSET(key, dt,
                                       QString("[%1] %2").arg(DataThread::inst().activeUserName()).arg(log)));
    _rediscli->sendCommand(Command::EXPIRE(key, 86400 * 10));
}
