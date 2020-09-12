#ifndef LOGINFRAME_H
#define LOGINFRAME_H

/**
  登录主页面，可以负责系统设置的用户登录和操作员登录
  */
#include <QFrame>
#include "gridlayoutex.h"
#include "vkeyboardex.h"
#include "posbuttonex.h"

class LoginFrame : public QFrame
{
    Q_OBJECT

public:
    explicit LoginFrame(QWidget *parent = nullptr);
    ~LoginFrame() override;
public:
    void resizeEvent(QResizeEvent*) override;

    void login(bool isAdmin);
private:
    void init();

public slots:
    void onKeyDown(const QString&);
    void onVKeyDown(const QString&);

    void focussed(QWidget* _this, bool hasFocus);

private:
    GridLayoutEx* _layout;
    /**
     * @brief _role
     * 30 业务经理, 经理有设置权限
     * 31~39是业务员，只是操作权限
     */
    int _role;
    bool _isLoggedIn;

    VKeyboardEx* _vkb;
};

#endif // LOGINFRAME_H
