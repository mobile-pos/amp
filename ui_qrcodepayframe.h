/********************************************************************************
** Form generated from reading UI file 'qrcodepayframe.ui'
**
** Created by: Qt User Interface Compiler version 5.13.0
**
** WARNING! All changes made in this file will be lost when recompiling UI file!
********************************************************************************/

#ifndef UI_QRCODEPAYFRAME_H
#define UI_QRCODEPAYFRAME_H

#include <QtCore/QVariant>
#include <QtWidgets/QApplication>
#include <QtWidgets/QFrame>
#include <QtWidgets/QLabel>

QT_BEGIN_NAMESPACE

class Ui_QRCodePayFrame
{
public:
    QLabel *qrcodeView;
    QLabel *sidLabel;

    void setupUi(QFrame *QRCodePayFrame)
    {
        if (QRCodePayFrame->objectName().isEmpty())
            QRCodePayFrame->setObjectName(QString::fromUtf8("QRCodePayFrame"));
        QRCodePayFrame->resize(311, 266);
        qrcodeView = new QLabel(QRCodePayFrame);
        qrcodeView->setObjectName(QString::fromUtf8("qrcodeView"));
        qrcodeView->setGeometry(QRect(20, 20, 281, 191));
        qrcodeView->setAlignment(Qt::AlignCenter);
        sidLabel = new QLabel(QRCodePayFrame);
        sidLabel->setObjectName(QString::fromUtf8("sidLabel"));
        sidLabel->setGeometry(QRect(16, 230, 281, 20));
        sidLabel->setAlignment(Qt::AlignCenter);

        retranslateUi(QRCodePayFrame);

        QMetaObject::connectSlotsByName(QRCodePayFrame);
    } // setupUi

    void retranslateUi(QFrame *QRCodePayFrame)
    {
        QRCodePayFrame->setWindowTitle(QCoreApplication::translate("QRCodePayFrame", "Frame", nullptr));
        qrcodeView->setText(QCoreApplication::translate("QRCodePayFrame", "TextLabel", nullptr));
        sidLabel->setText(QCoreApplication::translate("QRCodePayFrame", "TextLabel", nullptr));
    } // retranslateUi

};

namespace Ui {
    class QRCodePayFrame: public Ui_QRCodePayFrame {};
} // namespace Ui

QT_END_NAMESPACE

#endif // UI_QRCODEPAYFRAME_H
