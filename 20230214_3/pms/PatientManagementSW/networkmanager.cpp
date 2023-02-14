#include "networkmanager.h"

#include <QBoxLayout>
#include <QDataStream>
#include <QTcpSocket>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>
#include <QMessageBox>

NetworkManager::NetworkManager(QObject *parent)
    : QObject{parent}
{

}

void NetworkManager::connectSocket(QString ip, int port)
{
    socket = new QTcpSocket(this);
    socket->connectToHost(ip, port);

    connect(socket, SIGNAL(readyRead()), this, SLOT(receiveData()));
    connect(socket, SIGNAL(disconnected()), SLOT(disconnect()));

    if(!socket->waitForConnected(1000))
    {
        disconnect();
        qDebug()<<("DataSocket connect fail\n");
    }
    else{
        connectCount = 1;   //연결된 상태

        qDebug()<<("DataSocket connect\n");
        QString connectData = "SEN^CNT<CR>PMS<CR>NULL";

        QByteArray sendTest = connectData.toStdString().c_str();
        socket->write(sendTest);

        emit changeScreenSignal(1);
    }


    fileSocket = new QTcpSocket(this);
    fileSocket->connectToHost(ip, port+1);

    if(fileSocket->waitForConnected(1000))
        fileSocket->write("CNT<CR>PMS<CR>NULL");
    else
        qDebug()<<("FileServer connect failed\n");

    connect(fileSocket, SIGNAL(readyRead()), this, SLOT(receiveFile()));

}


bool NetworkManager::connectToMainHost(QString host)
{
    socket->connectToHost(host, 8000);
    return socket->waitForConnected();
}

//연결이 잘 되다가 끊겼을 때
void NetworkManager::disconnect()
{
    if(connectCount == 1){

        QMessageBox::critical(nullptr, tr("경고"),
                              tr("서버와의 연결이 끊어졌습니다"));
        connectCount = 0;   //로그인화면으로 돌아가니까 0으로 초기화
        emit changeScreenSignal(0);

    }
    else if(connectCount == 0)
    {
        QMessageBox::critical(nullptr, tr("경고"),
                              tr("서버 연결이 불가능한 상태입니다."));
        return;

    }
    socket->deleteLater();
    fileSocket->deleteLater();
}





void NetworkManager::receiveFile() //P00004 파일 2개짜리로 해보기
{
    downloadOrNotFlag =0;   //다운로드 중임을 알려주는 flag

    fileSocket = dynamic_cast<QTcpSocket*>(sender());


    allFile.append(fileSocket->readAll());

    if(QString(allFile).right(5) == "<FIN>")
    {
        qDebug() << "allFile size: " <<allFile.size();
        qDebug() << "Finish Catched";
        makeFiles();
    }
}


void NetworkManager::makeFiles()
{

    QByteArray fileInfoArray = allFile.split('|')[0];
    qDebug() << "fileInfoArray.size: " << fileInfoArray.size(); //ex.53

    QString totalFileInfo = fileInfoArray.toStdString().c_str();
    int fileCount = totalFileInfo.split("<CR>")[0].toInt();
    QString fileInfo = totalFileInfo.split("<CR>")[1];


    QMap<int, int> fileMap; //인덱스, 파일크기
    QMap<int, QString> fileNameMap; //인덱스, 파일이름

    for(int i=0; i<fileCount; i++)
    {
        QString partInfo = fileInfo.split("<n>")[i];

        QString partFileName = partInfo.split("<f>")[0];
        qDebug() << "partFileName"<<partFileName;

        int partFileSize = partInfo.split("<f>")[1].toInt();
        qDebug() << "partFileSize"<<partFileSize;

        fileMap.insert(i, partFileSize);
        fileNameMap.insert(i, partFileName);
        allFileSize += partFileSize;
    }


    QDir dir("Image");
    if (!dir.exists())
        dir.mkpath(".");


    int firstSize, secondSize;
    for(int i=0; i<fileCount; i++)
    {
        fileName = dir.path() + "/" + fileNameMap.value(i);
        QFile file;
        file.setFileName(fileName);
        file.open(QIODevice::WriteOnly);

        QByteArray writeArray;
        if(i==0){
            writeArray.append(allFile.sliced(fileInfoArray.size()+1, fileMap.value(0)));
            firstSize = fileInfoArray.size()+1;
            secondSize = fileMap.value(0);
        }
        else{
            writeArray.append(allFile.sliced(firstSize+secondSize, fileMap.value(i)));
            firstSize = firstSize+secondSize;
            secondSize = fileMap.value(i);

        }

        file.write(writeArray);



    }
    allFile.clear();
    downButtonClicked = 0;
    downloadOrNotFlag =1;   //다 다운로드 받았다는 flag

    qDebug() <<"make file finished!";

    //파일이 다 write되었을 때 patientInfoManager 쪽으로 신호를 보내줌
    //(파일이 정상적으로 끝까지 다운로드되기 전까지는 검색버튼을 다시 누르지 못하도록 만들기 위함)
    emit fileSendedSig(downloadOrNotFlag);  //1을 보내면 파일전송완료되었다는 뜻
    emit PSEDataInNET(id);
}


