#include "mainserver.h"
#include "ui_mainserver.h"
#include <QTcpServer>
#include <QTcpSocket>
#include <QSqlQuery>
#include <QSqlDatabase>
#include <QTableWidget>
#include <QSqlRecord>
#include <QTcpSocket>
#include <QTextStream>

//mvc패턴 적용시키기 위해 정보 변경&삭제시 뷰어와 이미징 모듈에 패킷 보내줄 것 ex. ACK^PMO<CR>P00001<CR>NULL

MainServer::MainServer(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainServer)
{
    ui->setupUi(this);
    server = new QTcpServer(this);

    connect(server, SIGNAL(newConnection()), this, SLOT(newConnection()));

    QString socket_data = QString("MainServer Listening: %1\n").arg(server->listen(QHostAddress::Any, 8000) ? "true" : "false");
    ui->textEdit->append(socket_data);



    fileServer = new QTcpServer(this);
    //fileServer->listen(QHostAddress::Any, 8001);
    QString fileSocket_data = QString("FileServer Listening: %1\n").arg(fileServer->listen(QHostAddress::Any, 8001) ? "true" : "false");
    connect(fileServer, SIGNAL(newConnection()), this, SLOT(newFileConnection()));
    ui->textEdit->append(fileSocket_data);


    //DB 로드
    this->loadData();

}

MainServer::~MainServer()
{
    delete ui;
}

//새로운 데이터 소켓이 연결될 때
void MainServer::newConnection()
{
    qDebug() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
    QTcpSocket* socket= server->nextPendingConnection();
//    while (server->hasPendingConnections())
//    {
//        socket = server->nextPendingConnection();
//        connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
//        connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
//        QByteArray *buffer = new QByteArray();
//        qint32 *s = new qint32(0);
//        buffers.insert(socket, buffer);
//        sizes.insert(socket, s);
//    }
    connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
    connect(socket, SIGNAL(disconnected()), this, SLOT(socketDisconnected()));
}

//새로운 파일 소켓이 연결될 때
void MainServer::newFileConnection()
{
    QTcpSocket* fileSocket = fileServer->nextPendingConnection();           //receivedSocket에 fileServer에서 저장해두었던 다음 보류중인 연결을 연결해준다
    connect(fileSocket, SIGNAL(readyRead()), this, SLOT(receiveFile()));     //받은 소켓에서 정보를 읽어 serverform에서 파일 전송이 가능하도록 만듦
    qDebug("new file connection");
    qDebug() << fileSocket->readAll().toStdString().c_str();
}




void MainServer::receiveFile()
{
    //이미지 파일명: type_date.bmp

    //ex.CNT<CR>IMG<CR>NULL
    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());

    if (pmsFileSocket != socket && imagingFileSocket != socket && viewerFileSocket != socket) {
        QByteArray arr = socket->readAll();
        QString id = QString(arr).split("<CR>")[1];
        if (id == "IMG") {
            qDebug("%d: RECEIVED IMG SOCKET", __LINE__);
            imagingFileSocket = socket;
        }
        else if (id == "PMS") {
            qDebug("%d: RECEIVED PMS SOCKET", __LINE__);
            //qDebug() << "pmsFileSocket saveData: " << QString(arr);
            pmsFileSocket = socket;
        } else if(id == "VEW") {
            qDebug("%d: RECEIVED VIEWER SOCKET", __LINE__);
            //qDebug() << "viewerFileSocket saveData: " << QString(arr);
            viewerFileSocket =socket;
        }

        return;
    }

    // Beginning File Transfer
    if (byteReceived == 0) {                                    // First Time(Block) , var byteReceived is always zero
        QDataStream in(imagingFileSocket);
        in.device()->seek(0);
        in >> totalSize >> byteReceived >> currentPID >> type;

        QDir dir(QString("./Image/%1").arg(currentPID));
        if (!dir.exists())
            dir.mkpath(".");

        //fileName = type + "_" + QDate::currentDate().toString("yyyyMMdd") + ".bmp";
        fileName = type + "_" + QDate::currentDate().toString("yyyyMMdd") + ".jpg";
        currentFileName = dir.path() + "/" + fileName;
        file = new QFile(currentFileName);
        file->open(QFile::WriteOnly);

    } else {
        inBlock = imagingFileSocket->readAll();

        byteReceived += inBlock.size();
        file->write(inBlock);
        file->flush();
    }

    if (byteReceived == totalSize) {        // file sending is done
        qDebug() << QString("%1 receive completed").arg(fileName);
        inBlock.clear();
        byteReceived = 0;
        totalSize = 0;
        file->close();
        delete file;





        QString newIID = makeImageNo();
        qDebug() << "newIID" << newIID;
        qDebug() << "currentPID" << currentPID;
        qDebug() << "type" << type;
        qDebug() << "image_date" << QDate::currentDate().toString("yyyyMMdd");
        qDebug() << "currentFileName" << currentFileName;


        query3->prepare("INSERT INTO image (image_no, patient_no, type, image_date, image_path)"
                        "VALUES(:image_no, :patient_no, :type, :image_date, :image_path)");


        query3->bindValue(":image_no", newIID);
        query3->bindValue(":patient_no", currentPID);
        query3->bindValue(":type", type);
        query3->bindValue(":image_date", QDate::currentDate().toString("yyyyMMdd"));
        query3->bindValue(":image_path", currentFileName);
        query3->exec();

        qDebug()<<"새로운 이미지 정보 저장 완료";
        updateRecentData();







    }

}



void MainServer::goOnSend(qint64 numBytes)
{

    numBytes = 40;
    outBlock.clear();

    /*파일의 전체 크기에서 numBytes씩만큼 나누어 전송*/
    byteToWrite -= numBytes; // 데이터 사이즈를 유지
    outBlock = file->read(qMin(byteToWrite, numBytes));
    pmsFileSocket->write(outBlock);

    if (byteToWrite == 0) {                 // 전송이 완료되었을 때(이제 더이상 남은 파일 크기가 없을 때)
        file->close();
        delete file;
    }
}



void MainServer::sendFile()
{
    qDebug() << saveFileData;


    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    //QString filename = id; //여기까지함

    //요기 이상한거같음!!!
    qDebug() << "((((((currentPID" << currentPID;    //ex. P00003
    QDir dir(QString("./Image/%1").arg(currentPID));
    QStringList filters;
    filters << "*.png" << "*.jpg" << "*.bmp";
    QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);
    qDebug() << "폴더 안의 전체 파일 개수-fileInfoList.count(): " << fileInfoList.count();



    //QDataStream out(&outBlock, QIODevice::WriteOnly);

    //보내는 형식: 파일개수<CR>파일명<f>파일크기<n>파일명<f>파일크기<n>|파일정보1파일정보2...
    QByteArray allFile;
    allFile.append(QString::number(fileInfoList.count()).toStdString());
    allFile.append("<CR>");


    QByteArray tempArray;

    for(int i = 0; i < fileInfoList.count(); i++)
    {
        QString currentFileName = dir.path() + "/" + fileInfoList.at(i).fileName();
        qDebug() << currentFileName;

        file = new QFile(currentFileName);
        file->open(QFile::ReadOnly);

        //파일명과 파일크기 전송
        QString fileString = fileInfoList.at(i).fileName() + "<f>"
                + QString::number(file->size()) + "<n>";

        allFile.append(fileString.toStdString());

        tempArray.append(file->readAll());
        qDebug() << "tempSize " << file->size();
    }
    allFile.append("|");
    allFile.append(tempArray);  // 파일정보 전송
    allFile.append("<FIN>");

    qDebug() << "allFile.size()" << allFile.size();


    //    QDataStream out(&sendAllFile, QIODevice::WriteOnly);
    //    out << allFile.size() << allFile;
    if(sendFileFlag == 0)
        pmsFileSocket->write(allFile);
    else if(sendFileFlag == 1)
        viewerFileSocket->write(allFile);

}

