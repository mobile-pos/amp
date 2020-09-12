#include "tabviewpagerframe.h"
#include <QLabel>
#include <QDebug>

#define PAGECONTROLID 10000

TabViewPagerFrame::TabViewPagerFrame(QWidget *parent) :
    QFrame(parent),
    _curPage(0)
{
    init();
}

TabViewPagerFrame::~TabViewPagerFrame()
{
}

void TabViewPagerFrame::init() {
    _layout = new GridLayoutEx(1, this);

    //默认为1列，实际需要根据具体的来设置
    _layout->addWidget(new POSLabelEx(QObject::tr("页码控制器"), _layout));
}
void TabViewPagerFrame::setPager(int total, int pagesize) {
    this->_total = total;
    this->_pageSize = pagesize;

    int count = total / pagesize;
    if( total % pagesize != 0) {
        count += 1;
    }

    if( _layout != nullptr ) {
        _layout->clean();
        delete _layout;
        _layout = nullptr;
    }

   /**
//如果  page count > 5 ，则中间用省略号代替

_____ [ prev page ] [1] [2] [...] [ pagecount - 2] [ pagecount - 1] [next page]

//如果小于 5，则直接显示页码

_________________ [1] [2] [3] [4] [5]
*/
    QRect rect = this->geometry();
    //只有一行，10列
    _layout = new GridLayoutEx(10, this);
    _layout->setGeometry(0, 0, rect.width(), rect.height());

//    _layout->setStyleSheet("background-color: rgb(0,0,0)");
    _layout->setRowHeight( rect.height() );

    if(count < 5)  {
        {
            _layout->addWidget( new QLabel(_layout), 10 - count);
        }
        for(int i = 0; i < count; i++) {
            VKeyItemButton* _kb = new VKeyItemButton( QString::number(i + 1), this, _layout);
            _layout->addWidget(_kb);
        }

    } else {
        {
            _layout->addWidget( new QLabel(_layout), 3);
        }
        //
        {
            VKeyItemButton* _kb = new VKeyItemButton( "prevPage", this, _layout);
            _kb->setText(tr("上一页"));
            _layout->addWidget(_kb);
        }
        {
            VKeyItemButton* _kb = new VKeyItemButton( QString::number(1), this, _layout);
            _layout->addWidget(_kb, 1, 1, PAGECONTROLID);
        }
        {
            VKeyItemButton* _kb = new VKeyItemButton( QString::number(2), this, _layout);
            _layout->addWidget(_kb, 1, 1, PAGECONTROLID + 1);
        }
        {
            VKeyItemButton* _kb = new VKeyItemButton( "...", this, _layout);
            _layout->addWidget(_kb, 1, 1, PAGECONTROLID + 2);
        }
        {
            VKeyItemButton* _kb = new VKeyItemButton( QString::number(count - 2), this, _layout);
            _layout->addWidget(_kb, 1, 1, PAGECONTROLID + 3);
        }
        {
            VKeyItemButton* _kb = new VKeyItemButton( QString::number(count - 1), this, _layout);
            _layout->addWidget(_kb, 1, 1, PAGECONTROLID + 4);
        }
        {
            VKeyItemButton* _kb = new VKeyItemButton( "nextPage", this, _layout);
            _kb->setText(tr("下一页"));
            _layout->addWidget(_kb);
        }
    }

    _layout->show();
}

void TabViewPagerFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    if( _layout == nullptr ) {
        return;
    }

    QRect rect = this->geometry();
    _layout->setGeometry(0, 0, rect.width(), rect.height());
}

void TabViewPagerFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

    if(_layout != nullptr) {

        QRect rect = this->geometry();
        _layout->setGeometry(0, 0, rect.width(), rect.height());

        qDebug() << "layout : " << _layout->geometry();
        _layout->show();
    }
}

void TabViewPagerFrame::onKeyDown(QString value) {
    //下一页
    int count = this->_total / this->_pageSize;
    if( this->_total / this->_pageSize != 0) {
        count += 1;
    }

    if( "prevPage" == value ) {
        //上一页
        if( _curPage > 0 ) {
            _curPage -= 1;
        }
        emit(toPage(_curPage));

    } else if( "nextPage" == value ) {

        if( _curPage < count ) {
            _curPage += 1;
        }

        emit(toPage(_curPage));

    } else if( "..." == value ) {
        //不可操作
    } else if( 0 < value.toInt()) {
        //具体的页码
        _curPage = value.toInt() - 1;
        emit(toPage(_curPage));
    }

}
