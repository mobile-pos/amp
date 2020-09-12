#ifndef VKEYBOARDEX_H
#define VKEYBOARDEX_H

#include <QFrame>
#include <QToolButton>
#include <QLineEdit>
#include <QDialog>

#include "gridlayoutex.h"
#include "posbuttonex.h"

class VKeyboardDlg;
class LineEditEx : public POSLineEditEx
{
    Q_OBJECT
public:
    /**
     * @brief LineEditEx
     * @param parent
     * @param ownVK 是否自带虚拟键弹窗
     */
    LineEditEx(QWidget* parent, bool ownVK = false);

private:
    void showVK(bool active);
signals:
    void focussed(QWidget* _this, bool hasFocus);

    void onInputFinished(int code, const QString& txt);
protected:
    virtual void mouseReleaseEvent(QMouseEvent *);
    virtual void focusOutEvent(QFocusEvent *e);


private:
    VKeyboardDlg* _vkdlg;
    bool _ownVK;
};

class VKeyItemButton : public POSToolButtonEx
{
    Q_OBJECT
public:
    VKeyItemButton(const QString& value = "", QWidget* pWnd = nullptr, QWidget* parent = nullptr);
    ~VKeyItemButton() override;
public slots:
    void onClicked();
signals:
    void onKeyDown(const QString&);
};

class VKeyboardEx : public QFrame
{
    Q_OBJECT

public:
    explicit VKeyboardEx(QWidget *parent = nullptr);
    ~VKeyboardEx() override;

public:
    void init(int type = 0); // 1 number board, other for full board
    inline void setTarget(LineEditEx* target) {
        _target = target;
    }

    inline void setCellBord(int bord) {
        _layout->setCellBord(bord);
    }
public:
    void resizeEvent(QResizeEvent*) override;
private:
    void initNumber1KB();
    void initNumber2KB();
    void initNumber3KB();

    void initFullKB();

public slots :
    void onKeyDown(const QString&);

signals:
    void onVKeyDown(const QString&);
private:
    GridLayoutEx* _layout;

    LineEditEx* _target;

    int _kbType;
};


class VKeyboardDlg : public QDialog
{
    Q_OBJECT
public:
    explicit VKeyboardDlg(LineEditEx* target, QWidget *parent = nullptr);
    ~VKeyboardDlg() override;
public:
    void resizeEvent(QResizeEvent *) override;

    virtual void focusOutEvent(QFocusEvent *e) override;
public slots :
    void onVKeyDown(const QString&);
private:
    VKeyboardEx* _vk;
};

#endif // VKEYBOARDEX_H
