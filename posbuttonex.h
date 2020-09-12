#ifndef POSBUTTONEX_H
#define POSBUTTONEX_H

#include <QPushButton>
#include <QToolButton>
#include <QLabel>
#include <QCheckBox>
#include <QLineEdit>


class POSLineEditEx : public QLineEdit
{
    Q_OBJECT
public:
    explicit POSLineEditEx(QWidget *parent = nullptr);

    virtual ~POSLineEditEx();

    void setPlaceholderText(const QString &text);
};

class POSLabelEx : public QLabel
{
    Q_OBJECT
public:
    explicit POSLabelEx(QWidget *parent = nullptr);
    explicit POSLabelEx(const QString &text, QWidget *parent = nullptr);

    virtual ~POSLabelEx();

    void setText(const QString &text);

};


class POSCheckBoxEx : public QCheckBox
{
    Q_OBJECT
public:
    explicit POSCheckBoxEx(QWidget *parent = nullptr);
    explicit POSCheckBoxEx(const QString &text, QWidget *parent = nullptr);

    virtual ~POSCheckBoxEx();

    void setText(const QString &text);

};


class POSButtonEx : public QPushButton
{
    Q_OBJECT
public:
    explicit POSButtonEx(QWidget *parent = nullptr);
    explicit POSButtonEx(const QString &text, QWidget *parent = nullptr);

    virtual ~POSButtonEx();

    void setText(const QString &text);

};

class POSToolButtonEx : public QToolButton
{
    Q_OBJECT
public:
    explicit POSToolButtonEx(QWidget *parent = nullptr);

    virtual ~POSToolButtonEx();

    void setText(const QString &text);

};

namespace POS {
    void retranslateUI();
}

#endif // POSBUTTONEX_H
