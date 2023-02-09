#ifndef CHOOSEPATIENTMANAGER_H
#define CHOOSEPATIENTMANAGER_H

#include <QWidget>

namespace Ui {
class ChoosePatientManager;
}

class ChoosePatientManager : public QWidget
{
    Q_OBJECT

public:
    explicit ChoosePatientManager(QWidget *parent = nullptr);
    ~ChoosePatientManager();

private:
    Ui::ChoosePatientManager *ui;
};

#endif // CHOOSEPATIENTMANAGER_H
