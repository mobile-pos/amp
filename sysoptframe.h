#ifndef SYSOPTFRAME_H
#define SYSOPTFRAME_H

#include <QFrame>
#include <QTextEdit>
#include <QProcess>

#include "gridlayoutex.h"
#include "vkeyboardex.h"

class SysOptFrame : public QFrame
{
    Q_OBJECT

public:
    explicit SysOptFrame(QWidget *parent = nullptr);
    ~SysOptFrame() override;

public:
    void init();

public slots:
    void readOutput();

    void callFinished(int code, QProcess::ExitStatus es);

    void onUpdateClicked();
    void onBackupClicked();
    void onTouchCheckClicked();

    void onSaveBackupHourClicked();
    void onSaveUpdateHourClicked();
public:
    void resizeEvent(QResizeEvent*) override;
    void showEvent(QShowEvent *event) override;
private:
    QTextEdit* _log;

    GridLayoutEx* _layoutLeft;
    GridLayoutEx* _layoutRight;

    QProcess* _process;
};

#endif // SYSOPTFRAME_H
