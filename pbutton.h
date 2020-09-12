#ifndef PBUTTON_H
#define PBUTTON_H

#include <QPushButton>

class PButton : public QPushButton
{
    Q_OBJECT
public:
    explicit PButton(QPushButton *parent = nullptr);

signals:

public slots:
};

#endif // PBUTTON_H
