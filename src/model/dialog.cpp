#include <indedds-monitor/model/dialog.h>
#include "ui_dialog.h"

Dialog::Dialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Dialog)
{
    ui->setupUi(this);
}

void Dialog::openDialog() {
    if (!isVisible()) {
        this->exec();
    }
}



Dialog::~Dialog()
{
    delete ui;
}
