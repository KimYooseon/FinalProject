#include "enrollmanager.h"
#include "ui_enrollmanager.h"
#include <QMessageBox>
#include <QFileDialog>
#include <QPixmap>

EnrollManager::EnrollManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::EnrollManager)
{
    ui->setupUi(this);
    this->setFixedSize(782, 650);


    QString enrollButtonStyle = "QPushButton { "
                                "background-color: #ED8817;"
                                "border-radius:10px;"
                                "color:#ffffff;"
                                "outline: 0; "
                                "}"
                                "QPushButton:hover { "
                                "background-color: #F2A754;"
                                "border-radius:10px;"
                                "color:#ffffff;"
                                "outline: 0; "
                                "}";

    QString selectFileButtonStyle = "QPushButton { "
                                    "background-color: rgb(150, 150, 150);"
                                    "border-radius:10px;"
                                    "color:#ffffff;"
                                    "outline: 0; "
                                    "}"
                                    "QPushButton:hover { "
                                    "background-color: #F2A754;"
                                    "border-radius:10px;"
                                    "color:#ffffff;"
                                    "outline: 0; "
                                    "}";








    ui->enrollPushButton->setStyleSheet(enrollButtonStyle);
    ui->selectFilePushButton->setStyleSheet(selectFileButtonStyle);

    ui->birthDateEdit->setCalendarPopup(true);
    ui->birthDateEdit->show();


    connect(ui->enrollPushButton, SIGNAL(clicked()), this, SLOT(on_pushButton_clicked()));




}

EnrollManager::~EnrollManager()
{


    delete ui;
}

void EnrollManager::on_pushButton_clicked()
{
    //int id = makeID(); //??????????????????
    QString /*pid, */name, sex, date, tel, address, memo;



    name = ui->nameLineEdit->text();
    //sex = ui->sexComboBox->currentText();                 //???????????????????????????
    if(ui->maleRadioButton->isChecked()==1)
        sex = "??????";
    else if(ui->femaleRadioButton->isChecked()==1)
        sex = "??????";

    date = ui->birthDateEdit->date().toString("yyyy-MM-dd");
    tel = ui->telLineEdit->text();
    address = ui->addressTextEdit->toPlainText();
    memo = ui->memoTextEdit->toPlainText();



    //???????????? ???????????? ????????? ??? ?????? ???????????? ????????? ?????????(?????? ????????? ?????? ?????? ????????????)
    if(name == "" || sex == "" || date == "" || tel == "" || address == "")
    {
        QMessageBox::critical(this, tr("??????"), tr("?????? ?????? ??????(*)??? ?????? ??????????????????."));
        return;
    }




    QString editFlag = "0";
    QString saveFileName;

    //???????????? ????????? ????????? ????????? Face????????? ??????
    if(fileName.length()>0)
    {
        saveFileName = QString("./Face/%1.png").arg(pidPhoto);
        QFile::copy(fileName, saveFileName);
        editFlag ="1";

    }

qDebug() <<"??????: "<<fileName<<", "<<saveFileName;



    //qDebug() << sex << "/" << date << "/" << address;

    QString newData = "SEN^PER<CR>" + newPID + "<CR>" + name + '|' + sex + '|' + date + '|' + tel + '|' + address + '|' + memo;
    emit sendNewData(newData);

    QString showdata = name + '|' + sex + '|' + date + '|' + tel + '|' + address + '|' + memo + '|' + pidPhoto + "|"+ editFlag;
    emit sendNewDataForShow(newPID, showdata);





    QMessageBox::information(this, tr("???????????? ??????"), \
                             tr("???????????? ????????? ?????????????????????."));

    this->close();

    ui->nameLineEdit->clear();

    ui->maleRadioButton->clearFocus();
    ui->maleRadioButton->setAutoExclusive(false);
    ui->maleRadioButton->setChecked(false);
    ui->maleRadioButton->setAutoExclusive(true);

    ui->femaleRadioButton->clearFocus();
    ui->femaleRadioButton->setAutoExclusive(false);
    ui->femaleRadioButton->setChecked(false);
    ui->femaleRadioButton->setAutoExclusive(true);

    ui->birthDateEdit->clear();
    ui->telLineEdit->clear();
    ui->addressTextEdit->clear();
    ui->memoTextEdit->clear();
    ui->enrollImageLabel->clear();
}

void EnrollManager::newPIDSended(QString sendedPID)
{
    newPID = sendedPID;
    qDebug()<<"enroll box - pid : " << newPID;
    ui->pidLineEdit->setText(newPID);
}



void EnrollManager::on_selectFilePushButton_clicked()
{
    pidPhoto = ui->pidLineEdit->text();


    fileName = QFileDialog::getOpenFileName(this,
                                            tr("Open Image"), QString("./Face/%1").arg(pidPhoto), tr("Image Files (*.png *.jpg *.bmp)"));

    qDebug() << "fileName"<<fileName;



    QFileDialog dialog(this);
    dialog.setNameFilter(tr("Images (*.png *.xpm *.jpg)"));

    QPixmap pix(QString("%1").arg(fileName));

    pix.scaled(150,180,Qt::IgnoreAspectRatio);
    ui->enrollImageLabel->setPixmap(pix.scaled(ui->enrollImageLabel->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));




}
