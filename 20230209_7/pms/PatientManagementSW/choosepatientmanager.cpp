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
