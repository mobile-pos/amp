#include "gratuityframe.h"
#include <QLabel>
#include <QDebug>
#include <QMessageBox>

#include "datathread.h"

GratuityItem::GratuityItem() {
    _creditName = "";
    _tips = "0.0";
    _amount = 0.0;
}

class GratuityModel : public QAbstractTableModel {
   QList<GratuityItem> m_data;

signals:
   void GoodsChanged(const QList<GratuityItem>&);

public:
   GratuityModel(QObject * parent = {}) : QAbstractTableModel{parent} {}

   int rowCount(const QModelIndex &) const override { return m_data.count(); }
   int columnCount(const QModelIndex &) const override { return 4; }

   QVariant data(const QModelIndex &index, int role) const override {
      if (role != Qt::DisplayRole && role != Qt::EditRole) return {};
      const auto & item = m_data[index.row()];
      switch (index.column()) {
      case 0: return index.row();
      case 1: return item._creditName;
      case 2: return item._tips;
      case 3: return item._amount;
      default: return {};
      };
   }

   QVariant headerData(int section, Qt::Orientation orientation, int role) const override {
      if (orientation != Qt::Horizontal || role != Qt::DisplayRole) return {};
      switch (section) {
      case 0: return "#";
      case 1: return "Card Type";
      case 2: return "Gratuity";
      case 3: return "Amount";
      default: return {};
      }

   }

   void append(const GratuityItem & credit) {
      beginInsertRows({}, m_data.count(), m_data.count());
      m_data.append(credit);
      endInsertRows();
   }

   void clean() {
       beginResetModel();
       m_data.erase(m_data.begin(), m_data.end());
       endResetModel();
   }

   QList<GratuityItem>& credits() {
       return m_data;
   }

   GratuityItem& credit(const QModelIndex& index) {
       return m_data[index.row()];
   }

   GratuityItem& credits(const QModelIndex& index) {
       return m_data[index.row()];
   }

   void remove(const QModelIndex& index) {
       if( m_data.length() > index.row() ) {
            m_data.removeAt(index.row());
       }
   }
};



////////////////////////////////////////////////////
GratuityFrame::GratuityFrame(QWidget *parent) :
    QFrame(parent),
    _ob(nullptr)
{
    this->setStyleSheet("background-color: rgb(199, 199, 171)");

    _layout = new GridLayoutEx(10, this);
    _layout->setObjectName("小费管理面板");

    _banks = new GridLayoutEx(2, _layout);
    _banks->setStyleSheet("background-color: rgb(221, 221, 168)");

    _vkb = new VKeyboardEx(_layout);
    _vkb->init(2);

    this->connect(_vkb, SIGNAL(onVKeyDown(const QString&)), this, SLOT(onVKeyDown(const QString&)));

    _tipsTb = new QTableView(_layout);

    _tipsModel = new GratuityModel();
    _tipsTb->setModel(_tipsModel);

    _tipsTb->setSelectionMode(QAbstractItemView::SingleSelection);
    _tipsTb->setEditTriggers(QAbstractItemView::NoEditTriggers);
    _tipsTb->setSelectionBehavior(QAbstractItemView::SelectRows);

    this->connect(_tipsTb, SIGNAL(clicked(const QModelIndex & )), this, SLOT(rowSelected(const QModelIndex &)));
    init();
}

GratuityFrame::~GratuityFrame()
{

}

