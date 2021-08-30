#include "dialogrestart.h"
#include "ui_dialogrestart.h"

DialogRestart::DialogRestart(QWidget *parent) :
    QDialog(parent),
    ui(new Ui::DialogRestart)
{
    ui->setupUi(this);
    this->setFixedSize(this->width(), this->height());
}

DialogRestart::~DialogRestart()
{
    delete ui;
}
