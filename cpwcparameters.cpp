#include "cpwcparameters.h"

cpwcParameters::cpwcParameters(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	//loadImage("sp.jpg");
}

cpwcParameters::~cpwcParameters()
{

}

void cpwcParameters::loadImage(QString str) 
{
	//QPixmap pm("sp.jpg");
	//this->lb_img->setPixmap(pm);
	//this->lb_img->show();
}