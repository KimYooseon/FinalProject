#ifndef CHOOSEPATIENTMANAGER_H
#define CHOOSEPATIENTMANAGER_H
#include <QWidget>
#include <QTreeWidget>

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

private slots:
    void sendedTempData(QString, QString);

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

signals:
    void choosePatientSignal(QString);
};

#endif // CHOOSEPATIENTMANAGER_H
