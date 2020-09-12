#include "vkeyboardex.h"
#include <QLabel>
#include <QDebug>
#include <QDialog>
#include <QApplication>
#include <QDesktopWidget>


//////////////////////////////////////////////////////////


LineEditEx::LineEditEx(QWidget* parent, bool ownVK)
    : POSLineEditEx(parent),
      _vkdlg(nullptr),
      _ownVK(ownVK) {
    //创建一个非模式的对话框，并把 vkeyboard 绑定到上面

    qDebug() << "LineEditEx init ....";

    if(true == _ownVK) {
        _vkdlg = new VKeyboardDlg(this, this->parentWidget());
    }
    qDebug() << "LineEditEx init end";
}

void LineEditEx::mouseReleaseEvent(QMouseEvent *e) {
    QLineEdit::mouseReleaseEvent(e);

    emit(focussed(this, true));

    showVK(true);
}
void LineEditEx::focusOutEvent(QFocusEvent *e) {
    QLineEdit::focusOutEvent(e);
    emit(focussed(this, false));

    showVK(false);
}

void LineEditEx::showVK(bool active) {
    if(nullptr == _vkdlg) {
        return;
    }
    if(true == active) {
        //调整位置
//        QRect rect = this->frameGeometry();

        QDesktopWidget* desktopWidget = QApplication::desktop();
            //获取可用桌面大小
        QRect rect = desktopWidget->availableGeometry();

        QPoint ppos = QCursor::pos(); //mapFromGlobal(mapToGlobal({rect.left(), rect.bottom() + 10}));

        if( ppos.y() + 300 > rect.height() ) {
            ppos.setY( ppos.y() - 300 - 20);
        } else {
            ppos.setY( ppos.y() + 20);
        }
        if(ppos.x() + 520 > rect.width() ) {
            ppos.setX( rect.width() - 520);
        }
        _vkdlg->setGeometry( ppos.x(),
                            ppos.y(), 500, 300);

        int code = _vkdlg->exec();
        emit(onInputFinished(code, this->text()));
    } else {
        //非焦点下，隐藏
//        _vkdlg->hide();
    }
}

//////////////////////////////////////////////////////////
/// \brief VKeyboardEx::VKeyboardEx
/// \param target
/// \param parent
///
VKeyItemButton::VKeyItemButton(const QString& value, QWidget* pWnd, QWidget* parent)
    : POSToolButtonEx(parent)
{
    this->setObjectName(value);
    this->setText(value); //默认

    this->connect(this, SIGNAL(released()), this, SLOT(onClicked()));
    this->connect(this, SIGNAL(onKeyDown(const QString&)), pWnd, SLOT(onKeyDown(const QString&)));
}

void VKeyItemButton::onClicked() {
    emit( onKeyDown( this->objectName()) );
}

VKeyItemButton::~VKeyItemButton() {}



/////////////////////////////////////////////////////////////
/// \brief VKeyboardEx::VKeyboardEx
/// \param target
/// \param parent
///
VKeyboardEx::VKeyboardEx(QWidget *parent) :
    QFrame(parent)
  , _layout(nullptr),
    _target(nullptr)
{
    qDebug() << "VKeyboardEx::init";

    this->connect(this, SIGNAL(onVKeyDown(const QString&)), parent, SLOT(onVKeyDown(const QString&)));
    init(1);

    qDebug() << "VKeyboardEx::init end";
}

VKeyboardEx::~VKeyboardEx()
{
}

void VKeyboardEx::init(int type) {
    if( _layout != nullptr) {
        delete _layout;
        _layout = nullptr;
    }
    _kbType = type;

    switch(type) {
    case 1: return initNumber1KB();
    case 2: return initNumber2KB();
    case 3: return initNumber3KB();
    default:
        return initFullKB();
    }
}

template <class T>
int getArrayLen(T &array){

    return sizeof(array) / sizeof(array[0]);
}

