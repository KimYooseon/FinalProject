#include "mainwindow.h"
#include "ui_mainwindow.h"

#include "enrollmanager.h"
#include "imagemanager.h"
#include "medicalrecordmanager.h"
#include "patientinfomanager.h"
#include "patientstatusmanager.h"
#include "networkmanager.h"


//static inline QByteArray IntToArray(qint32 source);

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    ui->enrollButton->setIcon(QIcon("./enroll.png"));

    this->showMaximized();


    ui->enrollButton->setStyleSheet("QPushButton { "
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
                                "}");





    imageManager = new ImageManager(this);
    medicalRecordManager = new MedicalRecordManager(this);
    patientInfoManager = new PatientInfoManager(this);
    patientStatusManager = new PatientStatusManager(this);
    enrollManager = new EnrollManager(0);
    networkManager = new NetworkManager(this);


    QVBoxLayout *imageLayout = new QVBoxLayout(this);
    QVBoxLayout *recordLayout = new QVBoxLayout(this);
    QVBoxLayout *infoLayout = new QVBoxLayout(this);
    QVBoxLayout *statusLayout = new QVBoxLayout(this);


    imageLayout->addWidget(imageManager);
    recordLayout->addWidget(medicalRecordManager);
    infoLayout->addWidget(patientInfoManager);
    statusLayout->addWidget(patientStatusManager);


    ui->imageFrame->setLayout(imageLayout);
    ui->medicalRecordFrame->setLayout(recordLayout);
    ui->patientInfoFrame->setLayout(infoLayout);
    ui->patientStatusFrame->setLayout(statusLayout);



    connect(ui->enrollButton, SIGNAL(clicked()), this, SLOT(enrollClient()));

