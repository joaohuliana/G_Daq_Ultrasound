#ifndef CHOOSEPROBE_H
#define CHOOSEPROBE_H

#include <QDialog>
//#include <QImage>
#include "ui_chooseprobe.h"

class ChooseProbe : public QDialog, public Ui::ChooseProbe
{
	Q_OBJECT

public:
	ChooseProbe(QWidget *parent = 0);
	~ChooseProbe();

	void loadImage(QString str);

};

#endif // CHOOSEPROBE_H
