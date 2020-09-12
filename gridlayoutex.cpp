#include "gridlayoutex.h"

#include <QDebug>
#include <iterator>
#include <QMapIterator>

GLEPageButton::GLEPageButton(int pid, QWidget* parent)
    : POSToolButtonEx(parent)
    , _pageId(pid)
{
    this->connect(this, SIGNAL(pageChanged(int)), parent, SLOT(pageChanged(int)));
}

GLEPageButton::~GLEPageButton() {

}


GLENextPageButton::GLENextPageButton(int pid, QWidget* parent)
    : GLEPageButton(pid, parent)
{
    this->setText(tr("下一页"));
    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
    this->setStyleSheet("background-color: rgb(128, 64, 128)");
}
GLENextPageButton::~GLENextPageButton() {

}

void GLENextPageButton::onClicked() {
    emit(pageChanged(_pageId + 1));
}


GLEPrevPageButton::GLEPrevPageButton(int pid, QWidget* parent)
    : GLEPageButton(pid, parent)
{
    this->setText(tr("上一页"));
    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
    this->setStyleSheet("background-color: rgb(64, 128, 64)");
}
GLEPrevPageButton::~GLEPrevPageButton() {

}

void GLEPrevPageButton::onClicked() {
    emit(pageChanged(_pageId - 1));
}



///////////////////////////////////////////////////////////////
GridLayoutEx::GridLayoutEx(int columns,
                           QWidget *parent) :
    QFrame(parent)
  , _rowHeight(50)
  , _columnWidth(80)
  , _columns(columns)
  , _border(1)
  , _pageId(0)
  , _lastPageId(0)
{
    _items = new QList<LayoutItem>();
    //默每个page都会有两个按钮
    this->_prevPage = new GLEPrevPageButton(_pageId, this);
    this->_nextPage = new GLENextPageButton(_pageId, this);

    this->_prevPage->hide();
    this->_nextPage->hide();
}

GridLayoutEx::~GridLayoutEx()
{
}

void GridLayoutEx::resizeEvent(QResizeEvent* event) {
    QFrame::resizeEvent(event);

    resetPages();

}


