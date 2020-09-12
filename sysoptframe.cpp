#include "sysoptframe.h"
#include <QDebug>

#include "datathread.h"

SysOptFrame::SysOptFrame(QWidget *parent) :
    QFrame(parent)
{

    this->setStyleSheet("background-color: #ffffff");

    _layoutLeft = new GridLayoutEx(10, this);
    _layoutRight = new GridLayoutEx(4, this);


    _process = new QProcess(this);

    connect(_process, SIGNAL(readyReadStandardOutput()),
            this, SLOT(readOutput()) );

    connect(_process, SIGNAL(readyReadStandardError()),
            this, SLOT(readOutput()) );

    this->connect(_process, SIGNAL(finished(int, QProcess::ExitStatus)),
                  this, SLOT(callFinished(int, QProcess::ExitStatus)));

    init();
}

SysOptFrame::~SysOptFrame()
{
}

//读取crontab配置文件中的小时，分钟参数
int readCrontabTime(QString feature, int offset) {

//    QProcess::execute("crontab -r");
    //通过控制台输出获取数据 redis-cli
    QProcess *proc = new QProcess();
    QString cmd  = QString("bash /opt/sbin/cront.sh %1 %2")
                        .arg(feature)
                        .arg(offset);
    proc->start(cmd, QIODevice::ReadOnly);
    // Wait for it to start
    proc->waitForFinished(5 * 1000);
    // Continue reading the data until EOF reached
    QString out = proc->readAllStandardOutput();
    QString err = proc->readAllStandardError();
    qDebug() << "crontab out: " << out;
    qDebug() << "crontab err: " << err;

//    QProcess::execute("crontab -u mpos /opt/sbin/mpos.cron");

    if(err.trimmed().length() > 0) {
        return 9;
    }
    return out.trimmed().toInt();
}
void SysOptFrame::init() {
    //10列
    {
        //更新资源
        // 手动更新
        // 自动更新
       {
            POSLabelEx* lab = new POSLabelEx(tr("自动更新"), _layoutLeft);
            _layoutLeft->addWidget(lab, 2);

            //设置时间
            POSLabelEx* title = new POSLabelEx(tr("每天"), _layoutLeft);
            _layoutLeft->addWidget(title);

            LineEditEx* hour = new LineEditEx(_layoutLeft, true);
            hour->setText("9");
            hour->setObjectName("UpdateHour");
            _layoutLeft->addWidget(hour);
            _layoutLeft->addWidget(new QLabel(tr("时"), _layoutLeft), 1);

            LineEditEx* min = new LineEditEx(_layoutLeft, true);
            min->setText("00");
            min->setObjectName("UpdateMin");
            _layoutLeft->addWidget(min);
            _layoutLeft->addWidget(new QLabel(tr("分"), _layoutLeft), 1);


            POSButtonEx* submit = new POSButtonEx(tr("保存"), _layoutLeft);
            connect(submit, SIGNAL(released()), this, SLOT(onSaveUpdateHourClicked()));
            _layoutLeft->addWidget(submit);

            _layoutLeft->addWidget(new QLabel(_layoutLeft), 2);
        } {
            POSLabelEx* lab = new POSLabelEx(tr("手动更新"), _layoutLeft);

            _layoutLeft->addWidget(lab, 2);

            POSButtonEx* submit = new POSButtonEx(tr("执行"), _layoutLeft);
            connect(submit, SIGNAL(released()), this, SLOT(onUpdateClicked()));

            _layoutLeft->addWidget(submit);

            _layoutLeft->addWidget(new QLabel(_layoutLeft), 7);
        }

    } {
        _layoutLeft->addWidget(new QLabel(_layoutLeft), 10);
    } {
        //备份订单
        // 手动备份
        // 自动备份
        {
            POSLabelEx* lab = new POSLabelEx(tr("自动备份"), _layoutLeft);
            _layoutLeft->addWidget(lab, 2);
            //设置时间
            POSLabelEx* title = new POSLabelEx(tr("每天"), _layoutLeft);
            _layoutLeft->addWidget(title);

            LineEditEx* hour = new LineEditEx(_layoutLeft, true);
            hour->setText("09");
            hour->setObjectName("BackupHour");
            _layoutLeft->addWidget(hour);

            _layoutLeft->addWidget(new QLabel(tr("时"), _layoutLeft), 1);

            LineEditEx* min = new LineEditEx(_layoutLeft, true);
            min->setText("00");
            min->setObjectName("BackupMin");
            _layoutLeft->addWidget(min);
            _layoutLeft->addWidget(new QLabel(tr("分"), _layoutLeft), 1);

            POSButtonEx* submit = new POSButtonEx(tr("保存"), _layoutLeft);
            connect(submit, SIGNAL(released()), this, SLOT(onSaveBackupHourClicked()));
            _layoutLeft->addWidget(submit);

            _layoutLeft->addWidget(new QLabel(_layoutLeft), 2);
        } {
            POSLabelEx* lab = new POSLabelEx(tr("手动备份"), _layoutLeft);
            _layoutLeft->addWidget(lab, 2);

            POSButtonEx* submit = new POSButtonEx(tr("执行"), _layoutLeft);
            connect(submit, SIGNAL(released()), this, SLOT(onBackupClicked()));
            _layoutLeft->addWidget(submit);

            _layoutLeft->addWidget(new QLabel(_layoutLeft), 7);
        }
    } {
        _layoutLeft->addWidget(new QLabel(_layoutLeft), 10);
        _layoutLeft->addWidget(new QLabel(tr("执行日志:"), _layoutLeft), 10);
    } {
        //手动操作日志
        _log = new QTextEdit(_layoutLeft);
        _layoutLeft->addWidget(_log, 10, 12);
    }

    //right 4 列
    {
         //触摸屏设置
        {

            _layoutRight->addWidget(new QLabel(_layoutRight), 4);
        }

        _layoutRight->addWidget(new QLabel(_layoutRight), 1);

         POSButtonEx* submit = new POSButtonEx(tr("触摸屏校正"), _layoutRight);
         connect(submit, SIGNAL(released()), this, SLOT(onTouchCheckClicked()));
         _layoutRight->addWidget(submit, 2);

         _layoutRight->addWidget(new QLabel(_layoutRight), 1);
    }
}

void SysOptFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);

    QRect rect = this->geometry();

    int h = (rect.height() - 10);
    _layoutLeft->setRowHeight( h / 20 );
    _layoutLeft->setGeometry(0, 0, rect.width() * 0.7, h);

    _layoutRight->setRowHeight( rect.height() / 20 );
    _layoutRight->setGeometry(rect.width() * 0.7, 0, rect.width() * 0.3, rect.height());

//    _log->setMinimumHeight( 300);
}

void SysOptFrame::showEvent(QShowEvent *event) {
    QFrame::showEvent(event);

    _log->clear();

    int backupHour = readCrontabTime("upload-orders-to-server", 2);
    int backupMin = readCrontabTime("upload-orders-to-server", 1);

    LineEditEx* bh = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("BackupHour"));
    bh->setText(QString("%1").arg(backupHour, 2, 10));

    LineEditEx* bm = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("BackupMin"));
    bm->setText(QString("%1").arg(backupMin, 2, 10));

    int updateHour = readCrontabTime("update-local-server", 2);
    int updateMin = readCrontabTime("update-local-server", 1);

    LineEditEx* uh = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("UpdateHour"));
    uh->setText(QString("%1").arg(updateHour, 2, 10));

    LineEditEx* um = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("UpdateMin"));
    um->setText(QString("%1").arg(updateMin, 2, 10));
}


void SysOptFrame::readOutput() {
    QString str = "";

    str += _process->readAllStandardOutput();

    QString err = _process->readAllStandardError();
    if( err.length() > 0) {
        str += "\nError: ";
        str += err;
    }

    qDebug() << "output: " << str;
    _log->append(str.trimmed());
}


void SysOptFrame::callFinished(int code, QProcess::ExitStatus es) {

}

void SysOptFrame::onTouchCheckClicked() {
    QString out, err;
    DataThread::CallScript("/usr/local/eGTouch64withX/eGTouchU", out, err, 2 * 60);
}

void SysOptFrame::onUpdateClicked() {
    _log->clear();

    QString cmd = "bash /opt/pos/update-local-server-resource.sh";
    _process->start(QString("%1\n").arg(cmd).toStdString().c_str());
    if(false == _process->waitForStarted()) {
        qDebug() << "install scripupdate-local-server-resource.sht is not started";
        _log->append( tr("更新脚步启动失败"));
        return ;
    }

}

void SysOptFrame::onBackupClicked() {
    _log->clear();

    QString cmd = "bash /opt/pos/upload-orders-to-server.sh";
    _process->start(QString("%1\n").arg(cmd).toStdString().c_str());
    if(false == _process->waitForStarted()) {
        qDebug() << "install scripupdate-local-server-resource.sht is not started";
        _log->append( tr("更新脚步启动失败"));
        return ;
    }

}

void SysOptFrame::onSaveBackupHourClicked() {
    //更新 crontab的小时时间
    _log->clear();

    LineEditEx* bh = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("BackupHour"));
    int hour = bh->text().toInt();

    LineEditEx* bm = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("BackupMin"));
    int min = bm->text().toInt();

    QString cmd = QString("sed -i \"s#[0-9]* [0-9]* \\* \\* \\* bash /opt/pos/upload-orders#%1 %2 \\* \\* \\* bash /opt/pos/upload-orders#g\" /opt/sbin/mpos.cron")
            .arg(min)
            .arg(hour);
    _log->append(tr("定时配置") + QString("%1时%2分").arg(hour, 2).arg(min, 2));

    QProcess::execute(cmd);

    _log->append(tr("更新定时任务配置文件完成"));

    QProcess::execute("crontab -u mpos /opt/sbin/mpos.cron");

    _log->append(tr("重置定时任务完成"));
}


void SysOptFrame::onSaveUpdateHourClicked() {
    //更新 crontab的小时时间
    _log->clear();

    LineEditEx* uh = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("UpdateHour"));
    int hour = uh->text().toInt();
    LineEditEx* um = dynamic_cast<LineEditEx*>(_layoutLeft->getItembyObjectName("UpdateMin"));
    int min = um->text().toInt();

    QString cmd = QString("sed -i \"s#[0-9]* [0-9]* \\* \\* \\* bash /opt/pos/update-local#%1 %2 \\* \\* \\* bash /opt/pos/update-local#g\" /opt/sbin/mpos.cron")
            .arg(min)
            .arg(hour);

    _log->append(tr("定时配置") + QString("%1时%2分").arg(hour, 2).arg(min, 2));


    QProcess::execute(cmd);

    _log->append(tr("更新定时任务配置文件完成"));

    QProcess::execute("crontab -u mpos /opt/sbin/mpos.cron");

    _log->append(tr("重置定时任务完成"));

}

