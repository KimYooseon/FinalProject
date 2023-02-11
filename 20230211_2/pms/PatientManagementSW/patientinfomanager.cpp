#include "patientinfomanager.h"
#include "ui_patientinfomanager.h"

#include "choosepatientmanager.h"

#include <QMessageBox>
#include <QGraphicsEffect>
#include <QFile>
#include <QFileDialog>

PatientInfoManager::PatientInfoManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::PatientInfoManager)
{
    ui->setupUi(this);
    ui->clientInfoTableWidget->setColumnWidth(0,290);
    ui->searchPushButton->setIcon(QIcon("./search.png"));




    choosePatientManager = new ChoosePatientManager(0);


    QString pushButtonStyle = "QPushButton { "
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
    QString comboBoxStyle = "QComboBox { "
                            "background-color: #ED8817;"
                            "border-radius:10px;"
                            "color:#ffffff;"
                            "outline: 0; "
                            "}"
                            "QComboBox:hover { "
                            "background-color: #F2A754;"
                            "border-radius:10px;"
                            "color:#ffffff;"
                            "outline: 0; "
                            "}";

    QString labelStyle = "QLabel { "
                         "background-color: rgb(150, 150, 150);"
                         "border-radius:10px;"
                         "color:#ffffff;"
                         "outline: 0; "
                         "}";

    QString changePhotoPushButtonStyle = "QPushButton { "
                                             "background-color: rgb(170, 170, 170);"
                                             "border-radius:5px;"
                                             "color:#ffffff;"
                                             "outline: 0; "
                                             "}"
                                             "QPushButton:hover { "
                                             "background-color: rgb(200, 200, 200);"
                                             "border-radius:5px;"
                                             "color:#ffffff;"
                                             "outline: 0; "
                                             "}"
                                             "QPushButton:disabled { "
                                             "background-color: #ffffff;"
                                             "border-radius:5px;"
                                             "border:1px solid rgb(220, 220, 220);"
                                             "color:rgb(220, 220, 220);"
                                             "outline: 0; "
                                                 "}";

        QGraphicsDropShadowEffect *effect = new QGraphicsDropShadowEffect;
        effect->setBlurRadius(5);
        effect->setXOffset(5);
        effect->setYOffset(5);
        effect->setColor(QColor(220,220,220));
        ui->label_2->setGraphicsEffect(effect);


        ui->modifyPushButton->setStyleSheet(pushButtonStyle);
        ui->deletePushButton->setStyleSheet(pushButtonStyle);
        ui->WaitPushButton->setStyleSheet(pushButtonStyle);
        ui->searchPushButton->setStyleSheet(pushButtonStyle);
        ui->searchComboBox->setStyleSheet(comboBoxStyle);
        ui->label_2->setStyleSheet(labelStyle);
        ui->changePhotoPushButton->setStyleSheet(changePhotoPushButtonStyle);


        //초기 화면에서는 사진 변경 기능은 비활성화 상태
        ui->changePhotoPushButton->setDisabled(true);


        pixmap = new QPixmap();
        pixmap->load("./Face/default.png");
        pixmap->scaled(200,180,Qt::IgnoreAspectRatio);

        ui->patientFace->setPixmap(pixmap->scaled(ui->patientFace->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));
        //ui->patientFace->setAlignment(Qt::AlignBottom | Qt::AlignCenter);



        connect(this, SIGNAL(sendTempData(QString, QString)), choosePatientManager, SLOT(sendedTempData(QString, QString)));   //인자 필요없지만 signal 재활용 위해 사용


}

PatientInfoManager::~PatientInfoManager()
{
    delete ui;
}



