#ifndef GRIDLAYOUTEX_H
#define GRIDLAYOUTEX_H

#include <QFrame>
#include <QWidget>
#include <QToolButton>
#include "posbuttonex.h"

class GLEPageButton : public POSToolButtonEx
{
    Q_OBJECT
public:
    explicit GLEPageButton(int pageId, QWidget* parent = nullptr);
    ~GLEPageButton() override;

public:
    inline void updatePageId(int pageId) { _pageId = pageId;}
signals:
    void pageChanged(int);
protected:
    int _pageId;
};

class GLENextPageButton : public GLEPageButton {
    Q_OBJECT
public:
    explicit GLENextPageButton(int pageId, QWidget* parent = nullptr);
    ~GLENextPageButton() override;
public slots:
    void onClicked();
};


class GLEPrevPageButton : public GLEPageButton {
    Q_OBJECT
public:
    explicit GLEPrevPageButton(int pageId, QWidget* parent = nullptr);
    ~GLEPrevPageButton() override;
public slots:
    void onClicked();
};


class GridLayoutEx : public QFrame
{
    Q_OBJECT

    class LayoutItem {
    public:
        LayoutItem(): _obj(nullptr), _pageId(0), _rowMerge(1), _colMerge(1) {}
    public:
        QWidget* _obj;
        int _id; //当前的item id
        int _pageId;
        int _rowMerge; //行合并的数量
        int _colMerge; //列合并的数量
    };

public:
    explicit GridLayoutEx(int columns = 5, QWidget *parent = nullptr);
    ~GridLayoutEx() override;

public:
    void addWidget(QWidget* item, int colMerge = 1, int rowMerge = 1, int id = 0);

    void clean();
    inline void setRowHeight(double h = 30) { this->_rowHeight = h;}

    void setCellBord(int border = 5) { this->_border = border;};
public:
    void resizeEvent(QResizeEvent* event) override;
    void showEvent(QShowEvent* event) override;

public slots:
    void pageChanged(int);
public:
    //重置pageid
    void resetPages();
    void resizeItems();
    bool existsItem(QList<LayoutItem>::iterator& itbefor, QWidget* obj, QRect& rect);

    QWidget* getItembyID(int id);
    QWidget* getItembyObjectName(const QString& oname);
private:
    double _rowHeight;
    int _columnWidth;

    int _columns; //列数需要指定，行数，是根据页面大小调整
    int _border;

    GLEPrevPageButton* _prevPage;
    GLENextPageButton* _nextPage;
    int _pageId;
    int _lastPageId;
    QList<LayoutItem>* _items;
};

#endif // GRIDLAYOUTEX_H
