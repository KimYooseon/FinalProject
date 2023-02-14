#ifndef NETWORKMANAGER_H
#define NETWORKMANAGER_H

#include <QObject>
#include <QtCore>
#include <QtNetwork>

#include <QtTest/QtTest>
#include <QTest>

class NetworkManager : public QObject
{
    Q_OBJECT
public:
    NetworkManager(QObject *parent = nullptr);

public slots:

    bool writeData(QByteArray data);
    void connectSocket(QString, int);


    void sendedIP(QString, int);


private slots:
    void newDataSended(QString);
    void receiveData();
    void receiveFile();


    void makeFiles();



    void downloadOrNotSlot();
    void disconnect();




private:
    QTcpSocket *socket;
    bool fd_flag = false;
    bool file_flag = false;
    bool send_flag = false;
    QTcpSocket *PMSocket;
    QByteArray *buffer;
    QString saveData;

    QString sendedPID;
    QHash<QTcpSocket*, QByteArray*> buffers; //We need a buffer to store data until block has completely received
    QTcpServer *server;
    QHash<QTcpSocket*, qint32*> sizes; //We need to store the size to verify if a block has received completely



    QTcpSocket *fileSocket;

    qint64 totalSize;
    qint64 byteReceived = 0;
    QString fileName;                           // Receiving FileName
    QString checkFileName;
    QFile* file;
    QByteArray inBlock;
    QString currentPID = "NULL";

    QByteArray *byteArray;
    QByteArray allFile;



    int allFileSize = 0;
    QByteArray allFileSended;
    QString id;



    int downloadOrNotFlag = 0;
    int downButtonClicked =0;


    //0일때는 서버 연결 시도를 했는데 연결 안 된 상태, 1일때는 연결되었다가 서버가 꺼져 연결 끊긴 상태
    int connectCount=0;


    QString hostIP = "";
    int hostPORT = 8000;


signals:
    void sendNewPID(QString);
    void sendSearchResult(QString, QString);
    void sendSRQRequest(QString);
    void sendVTSRequest(QString);
    void sendISVevent(QString);
    void sendVTFevent(QString);
    void sendVNTevent(QString);


    void PSEDataInNET(QString);

   void sendByteArray(const QPixmap&);

   void sendWTRevent(QString);

   void fileSendedSig(int);


   void sendMWLevent(QString);

   void sendAWLRequest(QString);

   void quitRequest();

   void changeScreenSignal(int);   //0이면 로그인 화면, 1이면 메인화면

};

#endif // NETWORKMANAGER_H