void PatientInfoManager::on_searchPushButton_clicked()
{
    int comboBoxIndex = ui->searchComboBox->currentIndex();     //i에 검색콤보박스의 현재인덱스 번호를 저장
    //qDebug()<< comboBoxIndex;
    QString searchInfo = ui->searchLineEdit->text();
    qDebug()<< searchInfo;
    QString searchData = "SEN^PSE<CR>" + QString::number(comboBoxIndex) + "<CR>" + searchInfo;

    ui->clientInfoTableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);   //검색하면 테이블 정보들 더블클릭해서 수정가능하도록 만듦
    qDebug() <<"searchData: " << searchData;





    emit sendSearchData(searchData);
    emit downloadOrNotSignal(); //새로운 환자 이미지 다운로드 해도 되는지 확인을 위해 signal 보내기

    ui->searchLineEdit->clear();
}

void PatientInfoManager::searchDataSended(QString id, QString data)
{
qDebug()<<"왜안대징";


    //P가 들어가 있지 않고(pid가 오면 정상적으로 검색된 것임) id가 2이상일 경우
    if(id.toInt() > 1 && id.contains("P", Qt::CaseInsensitive) == false)
    {
        //여기 하는중!!!!!!!!!!!!
        //환자 선택 창 띄움. 그 창에서 특정 treeWidget 클릭하면 PSE로 해당 PID 보내줘서 다시 검색하도록 만듦(검색 방법은 pid[즉, 0번째 방법)])
        choosePatientManager->show();


        //시그널로 정보를 보내줌. id는 검색된 환자수, data는 검색된 환자정보들의 모음
        emit sendTempData(id, data);

        return;
    }

qDebug() << "*******************서치데이타 받은거 id: " << id << ", data: " << data;


    qDebug("^^^^^^^^^1^^^^^^^^^^^^%d", __LINE__);
    qDebug() << "searchButtonClicked"<<searchButtonClicked;
    if(searchButtonClicked != 0)    //이전에 검색한 환자의 이미지 파일이 다 다운로드 된 상태가 아닐 때는 return
    {
        qDebug() << "혹시 여기?";
        return;
    }

    //    if(fileSendedFlag==0)   // 파일이 다 다운로드 된 상태가 아닐 때는 재검색한 환자의 정보를 띄우면 안됨
    //        return;
    qDebug("^^^^^^^^^^2^^^^^^^^^^^%d", __LINE__);


    qDebug() << "@@@@@@@@@data: " << data;
    //없는 환자 검색했을 때
    if(data=="<NEL>")
    {
        qDebug("^^^^^^^^^^3^^^^^^^^^^^%d", __LINE__);
        patientInDB = 0;

        //없는 환자 정보를 찾으면 환자 정보도 다 clear해줄 것
        ui->searchLineEdit->clear();
        ui->patientFace->clear();

        //테이블도 같이 clear해주자..................................~
        for(int i = 0 ; i<7; i++)
        {
            ui->clientInfoTableWidget->item(i, 0)->setText("");
        }


        //사진 변경도 불가능하게 만들기
        ui->changePhotoPushButton->setDisabled(true);

        //여기에 QMessageBox 띄우기
        QMessageBox::critical(this, tr("경고"), tr("해당 환자의 정보를 찾을 수 없습니다.\n"
                                                 "검색할 내용을 다시 한번 확인해 주세요."));

        return;
    }

    //있는 환자면 활성화
    ui->changePhotoPushButton->setDisabled(false);


    //    else-
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);


    //환자가 있음
    patientInDB = 1;

    pid = id;
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    name = data.split("|")[0];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    //qDebug() << "name in searchDataSended: " << name;
    sex = data.split("|")[1];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    birthdate = data.split("|")[2];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    tel = data.split("|")[3];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    address = data.split("|")[4];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    memo = data.split("|")[5];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    //fileName = data.split("|")[6];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    //qDebug() << "((((((((((((fileName: " <<fileName;

    //int editFlag = data.split("|")[7].toInt();
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    //qDebug() << "editFlag: " <<editFlag;

    //등록이 되었을 때부터 수정이 가능하도록 만드는 부분
    //if(editFlag == 1)
    //    ui->clientInfoTableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);

    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);

    qDebug("%d", __LINE__);

    //    QTableWidgetItem *item = new QTableWidgetItem;
    //    item->setText(pid);
    ui->clientInfoTableWidget->setItem(0, 0, new QTableWidgetItem(pid)/*item*/);
    ui->clientInfoTableWidget->setItem(1, 0, new QTableWidgetItem(name));
    //    qDebug() << "name in searchDataSended(in table):" << ui->clientInfoTableWidget->itemAt(1)->currentItem()->row()->text();
    //qDebug() << "name in searchDataSended(in table):"<< ui->clientInfoTableWidget->item(1, 0)->text();
    ui->clientInfoTableWidget->setItem(2, 0, new QTableWidgetItem(sex));
    ui->clientInfoTableWidget->setItem(3, 0, new QTableWidgetItem(birthdate));
    ui->clientInfoTableWidget->setItem(4, 0, new QTableWidgetItem(tel));
    ui->clientInfoTableWidget->setItem(5, 0, new QTableWidgetItem(address));
    ui->clientInfoTableWidget->setItem(6, 0, new QTableWidgetItem(memo));

    qDebug("%d", __LINE__);
    pixmap = new QPixmap();
    pixmap->load(QString("./Face/%1.png").arg(pid));
    pixmap->scaled(200,180,Qt::IgnoreAspectRatio);

    qDebug("%d", __LINE__);
    ui->patientFace->setPixmap(pixmap->scaled(ui->patientFace->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));


    qDebug("%d", __LINE__);

    searchButtonClicked+=1;


    //search버튼 클릭되면 이전 환자의 이미지가 보이지 않도록 imageManager쪽으로 signal 보내줌
    emit cleanImageSignal();


}


