#include "txparameters.h"

TxParameters::TxParameters(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	//loadImage("sp.jpg");
}

TxParameters::~TxParameters()
{

}

void TxParameters::loadImage(QString str) 
{
	//QPixmap pm("sp.jpg");
	//this->lb_img->setPixmap(pm);
	//this->lb_img->show();
}