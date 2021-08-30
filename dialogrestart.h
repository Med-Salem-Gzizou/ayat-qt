#ifndef DIALOGRESTART_H
#define DIALOGRESTART_H

#include <QDialog>

namespace Ui {
class DialogRestart;
}

class DialogRestart : public QDialog
{
    Q_OBJECT

public:
    explicit DialogRestart(QWidget *parent = nullptr);
    ~DialogRestart();

private:
    Ui::DialogRestart *ui;
};

#endif // DIALOGRESTART_H