void GratuityFrame::init() {
    qDebug() << "GratuityFrame::init";
    {
        //银行卡类型按钮
        {
            VKeyItemButton* kb = new VKeyItemButton("VISA", this, _banks);
            kb->setText("Visa");
            _banks->addWidget(kb);
        }
        {
            QLabel* kb = new QLabel("", _banks);
            _banks->addWidget(kb);
        }
        {
            VKeyItemButton* kb = new VKeyItemButton("MASTER", this, _banks);
            kb->setText("Master");
            _banks->addWidget(kb);
        }
        {
            QLabel* kb = new QLabel("", _banks);
            _banks->addWidget(kb);
        }

        {
            VKeyItemButton* kb = new VKeyItemButton("AMEX", this, _banks);
            kb->setText("AMEX");
            _banks->addWidget(kb);
        }
        {
            VKeyItemButton* kb = new VKeyItemButton("GIFT", this, _banks);
            kb->setText("Gift");
            _banks->addWidget(kb);
        }
        {
            VKeyItemButton* kb = new VKeyItemButton("Cash", this, _banks);
            kb->setText(tr("Cash"));
            _banks->addWidget(kb);
        }
    }
    //一行10个单元
    {
        _layout->addWidget(_tipsTb, 10, 3);
    } {
        {
            //选择是输入小费，还是输入总金额
            QCheckBox* cb = new QCheckBox(tr("总额"), _layout);
            cb->setChecked(true);
            cb->setObjectName("isAmountorTips");
            _layout->addWidget(cb, 1);
        }

        {
            //总计小费概述
            {
                QLabel* l = new QLabel("", _layout);
                _layout->addWidget(l, 1, 1);
            } {
                QLabel* l = new POSLabelEx(QObject::tr("小费总额："), _layout);
                l->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                _layout->addWidget(l, 2, 1);

                QLineEdit* _edit = new QLineEdit("0.0", _layout);
                _edit->setReadOnly(true);
                _layout->addWidget(_edit, 2, 1, 10000);
            }  {
                QLabel* l = new POSLabelEx(QObject::tr("实收总额："), _layout);
                l->setAlignment(Qt::AlignRight | Qt::AlignCenter);
                _layout->addWidget(l, 2, 1);

                QLineEdit* _edit = new QLineEdit("0.0", _layout);
                _edit->setReadOnly(true);
                _layout->addWidget(_edit, 2, 1, 10001);
            }
        }
    } {
        _layout->addWidget(_banks, 3, 5);
    } {
        _layout->addWidget(_vkb, 5, 5);
    } {

        {
            VKeyItemButton* kb = new VKeyItemButton("OK", this, _layout);
            kb->setText(QObject::tr("确定"));
            kb->setStyleSheet("background-color: rgb(120, 88, 120)");
            _layout->addWidget(kb, 2);
        }
        {
            VKeyItemButton* kb = new VKeyItemButton("Cancel", this, _layout);
            kb->setText(QObject::tr("放弃"));
            kb->setStyleSheet("background-color: rgb(120, 88, 40)");
            _layout->addWidget(kb, 2);
        }
    }

    qDebug() << "GratuityFrame::init end";
}
void GratuityFrame::resizeEvent(QResizeEvent* e) {
    QFrame::resizeEvent(e);


    QRect frame = this->frameGeometry();

    _layout->setRowHeight( frame.height() / 9);
    _layout->setGeometry(0, 0, frame.width(), frame.height());


    _banks->setRowHeight( 40 );
}

void GratuityFrame::showEvent(QShowEvent* e) {
    QFrame::showEvent(e);

}

void GratuityFrame::updateOrder(OrderBean* ob) {
    this->_ob = ob;
    _tipsModel->clean();

    QLineEdit* _edtips = dynamic_cast<QLineEdit*>( _layout->getItembyID(10000));
    _edtips->setText( QString::number( 0.0));

    QLineEdit* _edamount = dynamic_cast<QLineEdit*>( _layout->getItembyID(10001));
    _edamount->setText( QString::number( 0.0));

    //重新拉取分单的订单列表
    QList<QVariant> soids = DataThread::inst().loadSubOrders(this->_ob->oid);
    this->_ob->subOids.clear();
    foreach(QVariant soid, soids) {
        this->_ob->subOids.push_back( soid.toInt());
    }


    //自动计算没有分单的小费
    if( this->_ob->subOids.length() == 0 && _ob->payType == POSDATA::CREDIT ) {
        this->onKeyDown(_ob->creditType);
    } else if(this->_ob->subOids.length() == 0 && _ob->payType == POSDATA::CASH) {
        this->onKeyDown("Cash");
    }
}

void GratuityFrame::rowSelected(const QModelIndex &) {

}

