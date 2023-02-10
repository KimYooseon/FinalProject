#include "choosepatientmanager.h"
#include "ui_choosepatientmanager.h"
#include "networkmanager.h"

ChoosePatientManager::ChoosePatientManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ChoosePatientManager)
{
    ui->setupUi(this);

    networkManager = new NetworkManager(this);


    connect(this, SIGNAL(choosePatientSignal(QString)), networkManager, SLOT(newDataSended(QString)));

}

ChoosePatientManager::~ChoosePatientManager()
{
    delete ui;
}

void ChoosePatientManager::sendedTempData(QString count, QString data)
{
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

    QString sendData = "PSE<CR>0<CR>" + item->text(0);

    //PSE event 보내기
    emit choosePatientSignal(sendData);

    ui->treeWidget->clear();
    this->close();
}