//    for(int i = 0; i < fileInfoList.count(); i++)
//    {
//        QString currentFileName = dir.path() + "/" + fileInfoList.at(i).fileName();
//        qDebug() << currentFileName;

//        file = new QFile(currentFileName);
//        file->open(QFile::ReadOnly);



//        allFile.append(file->readAll());

//        allFile.append("<NEXT>");
//        qDebug() << "'''''''''''''allFile Size: " << allFile.size();

//    }

//    while(1)
//    {
//        goOnSend(40);

//        if(byteToWrite <= 0)
//            return;
//    }





//        qDebug() << "***********************byteToWrite: " <<byteToWrite;
//        totalSize += byteToWrite;

//        out << qint64(0) << qint64(0) << currentFileName;


//        totalSize += outBlock.size();    //첫번째 qint;
//        byteToWrite += outBlock.size();  //두번째 qint;

//        out.device()->seek(0);
//        out << totalSize << qint64(outBlock.size());
//    }

//    qDebug() << "totalSize" << totalSize;










//        byteToWrite = totalSize = file->size();
//        loadSize = 1024;

//        QDataStream out(&outBlock, QIODevice::WriteOnly);

//        out << qint64(0) << qint64(0) << currentFileName;
//        totalSize += outBlock.size();
//        byteToWrite += outBlock.size();

//        out.device()->seek(0);
//        out << totalSize << qint64(outBlock.size());

//        if(sendFileFlag==0){
//            pmsFileSocket->write(outBlock); // Send the read file to the socket    //서버로 보내줌
//        }
//        //정연
//        else if(sendFileFlag==1)
//            viewerFileSocket->write(outBlock);

//        qDebug() << QString("Sending file %1").arg(currentFileName);
//    }





//    if(currentPID.length()) {
//        file = new QFile(QString("./Image/%1/%2").arg(currentPID.first(6)).arg(currentPID));
//        qDebug() << "SSSSSSSSS" << file->fileName();
//        file->open(QFile::ReadOnly);

//        qDebug() << QString("file %1 is opened").arg(currentPID);

//        qDebug() <<"@@@@@@@@file->size(): " << file->size();

//        byteToWrite = totalSize = file->size(); // Data remained yet

//        loadSize = 1024; // Size of data per a block

//        QDataStream out(&outBlock, QIODevice::WriteOnly);

//        out << qint64(0) << qint64(0) << currentPID;
//qDebug() << outBlock.size();
//        totalSize += outBlock.size();
//        byteToWrite += outBlock.size();

//        out.device()->seek(0);
//        out << totalSize << qint64(outBlock.size());

//        if(sendFileFlag==0){
//            pmsFileSocket->write(outBlock); // Send the read file to the socket    //서버로 보내줌
//        }
//            //
//        else if(sendFileFlag==1)
//            viewerFileSocket->write(outBlock);

//    }
//qDebug() << QString("Sending file %1").arg(currentPID);






void MainServer::socketDisconnected()
{
    qDebug() <<"????";

    QTcpSocket *socket = dynamic_cast<QTcpSocket*>(sender());
    if(socket == viewerSocket)
        viewerSocket=nullptr;
    else if(socket == pmsSocket)
        pmsSocket=nullptr;
    else if(socket == imagingSocket)
        imagingSocket=nullptr;



//    QTcpSocket *socket = static_cast<QTcpSocket*>(sender());
//    QByteArray *buffer = buffers.value(socket);
//    qint32 *s = sizes.value(socket);
//    socket->deleteLater();
//    delete buffer;
//    delete s;
}

bool MainServer::writeData(QByteArray data)
{
    QTcpSocket* socket = (QTcpSocket*)(sender());
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        socket->write(data); // 데이터를 보내줌
        return socket->waitForBytesWritten();
    }
    else
    {
        return false;
    }
}

void MainServer::sendDataToClient(QString newData)
{
    QString sendData = newData;
    send_flag = writeData(sendData.toStdString().c_str()); //writeData의 첫 번째 인자는 char *data와 같은 형식임
    if(!send_flag)
        qDebug() << "Socket send fail\n";
}

