#include "chooseprobe.h"

ChooseProbe::ChooseProbe(QWidget *parent)
	: QDialog(parent)
{
	setupUi(this);
	//loadImage("sp.jpg");
}

ChooseProbe::~ChooseProbe()
{

}

void ChooseProbe::loadImage(QString str) 
{
	//QPixmap pm("sp.jpg");
	//this->lb_img->setPixmap(pm);
	//this->lb_img->show();
}

