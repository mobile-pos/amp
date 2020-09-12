#include "printersframe.h"
#include <QPrintDialog>
#include <QPrinterInfo>
#include <QGridLayout>
#include <QProcess>
#include <QMessageBox>

#include "datathread.h"

PrintersFrame::PrintersFrame(QWidget *parent) :
    QFrame(parent)
{
    _layout = new GridLayoutEx(2, this);
    init();
}

PrintersFrame::~PrintersFrame()
{
}


void PrintersFrame::init() {

    _layout->clean();
    {
        //设置打印机
        QPushButton* btn = new QPushButton(tr("新增打印机"), _layout);

        this->connect(btn, SIGNAL(released()), this, SLOT(printControl()));


        QPushButton* modify = new QPushButton(tr("修改打印机"), _layout);

        this->connect(btn, SIGNAL(released()), this, SLOT(printControl()));

        this->connect(modify, SIGNAL(released()), this, SLOT(modifyPrinter()));

        _layout->addWidget(btn);
        _layout->addWidget(modify);
    }

    {
        QLabel* lab = new QLabel(tr("前台打印机"),  _layout);

        this->_cashierPrinter = new QComboBox( _layout);


        QStringList list = QPrinterInfo::availablePrinterNames();
        foreach(QString pname, list) {

            _cashierPrinter->addItem(pname);
        }

        _layout->addWidget(lab);
        _layout->addWidget(_cashierPrinter);
    }

    {
        QStringList list = QPrinterInfo::availablePrinterNames();
        for(int i = 1; i < list.length(); i++) {
            //只有多个打印机才配置后厨打印机

            QLabel* lab = new QLabel(tr("后台厨房打印机"), _layout);

            QComboBox* printers = new QComboBox(_layout);

            this->_cookingPrinters.append(printers);

            printers->addItem("");

            foreach(QString pname, list) {
                printers->addItem(pname);
            }

            _layout->addWidget(lab);
            _layout->addWidget(printers);
        }
    }
    {
        _layout->addWidget(new QLabel(_layout), 2);
    }
    {
        QPushButton* submit = new QPushButton(tr("确定"), _layout);
        this->connect(submit, SIGNAL(released()), this, SLOT(onSubmitClick()));

        QPushButton* cancel = new QPushButton(tr("取消"), _layout);
        this->connect(cancel, SIGNAL(released()), this, SLOT(onCancelClick()));

        _layout->addWidget(submit);
        _layout->addWidget(cancel);
    }

    _layout->resetPages();
}

void PrintersFrame::showEvent(QShowEvent *e) {
    QFrame::showEvent(e);

    QString cprinter = DataThread::inst().getCashierPrinter();
    _cashierPrinter->setCurrentText(cprinter);

    QList<QString> kprinters = DataThread::inst().getKitchPrinter();
    int i = 0;
    foreach(QString kn, kprinters) {
        if(this->_cookingPrinters.length() <= i) {
            return;
        }

        this->_cookingPrinters[i]->setCurrentText(kn);
        i++;
    }
}
void PrintersFrame::onSubmitClick() {

    //修改打印机配置
    QStringList list = QPrinterInfo::availablePrinterNames();
    foreach(QString pn, list) {
        QProcess::execute(QString("redis-cli hset printers:config %1 idle").arg(pn));
    }

    QString cashier = _cashierPrinter->currentText();
    if(cashier.length() == 0) {
        QMessageBox::warning(nullptr, tr("出错"), tr("前台打印机不能为空"));
        return;
    }
    QProcess::execute(QString("redis-cli hset printers:config %1 cashier")
                      .arg(cashier));

    int id = 0;
    foreach(QComboBox* cb, this->_cookingPrinters) {
        id += 1;
        if(cb->currentText().length() == 0) {
            continue;
        }
        QProcess::execute(QString("redis-cli hset printers:config %1 kitchen-%2")
                          .arg(cb->currentText())
                          .arg(id));

    }

    int ret = DataThread::inst().updatePrinter2Cloud();
    if( 0 == ret) {
        QMessageBox::information(nullptr, tr("提示"), tr("打印机更新成功"));
    } else {
        QMessageBox::warning(nullptr, tr("出错"), tr("打印机更新失败: ") + QString::number(ret));
    }
}
void PrintersFrame::onCancelClick() {
    init();
}
void PrintersFrame::printControl() {

//    QProcess::execute("system-config-printer &");
    QString out, err;
    if(-1 == DataThread::CallScript("system-config-printer", out, err, 5 * 60)) {
        QMessageBox::warning(nullptr, "错误", "Printer 服务启动失败: " + err);
        return;
    }

    init();
}

void PrintersFrame::modifyPrinter() {
    QPrintDialog dlg;
    dlg.exec();
}
void PrintersFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    this->_layout->setGeometry(this->geometry());
}