void PatientInfoManager::on_deletePushButton_clicked()
{
    int delButtonNum = QMessageBox::critical(this, tr("경고"),
                                             tr("해당 환자와 연관된 사진이 모두 삭제됩니다.\n"
                                                "계속하시겠습니까?"), QMessageBox::Yes | QMessageBox::No);


    switch (delButtonNum) {
    case QMessageBox::Yes:
    {
        //patientStatusManager쪽으로 pid를 보내줌. patientStatusManager에서는 이 pid를 검색해 대기/수납 명단에 없다면 0, 있다면 1을 반환해줄 것
        emit sendDelPID(pid);


        /*이 부분 슬롯에 넣기
        QString delData = "PDE<CR>" + pid + "<CR> "; //지울 pid를 emit으로 네트워크 매니저로 보냄/네트워크 매니저는 서버로 삭제할 데이터 전달
        emit sendDelData(delData);

        ui->searchLineEdit->clear();
        ui->patientFace->clear();

        //환자 정보 clear
        for(int i=0; i<7;i++)
        {
            ui->clientInfoTableWidget->item(i, 0)->setText("");
        }
        */

        break;
    }
    case QMessageBox::No:
    {
        return;
        break;
    }
    }



    //    QMessageBox::critical(this, tr("경고"), \
    //                          tr("해당 환자와 연관된 사진이 모두 삭제됩니다. 계속하시겠습니까?"));



}

void PatientInfoManager::delFlagSended(int delFlag)
{
    if(delFlag == 0)
    {
        QString delData = "SEN^PDE<CR>" + pid + "<CR> "; //지울 pid를 emit으로 네트워크 매니저로 보냄/네트워크 매니저는 서버로 삭제할 데이터 전달
        emit sendDelData(delData);

        ui->searchLineEdit->clear();
        ui->patientFace->clear();

        //환자 정보 clear
        for(int i=0; i<7;i++)
        {
            ui->clientInfoTableWidget->item(i, 0)->setText("");
        }

        //얼굴 사진도 삭제
        QFile::remove(QString("./Face/%1.png").arg(pid));

    }
    else if(delFlag == 1)
    {
        QMessageBox::warning(this, tr("경고"), \
                             tr("대기리스트에 추가되어 있는 환자의 정보는 삭제하실 수 없습니다.\n"));
    }
}



