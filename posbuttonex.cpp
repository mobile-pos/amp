#include "posbuttonex.h"
#include <QDebug>

#include <QMap>


static QMap<POSLineEditEx*, QString> _edits;
static QMap<POSCheckBoxEx*, QString> _chboxs;
static QMap<POSLabelEx*, QString> _labs;
static QMap<POSButtonEx*, QString> _btns;
static QMap<POSToolButtonEx*, QString> _tbtns;


/////////////////////////////////////////////////
POSLineEditEx::POSLineEditEx(QWidget *parent) : QLineEdit(parent)
{
    _edits[this] = "";

}

POSLineEditEx::~POSLineEditEx() {
    if(_edits.keys().indexOf(this) >= 0) {
        _edits.erase( _edits.find(this));
    }
}

void POSLineEditEx::setPlaceholderText(const QString &text) {
    QLineEdit::setPlaceholderText(text);

    if(_edits[this] == "") {
        _edits[this] = text;
    }
}
/////////////////////////////////////////////////
POSCheckBoxEx::POSCheckBoxEx(QWidget *parent) : QCheckBox(parent)
{
    _chboxs[this] = "";

}
POSCheckBoxEx::POSCheckBoxEx(const QString &text, QWidget *parent): QCheckBox(text, parent) {
    _chboxs[this] = text;
}

POSCheckBoxEx::~POSCheckBoxEx() {
    if(_chboxs.keys().indexOf(this) >= 0) {
        _chboxs.erase( _chboxs.find(this));
    }
}

void POSCheckBoxEx::setText(const QString &text) {
    QCheckBox::setText(text);

    if(_chboxs[this] == "") {
        _chboxs[this] = text;
    }
}


///////////////////////////////////////////////////////

POSLabelEx::POSLabelEx(QWidget *parent) : QLabel(parent)
{
    _labs[this] = "";

}
POSLabelEx::POSLabelEx(const QString &text, QWidget *parent): QLabel(text, parent) {
    _labs[this] = text;
}

POSLabelEx::~POSLabelEx() {
    QMap<POSLabelEx*, QString>::iterator it = _labs.find(this);
    if(it != _labs.end()) {
        _labs.erase( it );
    }
}

void POSLabelEx::setText(const QString &text) {
    QLabel::setText(text);

    _labs[this] = text;
}


///////////////////////////////////////////////////////

POSButtonEx::POSButtonEx(QWidget *parent) : QPushButton(parent)
{
    _btns[this] = "";

}
POSButtonEx::POSButtonEx(const QString &text, QWidget *parent): QPushButton(text, parent) {
    _btns[this] = text;
}

POSButtonEx::~POSButtonEx() {
    if(_btns.keys().indexOf(this) >= 0) {
        _btns.erase( _btns.find(this));
    }
}

void POSButtonEx::setText(const QString &text) {
    QPushButton::setText(text);

    if(_btns[this] == "") {
        _btns[this] = text;
    }
}


///////////////////////////////////////////////////////

POSToolButtonEx::POSToolButtonEx(QWidget *parent) : QToolButton(parent)
{
    _tbtns[this] = "";

}

POSToolButtonEx::~POSToolButtonEx() {
    if(_tbtns.keys().indexOf(this) >= 0) {
        _tbtns.erase( _tbtns.find(this));
    }
}

void POSToolButtonEx::setText(const QString &text) {
    QToolButton::setText(text);

    if(_tbtns[this] == "") {
        _tbtns[this] = text;
    }
}


///////////////////////////////////////////////////////

namespace POS {

 void retranslateUI()  {
    foreach (POSLabelEx* item, _labs.keys()) {

//        qDebug() << "button text: " << item->text();

        item->setText( QObject::tr( _labs[item].toStdString().c_str()));
    }

    foreach (POSButtonEx* item, _btns.keys()) {

//        qDebug() << "button text: " << item->text();

        item->setText( QObject::tr( _btns[item].toStdString().c_str()));
    }

    foreach (POSToolButtonEx* item, _tbtns.keys()) {

//        qDebug() << "button text: " << item->text();

        item->setText( QObject::tr( _tbtns[item].toStdString().c_str()));
    }


    foreach (POSCheckBoxEx* item, _chboxs.keys()) {

//        qDebug() << "button text: " << item->text();

        item->setText( QObject::tr( _chboxs[item].toStdString().c_str()));
    }

    foreach (POSLineEditEx* item, _edits.keys()) {

        qDebug() << "button text: " << item->placeholderText();

        item->setPlaceholderText(QObject::tr( _edits[item].toStdString().c_str()));
    }
}

}
