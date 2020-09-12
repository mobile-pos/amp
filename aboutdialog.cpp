#include "aboutdialog.h"
#include "ui_aboutdialog.h"

#include "version.h"

AboutDialog::AboutDialog(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::AboutDialog)
{
    this->setWindowTitle(tr("关于"));

    ui->setupUi(this);

    ui->_version->setText( QString("%1.%2").arg(AMPVersion).arg(AMPBuild));

    ui->_compName->setText(tr("智小二科技"));

    ui->_contact->appendPlainText(tr("研发总部：中国深圳 \n\t电话：15013420092;"));
    ui->_contact->appendPlainText(tr("客服中心：中国杭州 \n\t电话：15013420092;"));
    ui->_contact->appendPlainText("");
    ui->_contact->appendPlainText(tr("当地经销商：美国纽约  \n\t电话：15013420092;"));
}

AboutDialog::~AboutDialog()
{
    delete ui;
}

void AboutDialog::on_OK_released()
{
    this->done(0);
}
