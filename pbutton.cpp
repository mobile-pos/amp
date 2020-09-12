#include "pbutton.h"

PButton::PButton(QPushButton *parent) : QPushButton(parent)
{
    QRect rect = this->geometry();
    rect.setHeight( 80 );
}