//    connect(patientInfoManager, SIGNAL(()),
//            this, SLOT(()));

    connect(enrollManager, SIGNAL(sendNewData(QString)), networkManager, SLOT(newDataSended(QString)));
    connect(enrollManager, SIGNAL(sendNewDataForShow(QString, QString)), patientInfoManager, SLOT(sendedNewDataForShow(QString, QString)));


    connect(medicalRecordManager, SIGNAL(sendReSearchData(QString)), networkManager, SLOT(newDataSended(QString)));



    connect(patientInfoManager, SIGNAL(sendSearchData(QString)), networkManager, SLOT(newDataSended(QString)));
    connect(this, SIGNAL(requestPID(QString)), networkManager, SLOT(newDataSended(QString)));
    connect(networkManager, SIGNAL(sendNewPID(QString)), enrollManager, SLOT(newPIDSended(QString)));
    connect(networkManager, SIGNAL(sendSearchResult(QString, QString)), patientInfoManager, SLOT(searchDataSended(QString, QString)));

    //????????? ??? ?????? ???????????? ????????? ??? ????????? pid??? ??????
    //connect(networkManager, SIGNAL(sendSearchResult(QString, QString)), imageManager, SLOT(PSEDataToImgManager(QString, QString)));
    connect(networkManager, SIGNAL(PSEDataInNET(QString)), imageManager, SLOT(PSEDataSended(QString)));

    connect(patientInfoManager, SIGNAL(sendDelData(QString)), networkManager, SLOT(newDataSended(QString)));
    //connect(patientInfoManager, SIGNAL(sendWaitInfo(QString, QString)), patientStatusManager, SLOT(waitInfoSended(QString, QString)));



    //?????? ????????? ???!
    //connect(patientInfoManager, SIGNAL(sendWaitInfo(QString)), patientStatusManager, SLOT(waitInfoSended(QString)));

    //???????????? ?????? ????????? ???????????? ???????????? ??????
    connect(networkManager, SIGNAL(sendAWLRequest(QString)), patientInfoManager, SLOT(sendedAWLRequest(QString)));






    //?????? ???????????? ?????? ?????????? ?????? ??????????????? ?????????
    connect(patientInfoManager, SIGNAL(sendWaitInfo(QString)), networkManager, SLOT(newDataSended(QString)));

    //??????????????? ?????????????????? ?????????
    //connect(patientStatusManager, SIGNAL(sendWaitInfo(QString)), networkManager, SLOT(newDataSended(QString)));
    connect(patientStatusManager, SIGNAL(sendRequest(QString)), networkManager, SLOT(newDataSended(QString)));
    connect(patientInfoManager, SIGNAL(sendModifyData(QString)), networkManager, SLOT(newDataSended(QString)));


    //connect(patientInfoManager, SIGNAL(sendWaitToServer(QString)), networkManager, SLOT(newDataSended(QString)));
    connect(networkManager, SIGNAL(sendSRQRequest(QString)), patientStatusManager, SLOT(statusRequestSended(QString)));
    connect(networkManager, SIGNAL(sendVTSRequest(QString)), patientStatusManager, SLOT(statusRequestSended(QString)));

    //????????? ???????????? ?????????????????? ?????? ?????? ??? ????????? ???
    connect(networkManager, SIGNAL(sendVNTevent(QString)), medicalRecordManager, SLOT(addNewRecord(QString)));

    connect(networkManager, SIGNAL(sendISVevent(QString)), patientStatusManager, SLOT(statusRequestSended(QString)));


    connect(networkManager, SIGNAL(sendVTFevent(QString)), patientStatusManager, SLOT(statusRequestSended(QString)));

    connect(networkManager, SIGNAL(sendSearchResult(QString, QString)), medicalRecordManager, SLOT(recordDataSended(QString, QString)));



    connect(patientInfoManager, SIGNAL(sendPIDtoWaitList(QString)), patientStatusManager, SLOT(PIDsendedtoWaitList(QString)));
    connect(patientStatusManager, SIGNAL(inWaitListSignal(int)), patientInfoManager, SLOT(inWaitListSlot(int)));



    connect(patientInfoManager, SIGNAL(sendDelPID(QString)), patientStatusManager, SLOT(delPIDSended(QString)));
    connect(patientStatusManager, SIGNAL(sendDelFlag(int)), patientInfoManager, SLOT(delFlagSended(int)));
    connect(patientInfoManager, SIGNAL(sendDelPID(QString)), imageManager, SLOT(delPIDSendedToImg(QString)));   //?????? ??????????????? signal ????????? ?????? ??????



    connect(patientStatusManager, SIGNAL(sendPayInfo(QString)), networkManager, SLOT(newDataSended(QString)));



    connect(networkManager, SIGNAL(sendWTRevent(QString)), patientStatusManager, SLOT(oldListSended(QString)));



    connect(patientInfoManager, SIGNAL(downloadOrNotSignal()), networkManager, SLOT(downloadOrNotSlot()));





//    connect(networkManager, SIGNAL(sendByteArray(QByteArray*)), patientInfoManager, SLOT(byteArraySended(QByteArray*)));
//    connect(networkManager, SIGNAL(sendByteArray(const QPixmap&)), patientInfoManager, SLOT(byteArraySended(const QPixmap&)));


    connect(networkManager, SIGNAL(fileSendedSig(int)), patientInfoManager, SLOT(fileSendedSlot(int)));


    //search?????? ???????????? ?????? ????????? ???????????? ????????? ????????? imageManager????????? signal ?????????
    connect(patientInfoManager, SIGNAL(cleanImageSignal()), imageManager, SLOT(cleanImageSlot()));   //?????? ??????????????? signal ????????? ?????? ??????


    //?????????????????? ?????? ????????? ??????????????? ??????
    connect(networkManager, SIGNAL(sendMWLevent(QString)), patientStatusManager, SLOT(statusRequestSended(QString)));


    connect(patientInfoManager, SIGNAL(sendWaitInfoToWaitList(QString, QString)), patientStatusManager, SLOT(waitInfoSended(QString, QString)));
}

MainWindow::~MainWindow()
{
    delete ui;
}


void MainWindow::enrollClient()
{
    enrollManager->show();
    QString requestPIDData = "SEN^PID<CR>0<CR>0";  //PID?????? ???????????? ???????????? ?????? ??????. ????????? ???????????? 0?????? ???????????? ???????????? ????????? ???????????????
    emit requestPID(requestPIDData);

}