void MainServer::receiveData()
{
    qDebug() << "%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%";
    QTcpSocket* socket = dynamic_cast<QTcpSocket*>(sender());



    QByteArray receiveData = socket->readAll();

    saveData = QString(receiveData);
    ui->textEdit->insertPlainText(saveData);
    ui->textEdit->insertPlainText("\n");

    qDebug() << "savedata: " << saveData;

    if(saveData.contains("<CR>", Qt::CaseInsensitive) == true)
    {

        qDebug() << "받은 데이터: "<<saveData;

        //어떤 이벤트인지에 따라 불러올 함수 써주기(각각 이벤트에 대한 함수 만들고 if-else문 타도록 만들자)
        QString event = saveData.split("<CR>")[0];



        if(saveData.contains("^", Qt::CaseInsensitive) == true)
        {
            qDebug() << "있다";
            event = event.split("^")[1];
        }

        else
        {
            qDebug() << "없다";
            return;
        }

        qDebug() << "확인!";









        QString id = saveData.split("<CR>")[1];
        QString data = saveData.split("<CR>")[2];

        qDebug() << "이벤트: " << event;

        if(event == "CNT"){

            /*어떤 모듈과 연관이 있는 소켓인지 알 수 있도록 map에 연결해 저장하는 부분*/
            if(id == "PMS")
            {
                sk.insert(pmsSocket, "PMS");
                pmsSocket = socket;
                qDebug() << "pmsSocket ready";
                qDebug() << "pmsSocket save: " << saveData;


                sendWaitingList(pmsSocket);


            }
            else if(id == "IMG")
            {
                sk.insert(imagingSocket, "IMG");
                imagingSocket = dynamic_cast<QTcpSocket*>(sender());
                qDebug() << "imagingSocket ready";

                sendWaitingList(imagingSocket);

                //뷰어가 연결되어 있었던 상태라면, 뷰어는 촬영SW가 커넥트 될 때마다 촬영요청을 수행하기 위해 연결되었다는 메세지를 받아야 함
                if(viewerSocket!=nullptr)
                {
                    QString tempData = saveData.split("^")[1];
                    QString sendData = "ACK^" + tempData;
                    viewerSocket->write(sendData.toStdString().c_str());
                }
            }
            else if(id == "VEW")
            {
                sk.insert(viewerSocket, "VEW");
                viewerSocket = dynamic_cast<QTcpSocket*>(sender());
                qDebug() << "viewerSocket ready";

                sendWaitingList(viewerSocket);
            }

        }
        else if(event == "VLG") //뷰어 로그인
        {

            qDebug()<<"Login Data: "<<id << ", " <<data;
            QModelIndexList indexes = dentistModel->match(dentistModel->index(0, 0), Qt::EditRole, id,
                                                          -1, Qt::MatchFlags(Qt::MatchCaseSensitive));
            //            QString sendData = "VLG<CR>";

            QString sendData;

            qDebug() << indexes;
            QString name;
            foreach(auto ix, indexes) {
                /*검색창에서 입력한 값으로 찾은 데이터를 통해 고객 데이터들을 반환*/
                name = dentistModel->data(ix.siblingAtColumn(1)).toString();
            }
            qDebug() << name;
            if(name == data)
            {
                sendData = "ACK^VLG<CR>";
                sendData = sendData +id + "<CR>True";
            }
            else
            {
                sendData = "ACK^VLG<CR>";
                sendData = sendData +id + "<CR>False";
            }

            qDebug() << "로그인 sendData: " << sendData;
            viewerSocket->write(sendData.toStdString().c_str());

        }
        /*환자관리 SW 이벤트*/
        else if(event == "PER")      //신규환자 등록 끝났을 때: PER(enroll)
        {
            query->prepare("INSERT INTO patient (patient_no, patient_name, patient_sex, patient_birthdate,"
                           "patient_tel, patient_address, patient_memo)"
                           "VALUES(:patient_no, :patient_name, :patient_sex, :patient_birthdate,"
                           ":patient_tel, :patient_address, :patient_memo)");


            query->bindValue(":patient_no", id);
            query->bindValue(":patient_name", data.split("|")[0]);
            query->bindValue(":patient_sex", data.split("|")[1]);
            query->bindValue(":patient_birthdate", data.split("|")[2]);
            query->bindValue(":patient_tel", data.split("|")[3]);
            query->bindValue(":patient_address", data.split("|")[4]);
            query->bindValue(":patient_memo", data.split("|")[5]);
            query->exec();

            qDebug()<<"새로운 환자 정보 저장 완료";
            patientModel->select();


            //저장 잘 되었다고 pms에 알려주기
            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;
            pmsSocket->write(sendData.toStdString().c_str());



        }
        else if(event == "PID")     //신규환자 PID 요청: PID
        {
            QString newPID = makeId();
            qDebug() << "newPID: " << newPID;
            QString sendData = "ACK^PID<CR>" + newPID + "<CR>";

            //이거 고치기 socket->write(sendData.toStdString().c_str());
            pmsSocket->write(sendData.toStdString().c_str());
            //sendDataToClient(sendData);

            //emit sendNewPID(newPID);
        }
        else if(event == "PDE")     //환자 정보 삭제: PDE(delete)
        {
            qDebug()<<"?????????";

            //이미지 폴더의 pid 폴더 삭제
            QDir dir(QString("./Image/%1").arg(id));
            qDebug() << dir.dirName();
            dir.removeRecursively();

            query->exec("delete from patient WHERE patient_no = '" + id + "'");
            patientModel->select();








            //image 테이블에서 지우려는 환자 이미지 정보도 함께 삭제
            query3->exec("select * from image WHERE patient_no = '" + id +"'");
            QSqlRecord imageRec =query3->record();
            qDebug()<<"Number of columns: "<<imageRec.count();

            QString dentistID, dentistName;
            while(query3->next())
                query3->exec("delete from image WHERE patient_no = '" + id + "'");






            //잘 삭제되었다고 pms에 알려주기
            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;
            pmsSocket->write(sendData.toStdString().c_str());




            imageModel->select();

        }
        else if(event == "PSE")     //검색: PSE(search)         //DB에 없는 환자 검색했을 때 죽는 거 예외처리 해야 함
        {
            qDebug() << "savedata: " << saveData;

            qDebug() << data;
            //            QString reportData ="<NEL>";
            //            query4->exec("select * from report WHERE patient_no = '"+data +"'");
            //            QSqlRecord reportRec =query4->record();
            //            qDebug()<<"Number of columns: "<<reportRec.count();
            //            qDebug() << "report value: " << query4->value(3);


            //            while(query4->next())
            //            {
            //                for(int i=0;i<reportRec.count();i++)
            //                {
            //                    //qDebug()<<"report i: "<<i <<"report data: "<<query4->value(i).toString();//output all names
            //                    QString tmpData = query4->value(i).toString()+"|";
            //                    reportData +=tmpData;
            //                    qDebug()<<"reportData : "<<reportData ;

            //                }
            //                query4->nextResult();
            //                reportData += "<NEL>";
            //            }



            QString sendData ="ACK^PSE<CR>";

            if(id == "0"){      //환자번호로 검색했을 때
                sendData = sendData + data + "<CR>";


                currentPID = data;

                query->exec("select * from patient WHERE patient_no = '" + data + "'");
                QSqlRecord rec = query->record();
                qDebug() << "Number of columns: " << rec.count();




                while (query->next())
                {
                    for(int i=1; i<rec.count() ; i++)
                    {
                        qDebug() << "i: " << i << "data: " << query->value(i).toString(); // output all names
                        QString data = query->value(i).toString() + "|";
                        sendData += data;
                        qDebug() << "sendData: " << sendData;
                    }
                }


                QString reportData ="<NEL>";
                query4->exec("select * from report WHERE patient_no = '"+data +"'");
                QSqlRecord reportRec =query4->record();
                qDebug()<<"Number of columns: "<<reportRec.count();
                qDebug() << "report value: " << query4->value(3);

                QString dentistID, dentistName;
                while(query4->next())
                {
                    for(int i=0;i<reportRec.count();i++)
                    {

                        if(i==2)
                        {

                            dentistID = query4->value(i).toString();
                            qDebug()<<"doctorID: " << dentistID;
                        }

                        //qDebug()<<"report i: "<<i <<"report data: "<<query4->value(i).toString();//output all names
                        QString tmpData = query4->value(i).toString()+"|";
                        reportData +=tmpData;
                        qDebug()<<"reportData : "<<reportData ;

                    }

                    query2->exec("select * from dentist WHERE dentist_no = '"+ dentistID +"'");
                    while(query2->next())
                    {
                        dentistName = query2->value(1).toString();
                        qDebug() << "Dentist Name: " <<dentistName;
                    }


                    query4->nextResult();
                    reportData += dentistName + "<NEL>";
                }

                sendData += reportData;

            }
            else if(id == "1"){     //환자이름으로 검색했을 때
                QString pid;
                qDebug() <<"왜 데이터 안나왕 data:" << data;
                query->exec("select * from patient WHERE patient_name = '" + data + "'");

                if(query->first()==false)
                    sendData+="NULL<CR>";


                qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                query->exec("select * from patient WHERE patient_name = '" + data + "'");
                //QSqlRecord rec = query->record();

                qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);

                int tempcount =0;
                while (query->next()){
                    tempcount ++;
                    qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                }

                qDebug() << "tempcount: " <<tempcount;  //3개 나왔다!!!!!!!!!!
                qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);

                if(tempcount>1)
                {
                    query->exec("select * from patient WHERE patient_name = '" + data + "'");
                    QSqlRecord tempRec = query->record();
                    QString tempSendData = "ACK^PSE<CR>" + QString::number(tempcount) + "<CR>";

                    //이 세 명에 대한 정보들 쭉 모아서 보내줌 ex. PSE<CR>검색된 환자수<CR>PID|이름|전화번호(인덱스4번)<r>PID|이름|전화번호(인덱스4번)<r>.....
                    while (query->next()){//검색된 환자 수만큼 검색
                        //for(int i=0;i<tempcount;i++)
                        //{
                        QString tempPID = query->value(0).toString();
                        QString tempPhoneNum = query->value(4).toString();

                        tempSendData += tempPID + "|" + data + "|" + tempPhoneNum + "<r>";


                        //}
                    }

                    qDebug() << "tempSendData: " << tempSendData;
                    pmsSocket->write(tempSendData.toStdString().c_str());



                    return;
                }


qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                query->exec("select * from patient WHERE patient_name = '" + data + "'");
                QSqlRecord rec = query->record();
                while (query->next()){
                    for(int i = 0; i<rec.count() ; i++)
                    {
                        qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                        if(i == 0)
                        {
                            qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                            qDebug() << "i: " << i << "data: " << query->value(i).toString();
                            qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                            QString data = query->value(i).toString();
                            pid = data; //pid만 저장해줌
qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                            data += "<CR>"; //구분자 붙임
                            sendData += data;
                            qDebug() << "sendData: " << sendData;


                        }
                        else
                        {
                            qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                            qDebug() << "i: " << i << "data: " << query->value(i).toString();
                            qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                            QString data = query->value(i).toString() + "|";
                            sendData += data;
                            qDebug() << "sendData: " << sendData;
                        }
                    }
                }

                qDebug("zzzzzzzzzzzzzzzzzz%d ", __LINE__);
                qDebug() << "sendData: " << sendData;

                qDebug()<<"pid: "<<pid;
                currentPID = pid;

                QString reportData ="<NEL>";
                query4->exec("select * from report WHERE patient_no = '"+ pid +"'");
                QSqlRecord reportRec =query4->record();

                QString dentistID, dentistName;
                while(query4->next())
                {

                    for(int i=0;i<reportRec.count();i++)
                    {
                        if(i==2)
                        {
                            dentistID = query4->value(i).toString();
                            qDebug()<<"doctorID: " << dentistID;
                        }

                        //qDebug()<<"report i: "<<i <<"report data: "<<query4->value(i).toString();//output all names
                        QString tmpData = query4->value(i).toString()+"|";
                        reportData +=tmpData;
                        qDebug()<<"reportData : "<<reportData ;

                    }

                    query2->exec("select * from dentist WHERE dentist_no = '"+ dentistID +"'");
                    while(query2->next())
                    {
                        dentistName = query2->value(1).toString();
                        qDebug() << "Dentist Name: " <<dentistName;
                    }

                    query4->nextResult();
                    reportData += dentistName + "<NEL>";
                }
                sendData += reportData;

            }

            sendFileFlag = 0;
            sendFile();

            qDebug() << "PSE's sendData: " << sendData;
            pmsSocket->write(sendData.toStdString().c_str());







            //this->loadData();
        }
        else if(event == "PMO")     //환자 정보 수정: PMO(modify)
        {
            qDebug() << "PMO's saveData: " << saveData;

            QString name = data.split("|")[0];
            QString sex = data.split("|")[1];
            QString birthdate = data.split("|")[2];
            QString tel = data.split("|")[3];
            QString address = data.split("|")[4];
            QString memo = data.split("|")[5];


            query->exec("update patient set patient_name = '" + name + "', patient_sex = '" + sex +
                        "', patient_birthdate = '" + birthdate + "', patient_tel = '" + tel +
                        "', patient_address = '" + address + "', patient_memo = '" + memo +
                        "'where patient_no = '" + id + "'");

            patientModel->select();


            //수정 잘 되었다고 pms에 알려주기
            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;
            pmsSocket->write(sendData.toStdString().c_str());


        }
        //필요없는듯
        //        else if(event == "PFN")     //수납 처리: PFN(finish)           @@@@@@@@@뷰어와 확인필요@@@@@@@@
        //        {
        //            qDebug() << "PFN's saveData: " << saveData;
        //            pmsSocket->write(saveData.toStdString().c_str());
        //        }



        else if(event == "AWL")     //대기 환자 추가: AWL(Add to Wait list)   //환자정보에서는 내부적으로 대기목록에 추가됨. 뷰어로만 보내주면 될 듯(pid랑 이름만).
            //=> 환자SW에서 대기 버튼 눌렀다는 정보를 서버에서 받고 해당 환자에 대한 여러가지 정보들을 촬영/뷰어SW로 보냄
            //위에 있는 기능은 진료시작/촬영시작에 해당하는 기능인 걸로 결정했음. 여기서는 pid랑 이름만 주기로. 딱 대기목록에 추가될 정보만 보내자
        {





            qDebug()<< "받은 진료대기환자 정보: " << saveData;
            qDebug() << "뷰어쪽으로 보내줄 진료대기환자 정보(촬영SW에는 안 보내줌): " + event + "<CR>" + id + "<CR>" + data.split("|")[0];
            //socket->write(saveData.toStdString().c_str());  //이 정보는 촬영SW와 뷰어SW쪽으로 보내져야 함.(지금 써져있는 socket은 임시..
            QString sendWaitData = "ACK^" + event + "<CR>" + id + "<CR>" + data.split("|")[0];


            //뷰어가 켜져있지 않을 때
            if(viewerSocket == nullptr)
            {
                QString sendData = "ERR^AWL<CR>" + id + "<CR>" + data.split("|")[0];
                pmsSocket->write(sendData.toStdString().c_str());

                //서버가 죽지 않도록 예외처리
                return;
            }


            //pms소켓에 뷰어에도 잘 추가되었다고 ACK보내줌
            pmsSocket->write(sendWaitData.toStdString().c_str());




            //**********여기는 정연이 뷰어SW가 켜져있을 때 다시 주석 풀기************ => 이제 주석 안써도 될 듯함!
            //qDebug() << "정연이 소켓 있는지 확인: " << viewerSocket->isValid();
            viewerSocket->write(sendWaitData.toStdString().c_str());







            //진료대기 환자 추가 시 대기리스트 파일에 저장
            QFile waitingList("waitingList.txt");
            if (!waitingList.open(QIODevice::WriteOnly | QIODevice::Append))
                return;

            QTextStream out(&waitingList);                 //파일이 있을 때

            out << id << "," <<"WT" << "\n";
            qDebug() << waitingList.pos();


            waitingList.close( );





        }

        /*촬영 SW 이벤트*/
        else if(event == "IPR")     //환자 준비: IPR(patient ready) [받는 정보: 이벤트, ID / 보낼 정보: 이벤트, ID, 이름, 생년월일, 성별]
        {

            QString sendData ="ACK^IPR<CR>";
            sendData = sendData + id + "<CR>";


            query->exec("select * from patient where patient_no = '" + id + "'");

            QSqlRecord rec = query->record();

            qDebug() << "Number of columns: " << rec.count();

            //와일문보기
            while (query->next()){

                for(int i = 1; i<4 ; i++)
                {
                    qDebug() << "i: " << i << "data: " << query->value(i).toString();
                    QString data = query->value(i).toString() + "|";
                    sendData += data;
                    qDebug() << "sendData: " << sendData;
                }
            }

            //미로오빠소켓
            socket->write(sendData.toStdString().c_str());




            //            imagingSocket->write(sendData.toStdString().c_str());

        }
        //파일소켓으로는 자동으로 이미지가 전송되고 받아지고 할 거고 ISV는 파일이 서버로 보내졌다는 사실만을 알려주는 이벤트임
        else if(event == "ISV")     //저장 및 전송: ISV(save) [받을 정보, 보낼 정보 동일: 이미지 No, 환자 ID, 이름, 촬영 타입] - imaging module에서 클릭될 시에 다른 모듈에서는 진료대기로 바뀜
        {


            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;




            //촬영SW쪽에도 메인서버에 잘 저장되었다고 ACK 보내기
            imagingSocket->write(sendData.toStdString().c_str());


            qDebug() << "ISV's saveData: " << saveData;




            /*확인필요!!*/
            //촬영중인 상태인 환자가 촬영완료 되었을 때 환자를 대기중으로 변경
            QFile oldList("waitingList.txt");
            oldList.open(QIODevice::Text | QIODevice::ReadOnly);
            QString dataText = oldList.readAll();
            qDebug() <<"dataText: "<<dataText;



qDebug("xxxxxxxxxxxxxxxxx %d",__LINE__);
            QString tempStatus = data;
            qDebug("xxxxxxxxxxxxxxxxx %d",__LINE__);
            if( tempStatus == "CEPH")
                tempStatus = "CE";
            else if(tempStatus=="PANO")
                tempStatus = "PA";
            else if(tempStatus=="BOTH")
                tempStatus = "BO";
qDebug("xxxxxxxxxxxxxxxxx %d",__LINE__);
            QString changeNeededText = id + "," + tempStatus;
            QString changeText = id + "," + "WT";

            QRegularExpression re(changeNeededText);
            QString replacementText(changeText);
qDebug("xxxxxxxxxxxxxxxxx %d",__LINE__);
            dataText.replace(re, replacementText);
            qDebug() <<"dataText: "<<dataText;
            QFile newList("waitingList.txt");
            if(newList.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream out(&newList);
                out << dataText;
            }
            newList.close();

qDebug("xxxxxxxxxxxxxxxxx %d",__LINE__);

            //pms나 뷰어가 켜져있지 않을 때
            if(pmsSocket == nullptr || viewerSocket == nullptr)
            {
                //서버가 죽지 않도록 예외처리
                return;
            }
            else
            {
                //촬영이 끝났음을 pms와 뷰어에 알려주기
                pmsSocket->write(sendData.toStdString().c_str());
                viewerSocket->write(sendData.toStdString().c_str());
            }


        }
        else if(event == "MWL")     //진료중이었던 환자 상태를 진료대기로 변경
        {
            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;




            //진료중에서 진료대기로 환자상태를 변경
            QFile oldList("waitingList.txt");
            oldList.open(QIODevice::Text | QIODevice::ReadOnly);
            QString dataText = oldList.readAll();

            //파일내용 변경해줘야 함
            QString changeNeededText = id + "," + "TM";
            QString changeText = id + "," + "WT";
            qDebug() << "@@@@@@@@changeNeededText: " << changeNeededText << " / changeText: " <<changeText;
            QRegularExpression re(changeNeededText);
            QString replacementText(changeText);

            dataText.replace(re, replacementText);
            qDebug() <<"dataText: "<<dataText;
            QFile newList("waitingList.txt");
            if(newList.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream out(&newList);
                out << dataText;
            }
            newList.close();



            //pms연결되어있으면 메세지 보내줌
            if(pmsSocket == nullptr)
                return;         //서버 죽지 않도록 예외처리
            else
                pmsSocket->write(sendData.toStdString().c_str());   //메세지 보내주는 부분

        }


        /*영상 뷰어 SW 이벤트*/
        else if(event == "VNT")     //처방전 작성: VNT (write note)
            //받을 정보: SEN^VNT<CR>PID<CR>환자이름|의사번호|의사이름|진료날짜|진료내용(처방내용)
            //보낼 정보: ACK^VNT<CR>PID<CR>진료차트 번호(이거는 내가 계산)|환자이름|의사번호|의사이름|진료날짜|진료내용(처방내용)
        {
            qDebug()<< saveData;

            query4->prepare("INSERT INTO report (report_no, patient_no, dentist_no, report_date, report_note)"
                            "VALUES(:report_no, :patient_no, :dentist_no, :report_date, :report_note)");

            query4->bindValue(":report_no", makeReportNo());    //오류있는듯 P00001 대신 R00001대야함
            query4->bindValue(":patient_no", id);
            query4->bindValue(":dentist_no", data.split("|")[1]);
            query4->bindValue(":report_date", data.split("|")[3]);
            query4->bindValue(":report_note", data.split("|")[4]);
            query4->exec();

            qDebug()<<"새로운 진료기록 정보 저장 완료";

            reportModel->select();




            /*DB에 잘 저장되었다고 뷰어로 ACK보내주는 부분*/
            //save데이터에서 이벤트 부분부터 잘라내서 저장
            QString tempData = saveData.toStdString().c_str();
            tempData = tempData.split("^")[1];
            QString sendData = "ACK^" + tempData;

            viewerSocket->write(sendData.toStdString().c_str());



            //pms가 켜져있으면
            if(pmsSocket !=nullptr)
            {
                pmsSocket->write(sendData.toStdString().c_str());   //새로운 진료기록 전송
            }
            else    //꺼져있으면 서버 죽지 않도록 만듦
                return;



        }
        else if(event == "VTS")     //진료 시작: VTS(treatment start)
            //[받을 정보: 이벤트, pid / 보낼 정보: 이벤트, pid, 이름, 성별, 생년월일, 메모]
        {


            qDebug() << "정연이 데이터 온 거: " << saveData;

            QString sendData ="ACK^VTS<CR>";
            sendData = sendData + id + "<CR>";

            qDebug() << "내가 가지고 있는 id 데이터: " << id;
            currentPID = id;

            query->exec("select * from patient where patient_no = '" + id + "'");

            QSqlRecord rec = query->record();

            qDebug() << "Number of columns: " << rec.count();

            //와일문보기
            while (query->next()){

                for(int i = 1; i<4 ; i++)
                {
                    qDebug() << "i: " << i << "data: " << query->value(i).toString();
                    QString data = query->value(i).toString() + "|";
                    sendData += data;
                    qDebug() << "sendData: " << sendData;

                    //
                }
                sendData += query->value(6).toString();

            }

            qDebug() << "sendData: " << sendData;

            viewerSocket->write(sendData.toStdString().c_str());

            //정연이쪽에 파일보내줌
            sendFileFlag = 1;
            sendFile();


            /*확인필요!!*/
            //진료대기 상태에서 진료가 시작되었을 때 환자상태를 진료중으로 변경
            QFile oldList("waitingList.txt");
            oldList.open(QIODevice::Text | QIODevice::ReadOnly);
            QString dataText = oldList.readAll();
            qDebug() <<"dataText: "<<dataText;




            //환자 이름은 NULL인 상태로 오므로 DB에서 검색해 변경해 줄 것
            query->exec("select * from patient WHERE patient_no = '" + id + "'");

            QString tempName;
            while (query->next()){
                tempName = query->value(1).toString();
            }



            qDebug() << "tempName"<<tempName;



            qDebug("%d", __LINE__);
            QString changeNeededText = id + "," + "WT";
            QString changeText = id + "," + "TM";
            qDebug() << "@@@@@@@@changeNeededText: " << changeNeededText << " / changeText: " <<changeText;
            QRegularExpression re(changeNeededText);
            QString replacementText(changeText);

            dataText.replace(re, replacementText);
            qDebug() <<"dataText: "<<dataText;
            QFile newList("waitingList.txt");
            if(newList.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream out(&newList);
                out << dataText;
            }
            newList.close();






            //pms가 켜져있지 않을 때
            if(pmsSocket == nullptr)
            {
                //서버가 죽지 않도록 예외처리
                return;
            }
            else
                pmsSocket->write(sendData.toStdString().c_str());


        }
        else if(event == "VTF")     //진료 완료: VTF(treatment finish) [받을 정보: 이벤트, pid / 보낼 정보: 이벤트, pid]
        {


            /*확인필요!!*/
            //진료중 상태에서 진료가 완료되었을 때 환자상태를 수납대기로 변경
            QFile oldList("waitingList.txt");
            oldList.open(QIODevice::Text | QIODevice::ReadOnly);
            QString dataText = oldList.readAll();
            qDebug() <<"dataText: "<<dataText;


            QString changeNeededText = id + "," + "TM";
            QString changeText = id + "," + "WP";

            QRegularExpression re(changeNeededText);
            QString replacementText(changeText);

            dataText.replace(re, replacementText);
            qDebug() <<"dataText: "<<dataText;
            QFile newList("waitingList.txt");
            if(newList.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream out(&newList);
                out << dataText;
            }
            newList.close();





            //진료완료 정보 저장 잘 되었다고 뷰어에 알려주기
            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;
            viewerSocket->write(sendData.toStdString().c_str());


            //pms가 켜져있지 않을 때
            if(pmsSocket == nullptr)
            {
                //서버가 죽지 않도록 예외처리
                return;
            }
            else
            {
                sendData += "|";  //pms의 statusRequestSended함수에서 name에 해당하는 부분을 |로 나누어주기 때문에 필요한 부분
                pmsSocket->write(saveData.toStdString().c_str()); //뷰어쪽에서 받은 정보 그대로 환자관리SW에 전송=>환자관리에서는 event가 VTS일 시에 환자 진료 상태 진료중으로 변경해주면 될 듯
            }

        }

        /*촬영 요청 이벤트(환자SW(진료대기)/뷰어SW(진료중)->촬영SW)*/
        else if(event == "SRQ")     //촬영 의뢰: SRQ(shoot request)
        {
            qDebug() << "saveData: " << saveData;

            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;



            //촬영SW가 켜져있지 않을 때
            if(imagingSocket == nullptr)
            {
                //들어왔던 소켓에 바로 ERR을 보내줌
                QString sendData = "ERR^SRQ<CR>" + id + "<CR>" + data;
                socket->write(sendData.toStdString().c_str());


                //서버가 죽지 않도록 예외처리
                return;
            }
            else
                imagingSocket->write(sendData.toStdString().c_str());









            //진료대기 환자 촬영 요청 시 대기리스트 파일에서 대기중인 환자를 촬영중으로 변경
            QFile oldList("waitingList.txt");
            oldList.open(QIODevice::Text | QIODevice::ReadOnly);
            QString dataText = oldList.readAll();
            qDebug() <<"dataText: "<<dataText;

            QString shootType = data.split("|")[1];
            if(shootType=="PANO")
                shootType ="PA";
            else if(shootType=="CEPH")
                shootType ="CE";
            else if(shootType=="BOTH")
                shootType ="BO";



            //pms소켓에서 촬영의뢰 보냈을 때는 현재 상태 WT, 뷰어에서 촬영의뢰 보냈을 때는 현재 상태 WT
            if(pmsSocket == socket)
            {


            QString changeNeededText = id + "," + "WT";
            QString changeText = id + "," + shootType;


            QRegularExpression re(changeNeededText);
            QString replacementText(changeText);

            dataText.replace(re, replacementText);
            qDebug() <<"dataText: "<<dataText;
            QFile newList("waitingList.txt");
            if(newList.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream out(&newList);
                out << dataText;
            }
            newList.close();
            }
            else if(viewerSocket == socket)

            {
                QString changeNeededText = id + "," + "TM";
                QString changeText = id + "," + shootType;


                QRegularExpression re(changeNeededText);
                QString replacementText(changeText);

                dataText.replace(re, replacementText);
                qDebug() <<"dataText: "<<dataText;
                QFile newList("waitingList.txt");
                if(newList.open(QFile::WriteOnly | QFile::Truncate)) {
                    QTextStream out(&newList);
                    out << dataText;
                }
                newList.close();
            }


            //            QRegularExpressionMatchIterator itr = re.globalMatch(dataText);

            //            while(itr.hasNext())
            //            {
            //                QRegularExpressionMatch match = itr.next();
            //                dataText.replace(match.capturedStart(0), match.capturedLength(0), replacementText);
            //            }

            //            QFile currentWaitingList("waitingList.txt");
            //            if(currentWaitingList.open(QFile::WriteOnly | QFile::Truncate)) {
            //                QTextStream out(&currentWaitingList);
            //                out << dataText;
            //            }
            //            currentWaitingList.close();


            //pms가 켜져있지 않을 때
            if(pmsSocket == nullptr)
            {
                //서버가 죽지 않도록 예외처리
                return;
            }
            else
                //촬영요청이 pms에서 오든 viewer에서 오든 둘 다 촬영중으로 바뀌었다는 신호를 받아야 하기 때문에 SRQ이벤트를 서버쪽에서 다시 보내주도록 하였음
                pmsSocket->write(sendData.toStdString().c_str());



            //뷰어가 켜져있지 않을 때
            if(pmsSocket == nullptr)
            {
                //서버가 죽지 않도록 예외처리
                return;
            }
            else
                viewerSocket->write(sendData.toStdString().c_str());



        }
        else if(event == "WPY")     //WPY: Wait Payment
        {
            //pms에서 수납처리를 누를 시에 오는 이벤트. waitingList 파일에서 해당 환자 정보를 삭제해 줌
            QFile oldList("waitingList.txt");
            oldList.open(QIODevice::Text | QIODevice::ReadOnly);
            QString dataText = oldList.readAll();
            qDebug() <<"dataText: "<<dataText;
            QString changeNeededText = id + "," + "WP\n";
            QString changeText = "";


            QRegularExpression re(changeNeededText);
            QString replacementText(changeText);

            dataText.replace(re, replacementText);
            qDebug() <<"dataText: "<<dataText;
            QFile newList("waitingList.txt");
            if(newList.open(QFile::WriteOnly | QFile::Truncate)) {
                QTextStream out(&newList);
                out << dataText;
            }
            newList.close();


            //잘 수납처리 되었고 대기리스트에서 해당 정보가 잘 삭제되었음을 pms에 알려주기
            QString tempData = saveData.split("^")[1];
            QString sendData = "ACK^" + tempData;
            pmsSocket->write(sendData.toStdString().c_str());


        }



        //buffer->clear(); //버퍼 비워주기
    }
    //<CR> 들어가 있지 않으면 return
    else
        return;
}



