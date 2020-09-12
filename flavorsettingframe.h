#ifndef FLAVORSETTINGDLG_H
#define FLAVORSETTINGDLG_H

#include <QLabel>
#include <QDialog>
#include <QPushButton>

#include "gridlayoutex.h"
#include "posbuttonex.h"

class FlavorSettingFrame;


class FlavorOptButton : public QToolButton
{
    Q_OBJECT

public:
    FlavorOptButton(const QString& opt, FlavorSettingFrame* pWnd, QWidget* parent = nullptr);
public slots:
    void onClicked();
signals:
    void setOption(QString opt);
private:
    QString _opt;
    FlavorSettingFrame* _pWnd;
};

class FlavorItemButton : public QToolButton
{
    Q_OBJECT

public:
    FlavorItemButton(const QString& item, FlavorSettingFrame* pWnd, QWidget* parent = nullptr);
public slots:
    void onClicked();
signals:
    void setItem(QString item);
private:
    QString _item;
    FlavorSettingFrame* _pWnd;
};

class FlavorSettingFrame : public QFrame
{
    Q_OBJECT

public:
    explicit FlavorSettingFrame(QWidget *parent = nullptr);
    ~FlavorSettingFrame() override;

public:
    void resizeEvent(QResizeEvent *) override;

    void retranslateUI();
public slots:
    void setOption(QString opt);
     void setItem(QString item);

     void onDoneClicked();
     void onCancelClicked();

     void onDeleteLastSel();
signals:
     void done(const QString& flavor);
     void cancel();

private:
    void initFlavors();
private:
    //header
    GridLayoutEx* _optGL;
    GridLayoutEx* _itemGL;

    QString _optSel;
    QList<QString> _flavor; //已选择好的口味

    POSButtonEx* _doneBtn;
    POSButtonEx* _cancelBtn;
    POSButtonEx* _delLastBtn; //删除最后一次选择的数据

    //显示已选好的口味
    POSLabelEx* _flavorSel;
};

#endif // FLAVORSETTINGDLG_H