bool GridLayoutEx::existsItem(QList<LayoutItem>::iterator& itbefor, QWidget* obj, QRect& rect) {

    QList<LayoutItem>::iterator it = _items->begin();
    for(; it != _items->end() && it != itbefor; it++) {
        //
        int pid = it->_pageId;
        if( pid != _pageId) {
            //只更新当前页的大小
            it->_obj->hide();
            continue;
        }
        if( it->_obj->isHidden() ) {
            continue;
        }

        QWidget* _obj = it->_obj;
        if(_obj == obj) {
            continue;
        }

        QRect _ir = _obj->geometry();

        // rect 的左上角或是右下角，在 _ir的区间内
        // _ir 的左上角或是右下角在rect的区间内
        if( rect.left() >= _ir.left() && rect.top() >= _ir.top()
                && rect.left() < _ir.right() && rect.top() < _ir.bottom()) {
            return true;
        } else if( rect.right() >= _ir.left() && rect.bottom() >= _ir.top()
                   && rect.right() <= _ir.right() && rect.bottom() <= _ir.bottom()) {
            return true;
        } else if( _ir.left() >= rect.left() && _ir.top() >= rect.top()
                && _ir.left() <= rect.right() && _ir.top() <= rect.bottom()) {
            return true;
        } else if( _ir.right() >= rect.left() && _ir.bottom() >= rect.top()
                   && _ir.right() <= rect.right() && _ir.bottom() <= rect.bottom()) {
            return true;
        }
    }

    return false;
}
void GridLayoutEx::resetPages() {

    QRect rect = this->frameGeometry();

    if(5 > this->_rowHeight ) {
        this->_rowHeight = 5;
    }

    double rows = rect.height() / this->_rowHeight;

//    qDebug() << this->objectName() << " resize event be called in gridlayoutex ...";
//    qDebug() << this->objectName() << " rows: " << rows << ", row height: " << this->_rowHeight;

    if(rows == 0.0) {
        return;
    }

    QList<LayoutItem>::iterator it = _items->begin();

    int size = _items->size();

    int curPageItemCount = 0;
    for(; it != _items->end(); it++, curPageItemCount++) {
        int totalItemsInPage = rows * this->_columns;
        if(totalItemsInPage == 0) {
            return;
        }

        if( totalItemsInPage >= size) {
            _lastPageId = curPageItemCount / totalItemsInPage;

        } else if( totalItemsInPage > 2 )  {
            _lastPageId = curPageItemCount / ( totalItemsInPage - 2);
        } else {
            _lastPageId = curPageItemCount / totalItemsInPage;
        }
        it->_pageId = _lastPageId;

//        qDebug() << this->objectName() << ", obj: " << it->_obj->objectName() << ", pageID: " << it->_pageId;
    }

    resizeItems();
}
void GridLayoutEx::resizeItems() {

//    qDebug() << this->objectName() << " resize items in this layout be called ...";

    QRect rect = this->geometry();
    if(5 > this->_rowHeight ) {
        this->_rowHeight = 5;
    }

    double rows = rect.height() / this->_rowHeight;
    this->_columnWidth = rect.width() / this->_columns;

    int w = this->_columnWidth, h = this->_rowHeight;
    int r = 0, c = 0 ;

    int i = 0;

    QList<LayoutItem>::iterator it = _items->begin();
    for(; it != _items->end(); it++) {
        //
        int pid = it->_pageId;
        if( pid != _pageId) {
            //只更新当前页的大小
            it->_obj->hide();
            continue;
        }

//        qDebug() << it->_obj->objectName()  << ", pageid: " << it->_pageId;
        QWidget* obj = it->_obj;

        QRect _ir;
        do {
            c = i % (this->_columns);
            r = (c == 0 ? ++r : r);

            _ir = QRect (c * w + _border, (r - 1) * h + _border,
                         it->_colMerge * w - _border,
                         it->_rowMerge * h - _border);

            if( existsItem(it, obj, _ir) == true) {
                i += 1;
            } else {
                break;
            }
        }while(true);
        //如果这区间有其它组件，则向下一个尝试
        obj->setGeometry(_ir);
        obj->show();

        i += it->_colMerge;
    }

    //显示页控制按钮

    //如果 qwidget 是GLEPageButton, 则放到最后，
    //如果一页能显示完所有的 qwidget，则不显示 GLEPageButton
    if( this->_columns >= 2) {
        this->_prevPage->setGeometry(( this->_columns - 2) * w,  (rows - 1) * h, w, h );

     } else if(this->_columns == 1) {
        this->_prevPage->setGeometry(( this->_columns - 1) * w,  (rows - 2) * h, w, h );

    }


    this->_nextPage->setGeometry(( this->_columns - 1) * w,  (rows - 1) * h, w, h );

    if( this->_pageId == 0 ) {
        this->_prevPage->hide();
    } else {
        this->_prevPage->show();
    }

    if( this->_pageId == this->_lastPageId) {
        this->_nextPage->hide();

        this->_prevPage->setGeometry(( this->_columns - 1) * w,  (rows - 1) * h, w, h );
    } else if( this->_lastPageId > 0) {
        this->_nextPage->show();
    }
}

void GridLayoutEx::showEvent(QShowEvent* event) {
    QFrame::showEvent(event);

    resizeItems();
}

void GridLayoutEx::pageChanged(int toPage) {
    qDebug() << " page chage to: " << toPage;
    this->_pageId = toPage;
    //更新所有的控制页 page id
    this->_prevPage->updatePageId(toPage);
    this->_nextPage->updatePageId(toPage);

    this->resizeItems();
}

void GridLayoutEx::addWidget( QWidget* item, int colMerge, int rowMerge, int id) {
    //
    LayoutItem li;
    li._obj = item;
    li._pageId = 0;
    li._rowMerge = rowMerge;
    li._colMerge = colMerge;
    if( id == 0) {
        li._id = this->_items->length();
    } else {
        li._id = id;
    }

    this->_items->push_back(li);

}
QWidget* GridLayoutEx::getItembyID(int id) {

    foreach (LayoutItem li, *this->_items) {
        if( li._id == id) {
            return li._obj;
        }
    }
    qDebug() << "not found object in gridlayout[" << this->objectName() << "] with id: " << id;
    return nullptr;
}
QWidget* GridLayoutEx::getItembyObjectName(const QString& oname) {

    foreach (LayoutItem li, *this->_items) {
        if( li._obj->objectName() == oname) {
            return li._obj;
        }
    }
    qDebug() << "not found object in gridlayout[" << this->objectName() << "] with object name: " << oname;
    return nullptr;
}

void GridLayoutEx::clean() {
    QList<LayoutItem>::iterator it = _items->begin();
    for(; it != _items->end(); it++) {
        it->_obj->hide();
    }
    _items->erase(_items->begin(), it);

    this->_pageId = 0;
}