void MainServer::loadData()
{
    /*DB를 추가하고 DB의 이름을 설정*/
    QSqlDatabase db = QSqlDatabase::addDatabase("QSQLITE", "databaseConnection");
    db.setDatabaseName("database.db");



    /*DB를 오픈해 새로운 테이블을 만듦*/
    if (db.open( )) {
        query= new QSqlQuery(db);
        //        query->exec("PRAGMA foreign_keys = ON");

        query->exec("CREATE TABLE IF NOT EXISTS patient(patient_no VARCHAR(10) Primary Key,"
                    "patient_name VARCHAR(10) NOT NULL, patient_sex VARCHAR(5) NOT NULL, patient_birthdate VARCHAR(15) NOT NULL,"
                    "patient_tel VARCHAR(15) NOT NULL, patient_address VARCHAR(60) NOT NULL, patient_memo VARCHAR(100));");
        patientModel = new QSqlTableModel(this, db);
        patientModel->setTable("patient");
        patientModel->select();
        patientModel->setHeaderData(0, Qt::Horizontal, tr("No"));
        patientModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        patientModel->setHeaderData(2, Qt::Horizontal, tr("Sex"));
        patientModel->setHeaderData(3, Qt::Horizontal, tr("Birthdate"));
        patientModel->setHeaderData(4, Qt::Horizontal, tr("Telephone Number"));
        patientModel->setHeaderData(5, Qt::Horizontal, tr("Address"));
        patientModel->setHeaderData(6, Qt::Horizontal, tr("Memo"));
        ui->patientTableView->setModel(patientModel);
        //query->exec("INSERT INTO patient VALUES ('P00001', '김유선', '여성', '2023-02-06', '010-1111-1111', 'ㅈㅅ', 'ㅁㅁ')");
        //query->exec("INSERT INTO patient VALUES ('P00002', '김도예', '여성', '2023-02-06','010-2222-2222', 'ㅈㅅ', 'ㅁㅁ')");
        //query->exec("INSERT INTO patient VALUES ('P00003', '한성은', '여성', '2023-02-06','010-3333-3333', 'ㅈㅅ', 'ㅁㅁ')");
        patientModel->select();



        query2 = new QSqlQuery(db);
        query2->exec("CREATE TABLE IF NOT EXISTS dentist(dentist_no VARCHAR(10) Primary Key,"
                     "dentist_name VARCHAR(10) NOT NULL, dentist_sex VARCHAR(5) NOT NULL, dentist_tel VARCHAR(15) NOT NULL);");

        dentistModel = new QSqlTableModel(this, db);
        dentistModel->setTable("dentist");
        dentistModel->select();
        dentistModel->setHeaderData(0, Qt::Horizontal, tr("No"));
        dentistModel->setHeaderData(1, Qt::Horizontal, tr("Name"));
        dentistModel->setHeaderData(2, Qt::Horizontal, tr("Sex"));
        dentistModel->setHeaderData(3, Qt::Horizontal, tr("Telephone Number"));
        ui->dentistTableView->setModel(dentistModel);
        //의사 정보는 수정삭제 불가능하게 만들어놨음. 고정된 정보
        query2->exec("INSERT INTO dentist VALUES ('D00001', '이정연', '여성', '010-1234-5678')");
        query2->exec("INSERT INTO dentist VALUES ('D00002', '안다미로', '남성', '010-8765-4321')");
        query2->exec("INSERT INTO dentist VALUES ('D00003', '박병규', '남성', '010-3456-7890')");
        query2->exec("INSERT INTO dentist VALUES ('1', '1', '남성', '010-3456-7890')");

        dentistModel->select();



        query3= new QSqlQuery(db);
        //        query3->exec("PRAGMA foreign_keys = ON");
        //        query3->exec("CREATE TABLE IF NOT EXISTS image(image_no VARCHAR(10) Primary Key, patient_no VARCHAR(10),"
        //                     "type VARCHAR(10) NOT NULL, image_date VARCHAR(15) NOT NULL, image_path varchar(300) NOT NULL, Foreign Key (patient_no) REFERENCES patient(patient_no) ON DELETE CASCADE);");
        query3->exec("CREATE TABLE IF NOT EXISTS image(image_no VARCHAR(10) Primary Key, patient_no VARCHAR(10) NOT NULL,"
                     "type VARCHAR(10) NOT NULL, image_date VARCHAR(15) NOT NULL, image_path varchar(300) NOT NULL);");
        imageModel = new QSqlTableModel(this, db);
        imageModel->setTable("image");

        imageModel->setHeaderData(0, Qt::Horizontal, tr("Image No"));
        imageModel->setHeaderData(1, Qt::Horizontal, tr("Patient No"));
        imageModel->setHeaderData(2, Qt::Horizontal, tr("Type"));
        imageModel->setHeaderData(3, Qt::Horizontal, tr("Image Date"));
        imageModel->setHeaderData(4, Qt::Horizontal, tr("Image Path"));
        ui->imageTableView->setModel(imageModel);
        query3->exec("INSERT INTO image VALUES ('I00001', 'P00002', 'CEPH', '20230205', './Image/P00002/20230205_CEPH.bmp')");
        query3->exec("INSERT INTO image VALUES ('I00002', 'P00002', 'PANO', '20230205', './Image/P00002/20230205_PANO.bmp')");
        query3->exec("INSERT INTO image VALUES ('I00003', 'P00001', 'PANO', '20230205', './Image/P00002/20230205_PANO.bmp')");

        imageModel->select();


        query4= new QSqlQuery(db);
        query4->exec("CREATE TABLE IF NOT EXISTS report(report_no VARCHAR(10) Primary Key, patient_no VARCHAR(10) NOT NULL,"
                     "dentist_no VARCHAR(10) NOT NULL, report_date VARCHAR(15) NOT NULL, report_note VARCHAR(500) NOT NULL);");
        reportModel = new QSqlTableModel(this, db);
        reportModel->setTable("report");
        reportModel->select();
        reportModel->setHeaderData(0, Qt::Horizontal, tr("Report No"));
        reportModel->setHeaderData(1, Qt::Horizontal, tr("Patient No"));
        reportModel->setHeaderData(2, Qt::Horizontal, tr("Dentist No"));
        reportModel->setHeaderData(3, Qt::Horizontal, tr("Report Date"));
        reportModel->setHeaderData(4, Qt::Horizontal, tr("Report Note"));
        ui->reportTableView->setModel(reportModel);

        /*임시로 데이터 넣어둔 것. 나중에 지워도 무관*/
        query4->exec("INSERT INTO report VALUES ('R00001', 'P00001', 'D00002', '2023-01-19', '19일 처방전')");
        query4->exec("INSERT INTO report VALUES ('R00002', 'P00001', 'D00002', '2023-01-20', '20일 처방전')");
        query4->exec("INSERT INTO report VALUES ('R00003', 'P00002', 'D00003', '2023-01-21', '21일 처방전')");
        reportModel->select();



    }
}


