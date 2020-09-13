#include "installwizard.h"

#include <QFile>
#include <QLabel>
#include <QGridLayout>
#include <QListWidget>
#include <QMessageBox>
#include <QProcess>
#include <QCryptographicHash>
#include <QComboBox>
#include <QPrinterInfo>
#include <QPrintDialog>

#include "datathread.h"

#include "vkeyboardex.h"


void InstallWizard::readOutput() {
    QString str = "";

    str += _process->readAllStandardOutput();

    QString err = _process->readAllStandardError();
    if( err.length() > 0) {
        str += "\nError: ";
        str += err;
    }
     
    qDebug() << "output: " << str;
    _installlog->append(str.trimmed());
}

InstallWizard::InstallWizard(QWidget *parent) :
    QWizard(parent),
    _printpage(nullptr)
{
    _process = new QProcess(this);

    connect(_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readOutput()) );

    connect(_process, SIGNAL(readyReadStandardError()),
            this, SLOT(readOutput()) );

    this->connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)),
                  this, SLOT(callFinished(int, QProcess::ExitStatus)));

    this->setOption(QWizard::NoBackButtonOnStartPage);
    this->setOption(QWizard::NoBackButtonOnLastPage);
    this->setOption(QWizard::NoCancelButton);


    QList<int> pages = this->pageIds();
    foreach(int pid, pages) {
        this->removePage(pid);
    }

    this->setPage(PAGE_FIRST, firstPage());
#ifdef MPOS_CLOUD
    this->setPage(PAGE_SETTING, settingPage());
#else
    this->setPage(PAGE_PRINTER, printerSetPage());
    this->setPage(PAGE_INSTALL, installPage());
#endif
    this->setPage(PAGE_FINISH, finishPage() );

    this->setWindowFlags( windowFlags() & ~Qt::WindowContextHelpButtonHint);

    this->setWindowTitle("Install Steps");
    this->setWizardStyle(QWizard::ModernStyle);

    this->connect(this, SIGNAL(currentIdChanged(int) ), this, SLOT(indexIdChanged(int)));

}

InstallWizard::~InstallWizard()
{
}

QWizardPage* InstallWizard::firstPage() {
    QWizardPage* first = new QWizardPage();
    first->setTitle(tr("账号设置"));
    first->setSubTitle(tr("输入当前餐馆的管理员账号"));

    QGridLayout *layout = new QGridLayout();
    layout->setRowStretch(10, 1);
    layout->setColumnStretch(1, 1);
    {
        QLabel* _label = new QLabel(tr("账号:"), first);
        _account = new LineEditEx(first, true);
        _account->setText("demoroot");

        layout->addWidget(_label);
        layout->addWidget(_account);

    }
    {
        QLabel* _label = new QLabel(tr("密码:"), first);
        _passwd = new LineEditEx(first, true);
        _passwd->setEchoMode(QLineEdit::Password);
        _passwd->setText("1356@aiwaiter");

        layout->addWidget(_label);
        layout->addWidget(_passwd);
    }

    first->setLayout(layout);
    return first;
}


void InstallWizard::printControl() {

//    QProcess::execute("system-config-printer");


    QString out, err;
    if(-1 == DataThread::CallScript("system-config-printer", out, err, 5 * 60)) {
        QMessageBox::warning(nullptr, "错误", "Printer 服务启动失败: " + err);
        return;
    }

    printerSetPage();

}

void InstallWizard::modifyPrinter() {
    QPrintDialog dlg;
    dlg.exec();
}

static void clearLayout(QLayout* layout) {
    QLayoutItem *item;
    while((item = layout->takeAt(0)) != 0){
        //删除widget
        if(item->widget()){

            delete item->widget();
            //item->widget()->deleteLater();
        }
        //删除子布局
        QLayout *childLayout = item->layout();
        if(childLayout){
            clearLayout(childLayout);
        }
        delete item;
    }
}

