#include "choosepatientmanager.h"
#include "ui_choosepatientmanager.h"


ChoosePatientManager::ChoosePatientManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChoosePatientManager)
{
    ui->setupUi(this);
}

ChoosePatientManager::~ChoosePatientManager()
{
    delete ui;
}

void ChoosePatientManager::sendedTempData(QString count, QString data)
{
    ui->treeWidget->clear();

    //환자 선택 창 띄움. 그 창에서 특정 treeWidget 클릭하면 PSE로 해당 PID 보내줘서 다시 검색하도록 만듦(검색 방법은 pid[즉, 0번째 방법)])
    this->show();

    for(int i=0 ; i<count.toInt() ; i++)
    {
        QTreeWidgetItem* row = new QTreeWidgetItem;
        ui->treeWidget->addTopLevelItem(row);

        QString tempLine = data.split("<r>")[i];

        row->setText(0, tempLine.split("|")[0]);
        row->setText(1, tempLine.split("|")[1]);
        row->setText(2, tempLine.split("|")[2]);
    }
}


void ChoosePatientManager::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    qDebug() << item->text(0);
    qDebug() << item->text(1);
    qDebug() << item->text(2);

    QString sendData = "SEN^PSE<CR>0<CR>" + item->text(0);

    //PSE event 보내기
    emit choosePatientSignal(sendData);

    ui->treeWidget->clear();
    this->close();
}

