#ifndef TABVIEWPAGERFRAME_H
#define TABVIEWPAGERFRAME_H
/**
  tabview 分页显示的控制器
  */
#include <QFrame>
#include "vkeyboardex.h"
#include "gridlayoutex.h"

class TabViewPagerFrame : public QFrame
{
    Q_OBJECT

public:
    explicit TabViewPagerFrame(QWidget *parent = nullptr);
    ~TabViewPagerFrame() override;

public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent*) override;

public:
    void setPager(int total, int pagesize = 10);

    int curPage() {
        return _curPage;
    }
private:
    void init();
public slots:
    void onKeyDown(QString value);
signals:
    void toPage(int);
private:
    int _pageSize; //每页显示的总行数
    int _total; //总页数
    int _curPage; //当前页码

    GridLayoutEx* _layout;
};

#endif // TABVIEWPAGERFRAME_H
