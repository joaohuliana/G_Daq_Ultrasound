#include "txcpwc.h"

TxCPWC::TxCPWC(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	//loadImage("sp.jpg");
}

TxCPWC::~TxCPWC()
{

}

void TxCPWC::loadImage(QString str) 
{
	//QPixmap pm("sp.jpg");
	//this->lb_img->setPixmap(pm);
	//this->lb_img->show();
}