QString MainServer::makeId( )
{
    int id;

    qDebug()<< "rowCount: " << patientModel->rowCount();

    if(patientModel->rowCount() == 0) {
        id = 1;
        return "P" + QString::number(id).rightJustified(5,'0');
    } else {
        int tempPid = patientModel->itemData(patientModel->index(patientModel->rowCount() - 1,0)).value(0).toString().right(5).toInt()+1; //마지막 row의 pid+1 값을 리턴
        return "P" + QString::number(tempPid).rightJustified(5,'0');
    }
}

QString MainServer::makeReportNo()
{
    int id;

    qDebug()<< "reportModel rowCount: " << reportModel->rowCount();

    if(reportModel->rowCount() == 0) {
        id = 1;
        return "R" + QString::number(id).rightJustified(5,'0');
    } else {
        int tempReportNo= reportModel->itemData(reportModel->index(reportModel->rowCount() - 1,0)).value(0).toString().right(5).toInt()+1; //마지막 row의 pid+1 값을 리턴
        return "R" + QString::number(tempReportNo).rightJustified(5,'0');
    }
}


QString MainServer::makeImageNo()
{
    int id;

    qDebug()<< "imageModel rowCount: " << imageModel->rowCount();

    if(imageModel->rowCount() == 0) {
        id = 1;
        return "I" + QString::number(id).rightJustified(5,'0');
    } else {
        int tempImageNo= imageModel->itemData(imageModel->index(imageModel->rowCount() - 1,0)).value(0).toString().right(5).toInt()+1; //마지막 row의 pid+1 값을 리턴
        return "I" + QString::number(tempImageNo).rightJustified(5,'0');
    }
}