void VKeyboardEx::initNumber2KB() {
    _layout = new GridLayoutEx(9, this);
    _layout->setObjectName("虚拟键盘");

    QString a[] = {"Del", "Clean", "OK"};
    int a_len = getArrayLen(a);

    QString b[] = {"1", "2", "3",
                   "4", "5", "6",
                   "7", "8", "9",
                   ".", "0", "*"
                  };

    int b_len = getArrayLen(b);

    for(int i = 0; i < b_len ; i++) {

        VKeyItemButton* kb = new VKeyItemButton( QString(b[i]), this, _layout);
        kb->setStyleSheet("background-color: rgb(77, 78, 76)");
        _layout->addWidget(kb, 2);

        if( i == 2 || i == 5 || i == 8) {

            _layout->addWidget( new QLabel(_layout) );

            VKeyItemButton* kb = new VKeyItemButton( QString(a[  (i / 3) % a_len ]), this, _layout);
            if(kb->objectName() == "OK") {
                kb->setText(tr("确定"));
            }
            kb->setStyleSheet("background-color: rgb(78, 94, 102)");
            _layout->addWidget(kb, 2);

        } else if( (i + 1 ) % 3 == 0) {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 3);
        }
    }
}

void VKeyboardEx::initNumber3KB() {
    _layout = new GridLayoutEx(9, this);
    _layout->setObjectName("虚拟键盘");

    QString a[] = {"Del", "Clean"};
    int a_len = getArrayLen(a);

    QString b[] = {"1", "2", "3",
                   "4", "5", "6",
                   "7", "8", "9",
                   ".", "0", "*"
                  };

    int b_len = getArrayLen(b);

    for(int i = 0; i < b_len ; i++) {

        VKeyItemButton* kb = new VKeyItemButton( QString(b[i]), this, _layout);
        kb->setStyleSheet("background-color: rgb(77, 78, 76)");
        _layout->addWidget(kb, 2);

        if( i == 2 || i == 5 || i == 8) {

            _layout->addWidget( new QLabel(_layout) );

            VKeyItemButton* kb = new VKeyItemButton( QString(a[  (i / 3) % a_len ]), this, _layout);
            if(kb->objectName() == "OK") {
                kb->setText(tr("确定"));
            }
            kb->setStyleSheet("background-color: rgb(78, 94, 102)");
            _layout->addWidget(kb, 2);

        } else if( (i + 1 ) % 3 == 0) {
            QLabel* _l = new QLabel("", _layout);
            _layout->addWidget(_l, 3);
        }
    }
}


void VKeyboardEx::initNumber1KB() {
    _layout = new GridLayoutEx(9, this);
    _layout->setObjectName("虚拟键盘");

    QString a[] = {"ABC", "+", "-", "Del", "Clean", "OK"};
    int a_len = getArrayLen(a);

    QString b[] = {"1", "2", "3",
                   "4", "5", "6",
                   "7", "8", "9",
                   "*", "0", "#",
                   "@", "$", "%",
                   ".", "SPACE", "/"
                  };

    int b_len = getArrayLen(b);

    for(int i = 0; i < b_len ; i++) {
        VKeyItemButton* kb = new VKeyItemButton( QString(b[i]), this, _layout);
        kb->setStyleSheet("background-color: rgb(77, 78, 76)");
        _layout->addWidget(kb, 2);

        if( (i + 1) % 3 == 0) {

            _layout->addWidget( new QLabel(_layout) );

            VKeyItemButton* kb = new VKeyItemButton( QString(a[  (i / 3) % a_len ]), this, _layout);
            kb->setStyleSheet("background-color: rgb(78, 94, 102)");
            if(kb->objectName() == "OK") {
                kb->setText(QObject::tr("确定"));
            }
            _layout->addWidget(kb, 2);
        }
    }
}

