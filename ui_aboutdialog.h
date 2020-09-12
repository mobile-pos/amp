/********************************************************************************
** Form generated from reading UI file 'aboutdialog.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_ABOUTDIALOG_H
#define UI_ABOUTDIALOG_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QDialog>
#include <QtWidgets/QGridLayout>
#include <QtWidgets/QLabel>
#include <QtWidgets/QLineEdit>
#include <QtWidgets/QPlainTextEdit>
#include <QtWidgets/QPushButton>
#include <QtWidgets/QWidget>

QT_BEGIN_NAMESPACE

class Ui_AboutDialog
{
public:
    QPushButton *OK;
    QWidget *gridLayoutWidget;
    QGridLayout *gridLayout;
    QLabel *label;
    QLineEdit *_version;
    QLabel *label_2;
    QPlainTextEdit *_contact;
    QLabel *_compName;

    void setupUi(QDialog *AboutDialog)
    {
        if (AboutDialog->objectName().isEmpty())
            AboutDialog->setObjectName(QString::fromUtf8("AboutDialog"));
        AboutDialog->resize(668, 305);
        OK = new QPushButton(AboutDialog);
        OK->setObjectName(QString::fromUtf8("OK"));
        OK->setGeometry(QRect(570, 10, 81, 51));
        gridLayoutWidget = new QWidget(AboutDialog);
        gridLayoutWidget->setObjectName(QString::fromUtf8("gridLayoutWidget"));
        gridLayoutWidget->setGeometry(QRect(0, 0, 551, 231));
        gridLayout = new QGridLayout(gridLayoutWidget);
        gridLayout->setObjectName(QString::fromUtf8("gridLayout"));
        gridLayout->setContentsMargins(0, 0, 0, 0);
        label = new QLabel(gridLayoutWidget);
        label->setObjectName(QString::fromUtf8("label"));

        gridLayout->addWidget(label, 1, 0, 1, 1);

        _version = new QLineEdit(gridLayoutWidget);
        _version->setObjectName(QString::fromUtf8("_version"));
        _version->setReadOnly(true);

        gridLayout->addWidget(_version, 1, 1, 1, 1);

        label_2 = new QLabel(gridLayoutWidget);
        label_2->setObjectName(QString::fromUtf8("label_2"));
        label_2->setLayoutDirection(Qt::LeftToRight);
        label_2->setAlignment(Qt::AlignRight|Qt::AlignTop|Qt::AlignTrailing);

        gridLayout->addWidget(label_2, 2, 0, 1, 1);

        _contact = new QPlainTextEdit(gridLayoutWidget);
        _contact->setObjectName(QString::fromUtf8("_contact"));
        _contact->setReadOnly(true);

        gridLayout->addWidget(_contact, 2, 1, 1, 1);

        _compName = new QLabel(AboutDialog);
        _compName->setObjectName(QString::fromUtf8("_compName"));
        _compName->setGeometry(QRect(0, 250, 541, 31));
        _compName->setAlignment(Qt::AlignCenter);

        retranslateUi(AboutDialog);

        QMetaObject::connectSlotsByName(AboutDialog);
    } // setupUi

    void retranslateUi(QDialog *AboutDialog)
    {
        AboutDialog->setWindowTitle(QCoreApplication::translate("AboutDialog", "\345\205\263\344\272\216", nullptr));
        OK->setText(QCoreApplication::translate("AboutDialog", "\347\241\256\345\256\232", nullptr));
        label->setText(QCoreApplication::translate("AboutDialog", "\347\211\210\346\234\254\345\217\267", nullptr));
        label_2->setText(QCoreApplication::translate("AboutDialog", "\350\201\224\347\263\273", nullptr));
        _compName->setText(QCoreApplication::translate("AboutDialog", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class AboutDialog: public Ui_AboutDialog {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_ABOUTDIALOG_H
