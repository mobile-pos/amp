#ifndef CONSUMERTYPEFRAME_H
#define CONSUMERTYPEFRAME_H
/****
 * 消费者类型变更操作面板
 *
*/
#include <QFrame>
#include <QLineEdit>
#include <QTextEdit>
#include <QLabel>
#include <QComboBox>
#include <QRadioButton>

#include "gridlayoutex.h"
#include "vkeyboardex.h"
#include "orderpainter.h"
#include "redis-cli/redisclient.h"
#include "posbuttonex.h"

using namespace Redis;

class ConsumerTypeFrame : public QFrame
{
    Q_OBJECT

public:
    explicit ConsumerTypeFrame(QWidget *parent = nullptr);
    ~ConsumerTypeFrame() override;

private:
    void init();

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

    void retranslateUI();
public:
    void updateCurName(const QString& type, const QString& name, const QString& addr);
signals:
    void done(const QString& type, const QString& name, const QString& addr);
public slots:
    void onVKeyDown(const QString&);
    void onKeyDown(const QString&);
    //redis cli slots
    void connected();
    void disconnected();
    void onReply(const QString& cmd, Reply value);

    void on_tangchi_type_clicked();
    void on_delevery_type_clicked();
    void on_pickup_type_clicked();

    void onInputFinished(int code, const QString& txt);
    //选择历史
    void currentTextChanged(const QString &);

    void onDeleveryClicked();
    void onPickupSelfClicked();
private:
    void updateTable(QVariantMap tab);
private:
    GridLayoutEx* _leftControl;
    VKeyboardEx* _vkb; // 键盘

    GridLayoutEx* _tablayout; //桌号
    RedisClient* _rediscli;

    QMap<QString, QVariantMap>* _tabs;

    POSLabelEx* _lname;
    LineEditEx* _name;
    POSLabelEx* _laddr;
    LineEditEx* _addr;

    QRadioButton* _isDelevery;
    QRadioButton* _isPickupSelf;
    QString _type;
    //上次一选中的
    QWidget* _lastbtn;

    //消费者历史地址
    QComboBox* _consAddresses;

};

#endif // CONSUMERTYPEFRAME_H