QWizardPage* InstallWizard::printerSetPage() {
    if(_printpage == nullptr) {
        _printpage = new PrinterSetPage();
        _printpage->setTitle(tr("打印机设置"));
        _printpage->setSubTitle(tr("选择前台和后厨的小票打印机"));
    }

    QGridLayout *layout = new QGridLayout();
    if(_printpage->layout() != nullptr) {
        layout = dynamic_cast<QGridLayout*>( _printpage->layout() );

        clearLayout(layout);

    } else {
        layout = new QGridLayout();
    }

    layout->setRowStretch(1, 0);
    layout->setColumnStretch(1, 0);
    {
        //设置打印机
        QPushButton* btn = new QPushButton(tr("新增打印机"));

        this->connect(btn, SIGNAL(released()), this, SLOT(printControl()));


        QPushButton* modify = new QPushButton(tr("修改打印机"));

        this->connect(btn, SIGNAL(released()), this, SLOT(printControl()));

        this->connect(modify, SIGNAL(released()), this, SLOT(modifyPrinter()));

        layout->addWidget(btn);
        layout->addWidget(modify);
    }

    {
        QLabel* lab = new QLabel(tr("前台打印机"),  _printpage);

        QComboBox* printers = new QComboBox( _printpage);

        _printpage->_prints.insert("Cashier", printers);

        QStringList list = QPrinterInfo::availablePrinterNames();
        foreach(QString pname, list) {
            printers->addItem(pname);
        }

        layout->addWidget(lab);
        layout->addWidget(printers);
    }

    {
        QStringList list = QPrinterInfo::availablePrinterNames();
        for(int i = 1; i < list.length(); i++) {
            //只有多个打印机才配置后厨打印机

            QLabel* lab = new QLabel(tr("后台厨房打印机"), _printpage);

            QComboBox* printers = new QComboBox(_printpage);

            _printpage->_prints.insert(QString("Kitchen-%1").arg(i),
                                       printers);

            printers->addItem("");

            foreach(QString pname, list) {
                printers->addItem(pname);
            }

            layout->addWidget(lab);
            layout->addWidget(printers);
        }
    }

    _printpage->setLayout(layout);

    return _printpage;
}
QWizardPage* InstallWizard::installPage() {
    _installpage = new InstallWPage();
    _installpage->setTitle(tr("安装"));
    _installpage->setSubTitle(tr("开始同步云资源到本地"));

    QGridLayout *layout = new QGridLayout();
    layout->setRowStretch(1, 0);
    layout->setColumnStretch(1, 0);

    _installlog = new QTextEdit(_installpage);
    layout->addWidget(_installlog);

    _installlog->append(tr("开始安装 ..."));

    _installpage->setLayout(layout);

    return _installpage;
}
QWizardPage* InstallWizard::settingPage() {

    QWizardPage* setting = new QWizardPage();
    setting->setTitle(tr("设置"));

    QGridLayout *layout = new QGridLayout();
    layout->setRowStretch(1, 0);
    layout->setColumnStretch(1, 0);

    _rid = new LineEditEx(setting);
    _rid->setReadOnly(true);
    layout->addWidget(_rid);



    _isCnServer = new QRadioButton("国内服", setting);
    _isUsServer = new QRadioButton("美东服", setting);
    _isCnServer->setChecked(true);

    layout->addWidget(new QLabel("选择服务区", setting));
    layout->addWidget(_isCnServer);
    layout->addWidget(_isUsServer);

    QLabel* _info = new QLabel(setting);
    _info->setText(tr("完成设置"));

    layout->addWidget(_info);

    setting->setLayout(layout);

    return setting;

}
QWizardPage* InstallWizard::finishPage() {
    QWizardPage* finish = new QWizardPage();
    finish->setTitle(tr("完成"));

    QGridLayout *layout = new QGridLayout();
    layout->setRowStretch(1, 0);
    layout->setColumnStretch(1, 0);

    QLabel* _info = new QLabel(finish);
    _info->setText(tr("完成安装"));

    layout->addWidget(_info);

    finish->setLayout(layout);

    return finish;
}