void PatientInfoManager::on_WaitPushButton_clicked()
{
    //이름과 pid는 바뀌지 않는 정보지만 나머지 정보는 검색 후에 수정했을 수도 있으니 현재 테이블에 저장되어있던 값을 가지고 와 저장해준후 서버로 보내기
    //qDebug() << "대기명단에 추가한 pid: " << ui->clientInfoTableWidget->itemAt(0,0)->text();
    pid = ui->clientInfoTableWidget->item(0, 0)->text();
    name = ui->clientInfoTableWidget->item(1, 0)->text();
    //qDebug() << "name: " << ui->clientInfoTableWidget->itemAt(5,0)->text();

    sex = ui->clientInfoTableWidget->item(2,0)->text();
    birthdate = ui->clientInfoTableWidget->item(3,0)->text();
    tel = ui->clientInfoTableWidget->item(4,0)->text();
    address = ui->clientInfoTableWidget->item(5,0)->text();
    memo = ui->clientInfoTableWidget->item(6,0)->text();

    emit sendPIDtoWaitList(pid);

//    //qDebug() << "waitSignal: " << waitSignal;



}

void PatientInfoManager::inWaitListSlot(int inWaitListOrNot)
{
    //qDebug()<<"inWaitLsit: " << inWaitListOrNot;

    if(inWaitListOrNot == 1)
    {
        QMessageBox::critical(this, tr("경고"), \
                              tr("이미 대기명단에 있는 환자입니다."));
        return;
    }


    if(inWaitListOrNot == 0)
    {
        QString sendData = "SEN^AWL<CR>" + pid + "<CR>" + name + "|" + sex + "|" + birthdate + "|" + tel + "|" + address + "|" + memo;
        //qDebug() << "서버로 보낼 대기환자 데이터: " <<sendData;

        emit sendWaitInfo(sendData);
    }
    //qDebug()<<" ";

}

void PatientInfoManager::on_modifyPushButton_clicked()
{
    //네트워크쪽으로 정보 넘겨주도록 signal emit하고 mainwindow에서 연결하고 서버에 넘겨서 update문으로 db 테이블 수정
    qDebug()<< "modify Button clicked";
    pid = ui->clientInfoTableWidget->item(0, 0)->text();
    name = ui->clientInfoTableWidget->item(1, 0)->text();
    sex = ui->clientInfoTableWidget->item(2,0)->text();
    birthdate = ui->clientInfoTableWidget->item(3,0)->text();
    tel = ui->clientInfoTableWidget->item(4,0)->text();
    address = ui->clientInfoTableWidget->item(5,0)->text();
    memo = ui->clientInfoTableWidget->item(6,0)->text();

    QString modifyData = "SEN^PMO<CR>" + pid + "<CR>" + name + "|" + sex + "|" + birthdate + "|" + tel + "|" + address + "|" + memo;
    //qDebug()<< "modifyData: " << modifyData;
    emit sendModifyData(modifyData);

}