void NetworkManager::downloadOrNotSlot()
{

    if(downButtonClicked != 0)  //처음 검색한 환자가 아니거나 다운로드가 끝난 후에 검색한 환자가 아닐 때(즉 다운로드 진행중일 때)
    {
        if(downloadOrNotFlag==0)
        {
            emit fileSendedSig(downloadOrNotFlag);  //0을 보내면 파일전송 중이라는 뜻
        }

    }
    downButtonClicked+=1;
}




bool NetworkManager::writeData(QByteArray data)
{
    if(socket->state() == QAbstractSocket::ConnectedState)
    {
        //socket->write(IntToArray(data.size()));
        socket->write(data); // 데이터를 보내줌
        return socket->waitForBytesWritten();
    }
    else
    {
        return false;
    }
}

//서버로 보내줄 데이터
void NetworkManager::newDataSended(QString newData)
{
    if(socket->waitForConnected())
    {
        QString sendData = newData; //MainServer의 textEdit에 띄울 정보
        send_flag = writeData(sendData.toStdString().c_str()); //writeData의 첫 번째 인자는 char *data와 같은 형식임

        if(!send_flag)
            qDebug() << "Socket send fail\n";

    }

}

//서버에서 받아올 데이터
void NetworkManager::receiveData()
{
    socket = static_cast<QTcpSocket*>(sender());
    QByteArray array = socket->readAll();

    saveData = QString(array);

    if(saveData.contains("<CR>", Qt::CaseInsensitive) == true)
    {

        //어떤 이벤트인지에 따라 불러올 함수 써주기(각각 이벤트에 대한 함수 만들고 if-else문 타도록 만들자)
        QString event = saveData.split("<CR>")[0];
        event = event.split("^")[1];

        id = saveData.split("<CR>")[1];
        QString data = saveData.split("<CR>")[2];

        qDebug() << "event: " << event;

        if(event == "PID")              //ACK만 옴
        {
            sendedPID = id;
            emit sendNewPID(id); //enrollment 클래스로 emit
            qDebug() << __LINE__ <<  "sendedPID: " << id << ":" << sendedPID;
        }
        else if(event == "PSE")         //ACK만 옴
        {
            qDebug() << "saveData" << saveData;
            emit sendSearchResult(id, data);    //patientInfoManager 클래스와 medicalRecordManager 클래스 두 곳으로 모두 보내줘야 함
        }
        else if(event == "SRQ") //촬영 의뢰: 환자SW와 뷰어SW ->촬영SW      //촬영SW가 꺼져있으면 ERR
        {
            qDebug()<<"SRQ event Received: " << saveData;
            emit sendSRQRequest(saveData);
        }
        else if(event == "VTS") //진료 시작      //ACK만 옴
        {
            qDebug()<<"VTS event Received: " << saveData;
            emit sendVTSRequest(saveData);
        }
        else if(event == "ISV")                 //ACK만 옴
        {
            qDebug()<<"ISV event Received: " << saveData;
            emit sendISVevent(saveData);
        }
        else if(event == "VTF")                 //ACK만 옴
        {
            qDebug()<<"VTF event Received: " << saveData;
            emit sendVTFevent(saveData);
        }
        else if(event == "VNT")                 //ACK만 옴
        {
            qDebug()<<"VNT event Received: " << saveData;
            emit sendVNTevent(saveData);
        }
        else if(event == "WTR")                 //ACK만 옴
        {
            qDebug()<<"WTR event Received: " << saveData;
            emit sendWTRevent(saveData);
        }
        else if(event == "MWL")                 //ACK만 옴
        {
            qDebug()<<"MWL event Received: " << saveData;
            emit sendMWLevent(saveData);
        }
        else if(event == "AWL") //대기 환자 추가: 뷰어가 꺼져있으면 ERR, 켜져있으면 ACK
        {
            qDebug()<<"AWL event Received: " << saveData;
            emit sendAWLRequest(saveData);  //patientInfoManager 클래스로 보내서 대기명단 추가를 막아 patientStatus 클래스까지 가지 않도록 만들자
        }

    }
}

void NetworkManager::sendedIP(QString ip, int port)
{
    qDebug() << "ip: " << ip;
    hostIP = ip;
    hostPORT = port;

    connectSocket(ip, port);
}