void VKeyboardEx::initFullKB() {

    _layout = new GridLayoutEx(12, this);
    _layout->setObjectName("虚拟键盘");

    QString kb[] = {"1", "2", "3", "4", "5", "6", "7", "8", "9", "0", "-", "=",
                    "q", "w", "e", "r", "t", "y", "u", "i", "o", "p", "[", "]",
                    "a", "s", "d", "f", "g", "h", "j", "k", "l", ";", "'", "|",
                    "z", "x", "c", "v", "b", "n", "m", ",", ".", "?", ":", "\"",
                    "~", "!", "@", "#", "$", "%", "^", "*", "(", ")", "_", "+",
                    "123", "SPACE", "CapsLock", "Clean", "Del", "OK"
                   };
    int kb_len = getArrayLen(kb);

    for(int i = 0; i < kb_len; i++) {
        if(kb[i] == "") {
            // qlabel
            _layout->addWidget(new QLabel(_layout), 1);
        } else {
            QString k = kb[i];
            VKeyItemButton* kb = new VKeyItemButton(k, this, _layout);
            if(kb->objectName() == "OK") {
                kb->setText(QObject::tr("确定"));
            }

            if(k == "SPACE" || "OK" == k || "Clean" == k || "Del" == k || "123" == k) {
                kb->setStyleSheet("background-color: rgb(78, 94, 102)");
                _layout->addWidget(kb, 2);

            } else if( "CapsLock" == k) {
                kb->setStyleSheet("background-color: rgb(69, 70, 67)");
                _layout->addWidget(kb, 2, 1, 10000);

            } else {
                kb->setStyleSheet("background-color: rgb(77, 78, 76)");
                _layout->addWidget(kb, 1);
            }

        }
    }
}

void VKeyboardEx::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    if(_layout == nullptr) {
        return;
    }
    QRect rect = this->frameGeometry();
    if( _kbType == 2) {
        _layout->setRowHeight( rect.height() / 4);
    } else {
        _layout->setRowHeight( rect.height() / 6); //显示6行
    }
    _layout->setGeometry(0, 0, rect.width(), rect.height() );
}

void VKeyboardEx::onKeyDown(const QString& value) {
    static bool isCapsLocked = false;
    QString _value = value;
    if("SPACE" == value) {
        _value = " ";
    }

    if( _value == "ABC") {
        this->init(255);

        QRect rect = this->frameGeometry();
        _layout->setRowHeight( rect.height() / 6); //显示6行
        _layout->setGeometry(0, 0, rect.width(), rect.height() );
        _layout->show();

        return;
    } else if("123" == _value) {
        this->init(1);

        QRect rect = this->frameGeometry();
        _layout->setRowHeight( rect.height() / 6); //显示6行
        _layout->setGeometry(0, 0, rect.width(), rect.height() );
        _layout->show();

        return;
    } else if("CapsLock" == _value) {

        QWidget* _btn = _layout->getItembyID(10000);
        isCapsLocked = !isCapsLocked;

        if( true == isCapsLocked ) {
            _btn->setStyleSheet("background-color: rgb(217, 224, 131)");
        } else {
            _btn->setStyleSheet("background-color: rgb(69, 70, 67)");
        }

        return;
    } else if("Clean" == _value) {
        if(nullptr != _target) {
            _target->setText("");
        }

        emit(onVKeyDown(_value));
    } else if("Del" == _value) {
        if(nullptr != _target) {
            QString txt = _target->text();
            if( txt.length() > 0) {
            _target->setText( txt.mid(0, txt.length() - 1));
            }
        }

        emit(onVKeyDown(value));
    } else if("OK" == _value) {
       emit(onVKeyDown(value));

    } else {

        if( isCapsLocked &&
                (_value.at(0) >= 'a' && _value.at(0) <= 'z')) {
            char c = _value.toLocal8Bit().at(0);
            c -= 32;
            _value = QString(c);
        }

        if(_target != nullptr && _value.length() == 1) {
            _target->setText( _target->text() + _value);
        }
        emit(onVKeyDown(_value));
    }

}




////////////////////////////////////////////////////////
VKeyboardDlg::VKeyboardDlg(LineEditEx* target, QWidget* parent) : QDialog(parent) {
    qDebug() << "VKeyboardDlg init ....";
    setWindowFlags(windowFlags()|Qt::FramelessWindowHint);

    _vk = new VKeyboardEx(this);
    _vk->setTarget(target);

    qDebug() << "VKeyboardDlg init end";
}
VKeyboardDlg::~VKeyboardDlg() {

}

void VKeyboardDlg::resizeEvent(QResizeEvent *e) {
    QDialog::resizeEvent(e);

    _vk->setGeometry(0, 0, width(), height());
}

void VKeyboardDlg::focusOutEvent(QFocusEvent *e) {
    QDialog::focusOutEvent(e);

    this->done(0);
}
void VKeyboardDlg::onVKeyDown(const QString& value) {
    if("OK" == value) {
        this->done(0);
    }
}