void PatientInfoManager::fileSendedSlot(int fileSendedSignal)
{
    //파일이 완전히 전송되었다면 Flag가 0에서 1로 바뀌었을 것.
qDebug() << "1 bbbbbbbbbbbbbbbbbbbbbb patientInDB: " << patientInDB;
    qDebug() <<"eeeeeeeeeeeeeeeeeeeeeee fileSendedSignal: " <<fileSendedSignal;

    //DB에서 검색한 환자를 찾을 수 없으면 파일 받지 않기
    if(patientInDB == 0)
        return;



    fileSendedFlag = fileSendedSignal;

    if(fileSendedFlag==0)
    {
        QMessageBox::critical(this, tr("경고"), tr("이전에 검색한 환자 이미지를 불러오는 중입니다.\n"
                                                 "다운로드가 완료되면 다시 검색해주세요."));
        return;
    }
    else if(fileSendedFlag ==1)
    {


        //없는 환자면 pid가 NULL이니 QMessageBox를 띄우지 않고 return해줌
        if(pid=="NULL")
        {
            //환자정보 테이블에 띄울 수 있는 상태로 초기화
            return;
        }

    qDebug() << "2 bbbbbbbbbbbbbbbbbbbbbb patientInDB: " << patientInDB;
        QMessageBox::information(this, tr("정보"), tr("검색한 환자 이미지가 전부 로드되었습니다."));

        //환자정보 테이블에 띄울 수 있는 상태로 초기화
        searchButtonClicked=0;
        qDebug() << "SDFFSDDSFDSD searchButtonClicked" << searchButtonClicked;
    }



}



//환자 등록시
void PatientInfoManager::sendedNewDataForShow(QString id, QString data)
{
    qDebug("^^^^^^^^^1^^^^^^^^^^^^%d", __LINE__);
    qDebug() << "searchButtonClicked"<<searchButtonClicked;
    //        if(searchButtonClicked != 0)    //이전에 검색한 환자의 이미지 파일이 다 다운로드 된 상태가 아닐 때는 return
    //            return;

    //        if(fileSendedFlag==0)   // 파일이 다 다운로드 된 상태가 아닐 때는 재검색한 환자의 정보를 띄우면 안됨
    //            return;
    qDebug("^^^^^^^^^^2^^^^^^^^^^^%d", __LINE__);
    //없는 환자 검색했을 때
    //        if(data=="<NEL>")
    //        {
    //            qDebug("^^^^^^^^^^3^^^^^^^^^^^%d", __LINE__);
    //            patientInDB = 0;
    //            //여기에 QMessageBox 띄우기
    //            QMessageBox::critical(this, tr("경고"), tr("해당 환자의 정보를 찾을 수 없습니다.\n"
    //                                                     "검색할 내용을 다시 한번 확인해 주세요."));
    //            ui->searchLineEdit->clear();
    //            return;
    //        }
    //    //    else
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    //        patientInDB = 1;

    pid = id;
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    name = data.split("|")[0];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    //qDebug() << "name in searchDataSended: " << name;
    sex = data.split("|")[1];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    birthdate = data.split("|")[2];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    tel = data.split("|")[3];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    address = data.split("|")[4];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    memo = data.split("|")[5];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    fileName = data.split("|")[6];
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    qDebug() << "((((((((((((fileName: " <<fileName;

    int editFlag = data.split("|")[7].toInt();
    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);
    qDebug() << "editFlag: " <<editFlag;

    //등록이 되었을 때부터 수정이 가능하도록 만드는 부분
    if(editFlag == 1)
        ui->clientInfoTableWidget->setEditTriggers(QAbstractItemView::DoubleClicked);

    qDebug("^^^^^^^^^^^^^^^^^^^^^%d", __LINE__);

    qDebug("%d", __LINE__);

    //    QTableWidgetItem *item = new QTableWidgetItem;
    //    item->setText(pid);
    ui->clientInfoTableWidget->setItem(0, 0, new QTableWidgetItem(pid)/*item*/);
    ui->clientInfoTableWidget->setItem(1, 0, new QTableWidgetItem(name));
    //    qDebug() << "name in searchDataSended(in table):" << ui->clientInfoTableWidget->itemAt(1)->currentItem()->row()->text();
    //qDebug() << "name in searchDataSended(in table):"<< ui->clientInfoTableWidget->item(1, 0)->text();
    ui->clientInfoTableWidget->setItem(2, 0, new QTableWidgetItem(sex));
    ui->clientInfoTableWidget->setItem(3, 0, new QTableWidgetItem(birthdate));
    ui->clientInfoTableWidget->setItem(4, 0, new QTableWidgetItem(tel));
    ui->clientInfoTableWidget->setItem(5, 0, new QTableWidgetItem(address));
    ui->clientInfoTableWidget->setItem(6, 0, new QTableWidgetItem(memo));

    qDebug("%d", __LINE__);
    pixmap = new QPixmap();
    pixmap->load(QString("./Face/%1.png").arg(pid));
    pixmap->scaled(200,180,Qt::IgnoreAspectRatio);

    qDebug("%d", __LINE__);
    ui->patientFace->setPixmap(pixmap->scaled(ui->patientFace->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));


    qDebug("%d", __LINE__);

    //환자가 등록되는 당시에는 환자 이미지가 없는 상태임. 따라서 searchButtonClicked를 0으로 만들어주어야 다음의 새로운 환자 검색이 가능함
    searchButtonClicked=0;


    //search버튼 클릭되면 이전 환자의 이미지가 보이지 않도록 imageManager쪽으로 signal 보내줌
    emit cleanImageSignal();
}

