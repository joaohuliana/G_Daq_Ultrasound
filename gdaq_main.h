#ifndef GDAQ_MAIN_H
#define GDAQ_MAIN_H

#include <QMainWindow>

namespace Ui {
class gdaq_main;
}

class gdaq_main : public QMainWindow
{
    Q_OBJECT

public:
    explicit gdaq_main(QWidget *parent = nullptr);
    ~gdaq_main();

private:
    Ui::gdaq_main *ui;
};

#endif // GDAQ_MAIN_H