QVariantMap InstallWizard::userLogin() {

    if( _account->text().length() == 0
            || _passwd->text().length() == 0) {
        QMessageBox::warning(nullptr, tr("出错"), tr("账号或密码不能为空"));
        return QVariantMap();
    }
    //check account is validate

    QString acc = _account->text();
    QString pwd = _passwd->text();
    QString key = acc + pwd;
    QString md5 = QString(QCryptographicHash::hash( key.toUtf8(),QCryptographicHash::Md5).toHex());

    DataThread dt;
    QVariantMap ret = dt.login(acc, md5);

    if(ret.empty() || ret.count("role") == 0 ) {

        QMessageBox::warning(nullptr, tr("出错"), tr("账号或密码无效,或是网路异常，请稍候重试"));
        return QVariantMap();
    } else if(ret["role"].toInt()!=30) {

        QMessageBox::warning(nullptr, tr("出错"), tr("当前账号不是餐馆管理员"));
        return QVariantMap();
    }

    return ret;
}
bool InstallWizard::validateCurrentPage() {
    if( -1 == this->nextId()) {

#ifdef MPOS_CLOUD
        this->done(0);
        return true;
#endif

        DataThread dt;
        QVariantMap gres = dt.loadRestInfo();
        if(gres.empty()) {
            //安装过程异常
            _installpage->setInstallStatus(false);
            QMessageBox::warning(nullptr, tr("出错"), tr("安装过程异常,请重试一次"));
            this->back();

            return false;
        }
        if( 0 != dt.updatePrinter2Cloud()) {
            _installpage->setInstallStatus(false);

            QMessageBox::warning(nullptr, tr("出错"), tr("配置参数同步到云端失败，请重试一次"));
            this->back();

            return false;
        }

        this->done(0);

    } else if( PAGE_PRINTER == this->nextId()) {
        QVariantMap ret = userLogin();

        _installlog->append(tr("账号: ") + ret["account"].toString());
        _installlog->append(tr("餐馆: ") + ret["name"].toString());
        _installlog->append(tr("地址: ") + ret["address"].toString());
        _installlog->append(tr("电话: ") + ret["phoneNumbers"].toString());

        _installlog->append("");

    } else if(PAGE_INSTALL == this->nextId()) {
        //点击了打印机设置的next
        //清空缓存
        QString receptionPrinter = _printpage->_prints["Cashier"]->currentText();
        if(receptionPrinter.length() == 0) {
            QMessageBox::warning(nullptr, tr("提醒"), tr("前台打印机不能为空, setting in app"));
            return true;
        }

        QProcess::execute("/usr/local/bin/redis-cli del printers:config ");
        //保存其他打印机
        QStringList printers = QPrinterInfo::availablePrinterNames();
        foreach(QString pn, printers) {

            QString cmd = QString("/usr/local/bin/redis-cli hset printers:config %1 idle")
                    .arg(pn);
            if(0 != QProcess::execute(cmd)) {
                QMessageBox::warning(nullptr, tr("出错"), tr("保存前台打印设置失败"));
                return false;
            }
        }

        QString cmd = QString("/usr/local/bin/redis-cli hset printers:config %1 cashier")
                .arg(receptionPrinter);

        if(0 != QProcess::execute(cmd)) {
            QMessageBox::warning(nullptr, tr("出错"), tr("保存前台打印设置失败"));
            return false;
        }

        _installlog->append(tr("前台打印机: ") + receptionPrinter);


        foreach(QString key, _printpage->_prints.keys()) {
            if(key.startsWith("Kitchen-")) {
                QString pn = _printpage->_prints[key]->currentText();
                if( pn.length() == 0) {
                    continue;
                }

                QString cmd = QString("/usr/local/bin/redis-cli hset printers:config %1 %2")
                                    .arg(pn)
                                    .arg(key.toLower());

                if(0 != QProcess::execute(cmd)) {
                    QMessageBox::warning(nullptr, tr("出错"), tr("保存前台打印设置失败"));
                    return false;
                }
                _installlog->append(QString(tr("厨房打印机(%1): %2")).arg(key.mid(QString("Kitchen-").length())).arg(pn));
            }
        }
    } else if(PAGE_SETTING == this->nextId()) {
        //检查账号对应的餐馆ID
        QVariantMap ret ;//  userLogin();
        ret["rid"] = 2;
        this->_rid->setText(ret["rid"].toString());

//        _installlog->append(tr("账号: ") + ret["account"].toString());
//        _installlog->append(tr("餐馆: ") + ret["name"].toString());
//        _installlog->append(tr("地址: ") + ret["address"].toString());
//        _installlog->append(tr("电话: ") + ret["phoneNumbers"].toString());

//        _installlog->append("");

    } else if(PAGE_FINISH == this->nextId()) {

        this->button(QWizard::NextButton)->setEnabled(false);
        this->button(QWizard::BackButton)->setEnabled(false);
#ifdef MPOS_CLOUD
        this->setting();
#else
        if(_installpage->isSuccess() == false) {

            this->install();
            return false;
        }
#endif
    }

    return true;
}

