#include "widget.h"
#include "ui_widget.h"

#include <QTextEdit>
#include <QLineEdit>
#include <QDataStream>
#include <QTcpSocket>
#include <QFileDialog>
#include <QFile>
#include <QFileInfo>

Widget::Widget(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::Widget)
{
    ui->setupUi(this);

    /* 파일 전송을 위한 소켓 */
    fileClient = new QTcpSocket(this);
    fileClient->connectToHost("127.0.0.1", 8001);
    if (fileClient->waitForConnected()) {
        fileClient->write("CNT<CR>IMG<CR>NULL");
    } else {
        qDebug() << "FILE SERVER CONNECT FAILED";
    }

    connect(ui->pushButton, SIGNAL(clicked( )), SLOT(sendFile()));     //fileButton을 누르면 sendFile( )이 실행됨

}

Widget::~Widget()
{
    delete ui;
}

/* 파일 전송시 여러번 나눠서 전송 */
void Widget::goOnSend(qint64 numBytes)      // 파일 전송을 시작
{

    /*파일의 전체 크기에서 numBytes씩만큼 나누어 전송*/
    byteToWrite -= numBytes; // 데이터 사이즈를 유지

    outBlock = file->read(qMin(byteToWrite, numBytes));

    fileClient->write(outBlock);

    if (byteToWrite == 0) {                 // 전송이 완료되었을 때(이제 더이상 남은 파일 크기가 없을 때)
        qDebug("File sending completed!");
    }
}

/* 파일 보내기 */
void Widget::sendFile() // 파일을 열고 파일 이름을 가져오는 부분 (including path)
{
    connect(fileClient, SIGNAL(bytesWritten(qint64)), SLOT(goOnSend(qint64)));  //데이터를 보낼 준비가되면 다른 데이터를 보내고, 데이터를 다 보냈을 때는 데이터 전송을 끝냄
    loadSize = 0;
    byteToWrite = 0;
    totalSize = 0;
    outBlock.clear();

    QString PID; //메인서버에 연결된 후에 특정 PID 환자의 사진을 보내달라고 요청이 오면 여기서 보냄

    QString filename = "./Image/P00006.png";
    //QString filename = QString("./%1.jpg").arg(PID).first(6);
    if(filename.length()) {             //파일이름 선택할 때 파일이름의 길이가 1이상이면
        file = new QFile(filename);     //file 객체를 만들어 파일을 readOnly로 연다
        file->open(QFile::ReadOnly);

        qDebug() << QString("file %1 is opened").arg(filename);

//        if (!isSent) { // Only the first time it is sent, it happens when the connection generates the signal connect
//            fileClient->connectToHost("127.0.0.1", PORT_NUMBER);    //fileClient에서 serverAddress와 serverPort를 통해 host로 커넥트 함
//            isSent = true;
//        }

        // When sending for the first time, connectToHost initiates the connect signal to call send, and you need to call send after the second time

        byteToWrite = totalSize = file->size(); // The size of the remaining data
        loadSize = 1024; // The size of data sent each time


        QDataStream out(&outBlock, QIODevice::WriteOnly);

        out << qint64(0) << qint64(0) << filename;  //filename과 name의 크기가 현재는 어떤 상태인지 모르니까 일단 만들어놓음

        totalSize += outBlock.size(); // The total size is the file size plus the size of the file name and other information
        byteToWrite += outBlock.size();

        out.device()->seek(0); // 앞으로 이동해 일단 0으로 설정되어 있었던 totalsize와 byteToWrite의 제대로 된 값을 적어줌 // Go back to the beginning of the byte stream to write a qint64 in front, which is the total size and file name and other information size
        out << totalSize << qint64(outBlock.size());

        fileClient->write(outBlock); // Send the read file to the socket    //서버로 보내줌

    }
    qDebug() << QString("Sending file %1").arg(filename);
}
