#ifndef TABLESFRAME_H
#define TABLESFRAME_H

#include <QFrame>
#include <QToolButton>
#include <QList>
#include <QThread>
#include "OrderData.h"
#include "redis-cli/redisclient.h"
#include "gridlayoutex.h"
#include "appordersframe.h"

using namespace Redis;


class TablesFrame;

/**
 * @brief The KaiTaiDlg class
 * 开台对话框，主要是实现，就餐人数和拼桌功能
 *
 */
class KaiTaiDlg : public QFrame
{
    Q_OBJECT
public:
    KaiTaiDlg(QWidget* parent = nullptr);
public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent *event) override;
public:
    void addTab(const QString& tab) ;
public slots:
    void onKeyDown(const QString&);
    void onVKeyDown(const QString&);

private:
    void kaiTai();
private:
    QList<QString> _tabs; //拼桌的量

    GridLayoutEx* _layout;
    VKeyboardEx* _vkb;

    bool _bmergeTab; //是否拼桌
};

class TabPushButton : public QToolButton
{
    Q_OBJECT
public:
    TabPushButton(const QString& tname, TablesFrame* target, QWidget* parent = nullptr);
public:
    inline int getTabId() { return m_tid;}

private slots:
    void onClicked();

signals:
    void onTabClicked(const QString& tname);
private:
    int m_tid;
    TablesFrame* m_pWnd;
};

class TableItem {
public:
    TableItem() {
        id = 0;
        satdown = 0;
        holdTime = 0;
        oid = 0;

        btn = nullptr;

        name = "";
    }
public:
    int id;
    int satdown;
    QString name;
    int holdTime;
    QString satTime;

    //绑定的订单号
    int oid; //订单号
public:
    TabPushButton* btn;
};


class TablesFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TablesFrame(QWidget *parent = nullptr);
    ~TablesFrame() override;
public:
    void resizeEvent(QResizeEvent *e) override;
    void showEvent(QShowEvent* e) override;
public:
    void init();
private:
    //台桌信息存入缓存中
    void tab2Redis(const QString& tname);
    void updateTable(QVariantMap tab);

public:
    void updateTables();
    int getOrderId(const QString& tabName);

    //重置台桌
    void resetTable(const QString&);
public slots:
    void connected();
    void disconnected();
    void onReply(const QString& cmd, Reply value);
    void onMonitor4TablStatus();
    void onTabClicked(const QString& tname);

private:
    GridLayoutEx* _layout;
    AppOrdersFrame* _appFrm;

    KaiTaiDlg* _ktFrm;


    QMap<QString, TableItem> m_tables;

public:
    RedisClient* _rediscli;
};

#endif // TABLESFRAME_H