//그냥 함수 지우고 저 select문만 다시 써주면 더 나을 듯. 나중에 기능구현 다 하구 지울 것!
void MainServer::updateRecentData()
{
    //    patientModel->select();
}


//확인 필요!!
void MainServer::sendWaitingList(QTcpSocket* specSocket)
{

    //대기목록 파일의 정보를 모듈로 보내주는 부분
    QFile oldList("waitingList.txt");

    qDebug() << oldList.exists();
    //waitingList.txt 파일이 존재하지 않는다면 대기리스트가 없는 것이므로 return 해준다
    if(oldList.exists() == false)
        return;


    oldList.open(QIODevice::Text | QIODevice::ReadOnly);
    QString dataText = oldList.readAll();
    qDebug() << "dataText"<<dataText;


    int waitLineCount = dataText.count(QLatin1Char('\n'));

    QString sendData = "ACK^WTR<CR>";


    //PMS에는 모든 정보를 보내줌
    if(specSocket == pmsSocket)
    {
        sendData += QString::number(waitLineCount) + "<CR>";


        for(int i=0; i<waitLineCount; i++)
        {
            QString tempLine = dataText.split("\n")[i];
            qDebug() << "tempLine" <<tempLine;


            //여기서 일단 pid를 이용해 이름 찾고 두번째 값인 현재상태도 붙여줌
            QString tempPID = tempLine.split(",")[0];

            //환자 이름은 없는 상태이므로 DB에서 검색해 같이 보내줄 것
            query->exec("select * from patient WHERE patient_no = '" + tempPID + "'");
            QString tempName;
            while (query->next()){
                tempName = query->value(1).toString();
            }

            //세 번째 값인 상태도 구해줌
            QString tempStatus = tempLine.split(",")[1];



            sendData += tempPID + "|" + tempName + "|" + tempStatus;

            //마지막 정보가 아니면 tempLine 뒤에 <r>을 붙여줌
            if(i != waitLineCount-1)
                sendData+="<r>";


        }
    }

    //    //대기목록 파일의 정보를 모듈로 보내주는 부분
    //    QFile oldList("waitingList.txt");

    //    qDebug() << oldList.exists();
    //    //waitingList.txt 파일이 존재하지 않는다면 대기리스트가 없는 것이므로 return 해준다
    //    if(oldList.exists() == false)
    //        return;


    //    oldList.open(QIODevice::Text | QIODevice::ReadOnly);
    //    QString dataText = oldList.readAll();
    //    qDebug() << "dataText"<<dataText;


    //    int waitLineCount = dataText.count(QLatin1Char('\n'));

    //    QString sendData = "WTR<CR>";


    //    //PMS에는 모든 정보를 보내줌
    //    if(specSocket == pmsSocket)
    //    {
    //        sendData += QString::number(waitLineCount) + "<CR>";


    //        for(int i=0; i<waitLineCount; i++)
    //        {
    //            QString tempLine = dataText.split("\n")[i];
    //            qDebug() << "tempLine" <<tempLine;

    //            for(int j=0 ; j<3; j++)
    //            {
    //                QString tempItem = tempLine.split(",")[j];
    //                qDebug() << tempItem;
    //                sendData += tempItem;

    //                if(j != 2)
    //                    sendData += "|";
    //            }

    //            if(i != waitLineCount-1)
    //                sendData+="<r>";
    //        }
    //    }
    //뷰어SW는 수납대기를 제외한 정보만 보내줌
    else if(specSocket==viewerSocket)
    {

        QString tempData;
        int tempCount = 0;

        for(int i=0; i<waitLineCount; i++)
        {
            QString tempLine = dataText.split("\n")[i];
            qDebug() << "tempLine" <<tempLine;

            QString tempPID = tempLine.split(",")[0];

            //환자 이름은 없는 상태이므로 DB에서 검색해 같이 보내줄 것
            query->exec("select * from patient WHERE patient_no = '" + tempPID + "'");
            QString tempName;
            while (query->next()){
                tempName = query->value(1).toString();
            }

            QString tempStatus = tempLine.split(",")[1];

            if(tempStatus != "WP")
            {
                tempCount++;
                tempData += tempPID + "|" + tempName + "|" + tempStatus;

                if(i != waitLineCount-1)
                {
                    tempData += "<r>";
                }
            }
        }
        sendData += QString::number(tempCount) + "<CR>" + tempData;

        qDebug() << "VIEWER SendData" << sendData;

        //        QString tempData;
        //        int tempCount = 0;

        //        for(int i=0; i<waitLineCount; i++)
        //        {
        //            QString tempLine = dataText.split("\n")[i];
        //            qDebug() << "tempLine" <<tempLine;

        //            QString tempPID = tempLine.split(",")[0];
        //            QString tempName = tempLine.split(",")[1];
        //            QString tempStatus = tempLine.split(",")[2];

        //            if(tempStatus != "WP")
        //            {
        //                tempCount++;
        //                tempData += tempPID + "|" + tempName + "|" + tempStatus;

        //                if(i != waitLineCount-1)
        //                {
        //                    tempData += "<r>";
        //                }
        //            }
        //        }
        //        sendData += QString::number(tempCount) + "<CR>" + tempData;

        //        qDebug() << "VIEWER SendData" << sendData;
    }


    //촬영SW는 촬영중인 정보만 보내줌
    else if(specSocket==imagingSocket)
    {

        QString tempData;
        int tempCount = 0;

        for(int i=0; i<waitLineCount; i++)
        {
            QString tempLine = dataText.split("\n")[i];
            qDebug() << "tempLine" <<tempLine;

            QString tempPID = tempLine.split(",")[0];


            //환자 이름은 없는 상태이므로 DB에서 검색해 같이 보내줄 것
            query->exec("select * from patient WHERE patient_no = '" + tempPID + "'");
            QString tempName;
            while (query->next()){
                tempName = query->value(1).toString();
            }


            QString tempStatus = tempLine.split(",")[1];

            if(tempStatus == "PA" || tempStatus == "CE" || tempStatus == "BO")
            {
                tempCount++;
                tempData += tempPID + "|" + tempName + "|" + tempStatus;

                if(i != waitLineCount-1)
                {
                    tempData += "<r>";
                }
            }
        }
        sendData += QString::number(tempCount) + "<CR>" + tempData;

        qDebug() << "IMG SendData" << sendData;




        //        QString tempData;
        //        int tempCount = 0;

        //        for(int i=0; i<waitLineCount; i++)
        //        {
        //            QString tempLine = dataText.split("\n")[i];
        //            qDebug() << "tempLine" <<tempLine;

        //            QString tempPID = tempLine.split(",")[0];
        //            QString tempName = tempLine.split(",")[1];
        //            QString tempStatus = tempLine.split(",")[2];

        //            if(tempStatus == "PA" || tempStatus == "CE" || tempStatus == "BO")
        //            {
        //                tempCount++;
        //                tempData += tempPID + "|" + tempName + "|" + tempStatus;

        //                if(i != waitLineCount-1)
        //                {
        //                    tempData += "<r>";
        //                }
        //            }
        //        }
        //        sendData += QString::number(tempCount) + "<CR>" + tempData;

        //        qDebug() << "IMG SendData" << sendData;
    }



    specSocket->write(sendData.toStdString().c_str());
}

//quit버튼을 누르면 정상종료. 대기리스트 파일이 삭제됨
void MainServer::on_quitPushButton_clicked()
{
    QFile::remove("./waitingList.txt");
    this->close();
}