void InstallWizard::callFinished(int code, QProcess::ExitStatus es) {
    this->button(QWizard::BackButton)->setEnabled(true);
    if(0 != code) {
        _installpage->setInstallStatus(false);
        QMessageBox::warning(nullptr, tr("出错"), tr("同步资源失败，请稍候再试"));

        this->button(QWizard::NextButton)->setEnabled(true);
        return;
    }
    qDebug() << "script call code: " << code << ", exitstatus: " << es;
    _installlog->append(tr("安装结束"));

    _installpage->setInstallStatus(true);

    this->next();
}

bool InstallWizard::setting() {
    //写设置文件
    QFile conf( QString("%1/.installed").arg(WORKDIR));

    conf.open(QIODevice::WriteOnly);
    if(conf.isOpen() == false) {
        QMessageBox::warning(nullptr, tr("出错"), QString("安装路径没权限: %1").arg(WORKDIR));
        return false;
    }
    conf.write( QString("shop-id=%1\n").arg( _rid->text().toInt()).toLocal8Bit());

    QString server = "";
    if( _isCnServer->isChecked()) {
        server += "cn";
    } // default us server

    server += "shop.aiwaiter.net";

    conf.write( QString("local-web-server=https://%1\n").arg(server).toLocal8Bit());
    conf.write( QString("redis-server=%1\n").arg(server).toLocal8Bit());
    conf.write( QString("redis-port=%1\n").arg(6379).toLocal8Bit());
    conf.write( QString("redis-auth=********\n").toLocal8Bit());
    conf.close();

    return true;
}

bool InstallWizard::install() {
    QString acc = _account->text();
    QString pwd = _passwd->text();


    QProcess::execute(QString("sed -i \"s/^area='.*'$/area='%1'/g\" /opt/pos/init-local-server-data.sh\n")
                    .arg("SCN").toStdString().c_str());

     QProcess::execute(QString("sed -i \"s/^user=.*$/user=%1/g\" /opt/pos/init-local-server-data.sh\n")
                    .arg(acc).toStdString().c_str());

     QProcess::execute(QString("sed -i \"s/^pwd=.*$/pwd=%1/g\" /opt/pos/init-local-server-data.sh\n")
                    .arg(pwd).toStdString().c_str());

    QString _cmd = "/bin/bash /opt/pos/init-local-server-data.sh";

    _process->start(QString("%1\n").arg(_cmd).toStdString().c_str());
    if(false == _process->waitForStarted()) {
        qDebug() << "install script is not started";
        _installlog->append( tr("安装脚步启动失败"));
        return false;
    }

    return true;
}
