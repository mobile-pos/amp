#ifndef INSTALLWIZARD_H
#define INSTALLWIZARD_H

#include <QWizard>
#include <QListWidget>
#include <QProcess>
#include <QTextEdit>
#include <QThread>
#include <QMap>
#include <QComboBox>
#include <QRadioButton>

#include "vkeyboardex.h"


class PrinterSetPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit PrinterSetPage(QWidget *parent = nullptr) : QWizardPage(parent){
        _isSuccess = false;
    }

public:
    void setInstallStatus(bool isSuccess) {
        _isSuccess = isSuccess;
    }
    bool isSuccess() {
       return _isSuccess;
    }
private:
    bool _isSuccess;
public:
    QMap<QString, QComboBox* > _prints;
};

class InstallWPage : public QWizardPage
{
    Q_OBJECT
public:
    explicit InstallWPage(QWidget *parent = nullptr) : QWizardPage(parent){
        _isSuccess = false;
    }

public:
    void setInstallStatus(bool isSuccess) {
        _isSuccess = isSuccess;
    }
    bool isSuccess() {
       return _isSuccess;
    }
private:
    bool _isSuccess;
};

class InstallWizard : public QWizard
{
    Q_OBJECT

public:
    explicit InstallWizard(QWidget *parent = nullptr);
    ~InstallWizard() override;

private:
    enum {
        PAGE_FIRST,
        PAGE_PRINTER,
        PAGE_INSTALL,
        PAGE_SETTING,
        PAGE_FINISH
    };

    QWizardPage* firstPage();
    QWizardPage* printerSetPage();
    QWizardPage* installPage();
    // setting for android 即云POS
    QWizardPage* settingPage();

    QWizardPage* finishPage();

    bool validateCurrentPage() override;

    bool install();
    // setting for android 即云POS
    bool setting();

private:
    QVariantMap userLogin();
public slots:

    void readOutput();
    void callFinished(int , QProcess::ExitStatus);

    //打印机
    void printControl();
    void modifyPrinter();
public:
    LineEditEx* _account;
    LineEditEx* _passwd;

    PrinterSetPage *_printpage;
    InstallWPage* _installpage;

    LineEditEx* _rid;

    QRadioButton* _isCnServer;
    QRadioButton* _isUsServer;
    QRadioButton* _isLocalServer; //本地服务器

    QTextEdit* _installlog;

    QProcess* _process;
};

#endif // INSTALLWIZARD_H