void PatientInfoManager::on_changePhotoPushButton_clicked()
{
    qDebug() << "클릭";

    pidPhoto = ui->clientInfoTableWidget->item(0, 0)->text();


    currentFileName = QString("./Face/%1.png").arg(pidPhoto);


    changeFileName = QFileDialog::getOpenFileName(this,
                                                   tr("Open Image"), QString("./Face/%1").arg(pidPhoto), tr("Image Files (*.png *.jpg *.bmp)"));

    qDebug() << "fileName"<<changeFileName;



    QFileDialog dialog(this);
    dialog.setNameFilter(tr("Images (*.png *.xpm *.jpg)"));


    QString saveFileName;

    //선택했던 파일을 디버그 파일의 Face폴더에 저장
    if(changeFileName.length()>0)
    {
        QFile::remove(QString("%1").arg(currentFileName));
        saveFileName = QString("%1").arg(currentFileName);
        QFile::copy(changeFileName, currentFileName);

    }

    QPixmap pix(QString("%1").arg(currentFileName));

    pix.scaled(150,180,Qt::IgnoreAspectRatio);
    ui->patientFace->setPixmap(pix.scaled(ui->patientFace->size(), Qt::KeepAspectRatio, Qt::SmoothTransformation));



    qDebug() <<"currentFileName: " <<currentFileName << " / " << "changeFileName: " <<changeFileName;
}

//대기명단 추가에 대한 서버로부터 응답받
void PatientInfoManager::sendedAWLRequest(QString saveData)
{
    QString tempData = saveData.split("^")[0];
    if(tempData=="ERR")
    {
        QMessageBox::critical(nullptr, tr("경고"), tr("뷰어가 켜져있지 않습니다.\n"
                                                    "대기명단에 환자를 추가하기 위해 뷰어를 켜 주세요."));

        waitError = 1;
    }
    else
    {
        waitError = 0;

        //이름과 pid는 바뀌지 않는 정보지만 나머지 정보는 검색 후에 수정했을 수도 있으니 현재 테이블에 저장되어있던 값을 가지고 와 저장해준후 서버로 보내기
        //qDebug() << "대기명단에 추가한 pid: " << ui->clientInfoTableWidget->itemAt(0,0)->text();

//이미 저장되어있는 상태임
//        pid = ui->clientInfoTableWidget->item(0, 0)->text();
//        name = ui->clientInfoTableWidget->item(1, 0)->text();
//        //qDebug() << "name: " << ui->clientInfoTableWidget->itemAt(5,0)->text();

//        sex = ui->clientInfoTableWidget->item(2,0)->text();
//        birthdate = ui->clientInfoTableWidget->item(3,0)->text();
//        tel = ui->clientInfoTableWidget->item(4,0)->text();
//        address = ui->clientInfoTableWidget->item(5,0)->text();
//        memo = ui->clientInfoTableWidget->item(6,0)->text();

        emit sendWaitInfoToWaitList(pid, name);


    }
}
