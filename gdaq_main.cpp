#include "gdaq_main.h"
#include "ui_gdaq_main.h"

gdaq_main::gdaq_main(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::gdaq_main)
{
    ui->setupUi(this);
}

gdaq_main::~gdaq_main()
{
    delete ui;
}