void GratuityFrame::updateSubOrdersGratuity() {

    QList<GratuityItem> credits = _tipsModel->credits();

    int i = 0;

    double totalamount = 0.0;
    foreach(GratuityItem gi, credits) {

        int soid = this->_ob->oid;

        double amount = 0;
        if(this->_ob->subOids.length() > 0 ) {
            //有分单
            if(i > this->_ob->subOids.length()) {
                QMessageBox::warning(this, "提示", "小费数量大于分单的数量");
                break;
            }

            soid = this->_ob->subOids[i++];

            OrderBean ob( *this->_ob);
            ob.isSubOrder = true;
            ob.parentOid = this->_ob->oid;
            ob.gratuity = gi._tips.toDouble();
            ob.amount = gi._amount;
            ob.payType = 1;
            ob.creditType = gi._creditName;

            ob.oid = soid;
            int ret = DataThread::inst().updateOrderInfo(ob.toMap());
            if(ret != 0) {
                QMessageBox::warning(this, "出错", "保存小费时出错");
            }

            amount += gi._amount;
        }

        this->_ob->gratuity += gi._tips.toDouble();

        totalamount += gi._amount;
    }

    if(totalamount > 0) {
        this->_ob->amount = totalamount;
    }

    //修改主订单的小费和应会费用
    int ret = DataThread::inst().updateOrderInfo(this->_ob->toMap());
    if(ret != 0) {
        QMessageBox::warning(this, "出错", "保存小费时出错");
    }
}
void GratuityFrame::onKeyDown(const QString& value) {
    qDebug() << "onKeyDown called " << value;

    if("OK" == value) {
        //总金额不能小于应付金额

        QLineEdit* _edamount = dynamic_cast<QLineEdit*>( _layout->getItembyID(10001));

        if(_edamount->text().toDouble() < this->_ob->amount) {
            QMessageBox::warning(this, "出错", "实收金额不能小于应付金额");
            return;
        }

        //完成小费录入
        updateSubOrdersGratuity();
        this->hide();

        emit(onGratuityDone(_tipsModel->credits()));
        return;

    } else if("Cancel" == value) {
        //放弃小费录入
        this->hide();
        return;

    }

    if(this->_ob->subOids.length() > 0) {
        if(_tipsModel->rowCount(QModelIndex()) >= this->_ob->subOids.length() ) {
            QMessageBox::warning(this, "提示", "小费数量大于分单的数量");
            return;
        }
    }else if(_tipsModel->rowCount(QModelIndex()) > 1){
        QMessageBox::warning(this, "提示", "未分单，小费不能多份");
        return;
    }
    //其它的键操作
    GratuityItem gi;
    gi._creditName = value;

    _tipsModel->append(gi);

    int rows = _tipsModel->rowCount(QModelIndex());
    if( rows == 0) {
        return;
    }
    _tipsTb->selectRow( rows - 1);

}
void GratuityFrame::onVKeyDown(const QString& kvalue) {
    qDebug() << "onVKeyDown called " << kvalue;

    QString v = "0";
    if( (kvalue >= "0" && kvalue <= "9") || kvalue == ".") {
        v = kvalue;
    } else {
        v = "";
    }

    //判断是输入总额，还是输入小费
    QCheckBox* cb = dynamic_cast<QCheckBox*>(_layout->getItembyObjectName("isAmountorTips"));
    bool isAmount = cb->isChecked();


    QItemSelectionModel *selections = _tipsTb->selectionModel();
    QModelIndexList selected = selections->selectedRows();

    //如果是拆单的，要平均计算每个amount
    double amount = this->_ob->amount;
    if( this->_ob->subOids.length() > 0 ) {
        amount = 1.0 * amount / this->_ob->subOids.length();
    }

    foreach(QModelIndex index, selected)
    {
        if( index.row() < 0) {
            continue;
        }
        GratuityItem& gi = _tipsModel->credit(index);

        QString input = QString::number(gi._amount);
        if(isAmount == false) {
            input = gi._tips;
        }

        if( kvalue == "Del" && input.length()  > 0) {
            input = input.mid(0, input.length() - 1);

        } else if( kvalue == "Clean" ) {
            input = "0.0";

        } else if( input == "0.0" ){
            input = QString::number(input.toDouble() + v.toDouble());

        } else {
            input = input + v;
        }

        if(isAmount == true) {
            gi._amount = input.toDouble();
            gi._tips = QString::number(gi._amount - amount);
        } else {
            //同时计算每个小单的总费用
            gi._tips = input;
            gi._amount = gi._tips.toDouble() + amount;
        }
    }

    //
    QList<GratuityItem> credits = _tipsModel->credits();
    double totaltips = 0.0;
    double totalamount = 0.0;
    foreach(GratuityItem gi, credits) {
        totaltips += gi._tips.toDouble();
        totalamount += gi._amount;
    }

    //更新小费的总费用
    QLineEdit* _edtips = dynamic_cast<QLineEdit*>( _layout->getItembyID(10000));
    _edtips->setText( QString::number( totaltips));

    QLineEdit* _edamount = dynamic_cast<QLineEdit*>( _layout->getItembyID(10001));
    _edamount->setText( QString::number( totalamount));



    _tipsModel->layoutChanged();
}
