#ifndef APPORDERSFRAME_H
#define APPORDERSFRAME_H

#include <QFrame>
#include <QThread>
#include "gratuityframe.h"

class AppOrdersModel;

class AppOrdersFrame;
class AppOrdersDataThread : public QThread
{
public:
    AppOrdersDataThread(AppOrdersFrame* appFrm) ; //: _appFrm(appFrm) {_isRunning = false;}
    ~AppOrdersDataThread() override;
public:
    void run() override;
    inline void stop() {
        _isRunning = false;
        QThread::wait();
    }
private:
    AppOrdersFrame* _appFrm;
    bool _isRunning;
};

class AppOrdersFrame : public QFrame
{
    Q_OBJECT

public:
    explicit AppOrdersFrame(QWidget *parent = nullptr);
    ~AppOrdersFrame() override;

public:
    void resizeEvent(QResizeEvent* e) override;
    void showEvent(QShowEvent* e) override;

public:
    void updateOrders();

    inline AppOrdersModel* getModel() {
        return _model;
    }

    void retranslateUI();
private:
    void init();

public slots:
    void onClicked(const QModelIndex &index);

    void rowSelected(const QModelIndex &index);

private:
    AppOrdersModel* _model;

    AppOrdersDataThread* _dataThread;
    QTableView* _odTab;

};

#endif // APPORDERSFRAME_H
