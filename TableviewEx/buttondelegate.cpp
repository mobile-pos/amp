#include "buttondelegate.h"

#include <QApplication>
#include <QMouseEvent>
#include <QDialog>
#include <QPainter>
#include <QStyleOption>
#include <QDesktopWidget>

ButtonDelegate::ButtonDelegate(const QString& label, QObject *parent) :
    QItemDelegate(parent),
    _label(label)
{
}

void ButtonDelegate::paint(QPainter *painter, const QStyleOptionViewItem &option, const QModelIndex &index) const
{
    QStyleOptionButton* button = m_btns.value(index);
    if (!button) {
        button = new QStyleOptionButton();
        button->rect = option.rect.adjusted(-4, -2, -4, -2);
        button->text = _label;
        button->state |= QStyle::State_Enabled;

        (const_cast<ButtonDelegate *>(this))->m_btns.insert(index, button);
    }
    painter->save();

    if (option.state & QStyle::State_Selected) {
//        painter->fillRect(option.rect, option.palette.dark());
    } else {
        painter->fillRect(option.rect.adjusted(-4, -2, -4, -2), option.palette.dark());
    }
    painter->restore();
    QApplication::style()->drawControl(QStyle::CE_PushButton, button, painter);


}

bool ButtonDelegate::editorEvent(QEvent *event, QAbstractItemModel *model, const QStyleOptionViewItem &option, const QModelIndex &index)
{
    if (event->type() == QEvent::MouseButtonPress) {

//        QMouseEvent* e =(QMouseEvent*)event;

//        if (option.rect.adjusted(8, 5, -8, -5).contains(e->x(), e->y()) && m_btns.contains(index)) {
//            m_btns.value(index)->state |= QStyle::State_Sunken;
//        }
    }
    if (event->type() == QEvent::MouseButtonRelease) {
        QMouseEvent* e =(QMouseEvent*)event;

        if (option.rect.adjusted(-4, -2, -4, -2).contains(e->x(), e->y()) && m_btns.contains(index)) {
            m_btns.value(index)->state &= (~QStyle::State_Sunken);

            emit(onClicked(index));
        }
    }
}
