#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTranslator>


#include "homeframe.h"
#include "menuframe.h"
#include "orderpainter.h"
#include "settinghomeframe.h"

#include "loginframe.h"
#include "posbuttonex.h"
#include "consumerdisplayframe.h"

#include "redis-cli/redisclient.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE


class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow() override;

    void initUI();

private slots:
    void onSetting();
    void onMaintain();
    void onOpenCashBoxClick();
    void onSwithcLang();
    void onAboutClicked();
    void onGuQingSetting();
    void onShowReport(); //查看报表

    void onDineInClicked(); //堂食
    void onBookingUnpaiedOrders(); //预约订单
    void onUnpaiedOrders(); //未付订单
    void onPaiedOrders(); //已付订单
    void onOrderingOrders(); //正在订单单中的订单，草稿

    void onLoginOut(); //注册当前用户
public slots:
    void openMenuFrame(const OrderBean& order);
    void ignoreClose();


private:
    void resizeEvent(QResizeEvent *event) override;
    void showEvent(QShowEvent *e) override;
    void closeEvent(QCloseEvent *event) override;
    void hideEvent(QHideEvent* e) override;
private:
    void initStatusBar();
    void initToolbar();
    void initKeyboard();
    void initOrderTimer();

    void initHomeFrame();
    void initMenuFrame();

    //重新加载文件资源
    void retranslateUI();
public:
    HomeFrame* Home();
    MenuFrame* FoodMenu();
    SettingHomeFrame* SetHome();

    void Login(bool isAdmin = false);

    //更新登录状态
    void updateLoginInfo();
public:
    void recordPOSEvent(const QString& event, const QString& log);
private:
    Ui::MainWindow *ui;

public slots:
    void connected();
    void disconnected();
    void onReply(const QString& cmd, Reply value);

private:
    // toolbar items
    POSButtonEx* m_toolbarSetting;
    POSButtonEx* m_toolbarLocker;
    POSButtonEx* m_toolbarMaintain;
    POSButtonEx* m_toolbarLang;

    // frame
    HomeFrame* m_home;
    MenuFrame* m_menu;
    //同屏客显
    ConsumerDisplayFrame* _conDisp;
    //后台管理
    SettingHomeFrame* _setHome;
    //登录
    LoginFrame* _loginFrm;

    //状态栏信息动态显示
    POSLabelEx* _userName;
    POSLabelEx* _isLogin;

    QLabel* _toolbarSpace;
    QTranslator translator;

private:
    RedisClient *_rediscli;

};


extern MainWindow* gWnd;
#endif // MAINWINDOW_H
