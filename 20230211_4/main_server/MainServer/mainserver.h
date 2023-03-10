#ifndef MAINSERVER_H
#define MAINSERVER_H

#include <QMainWindow>
#include <QtCore>
#include <QtNetwork>
#include <QString>
#include <QSqlTableModel>
#include <QTableWidget>

class QStandardItemModel;

namespace Ui {
class MainServer;
}

class MainServer : public QMainWindow
{
    Q_OBJECT
public:
    explicit MainServer(QWidget *parent = nullptr);
    ~MainServer();
    void loadData();
    void updateRecentData();


private slots:
    void newConnection();
    void newFileConnection();


    void socketDisconnected();
    void receiveData();
    bool writeData(QByteArray data);
    void receiveFile();




private slots:
    void sendDataToClient(QString);

    void goOnSend(qint64);

    void sendFile();


    void on_quitPushButton_clicked();



private:
    Ui::MainServer *ui;
    QTcpServer *server;
    QTcpServer *fileServer;

    QHash<QTcpSocket*, QByteArray*> buffers; //We need a buffer to store data until block has completely received
    QHash<QTcpSocket*, qint32*> sizes; //We need to store the size to verify if a block has received completely
    QString makeId();
    QString makeReportNo();
    QString makeImageNo();
    QSqlQuery *query;
    QSqlQuery *query2;
    QSqlQuery *query3;
    QSqlQuery *query4;
    QSqlQuery *query5;

    QTcpSocket *socket;

    QTcpSocket *pmsSocket = nullptr;
    QTcpSocket *imagingSocket = nullptr;
    QTcpSocket *viewerSocket = nullptr;


    QTcpSocket *pmsFileSocket;
    QTcpSocket *imagingFileSocket;
    QTcpSocket *viewerFileSocket;


    QByteArray *buffer;
    QString saveData;

    QSqlTableModel *patientModel;
    QSqlTableModel *dentistModel;
    QSqlTableModel *imageModel;
    QSqlTableModel *reportModel;


    bool fd_flag = false;
    bool send_flag = false;

    QMap<QTcpSocket *, QString> sk; //socket

    qint64 totalSize;
    qint64 byteReceived = 0;
    QFile* file;

    QByteArray inBlock;
    QString fileName;                           // Receiving FileName
    QString checkFileName;                      // Previous File Name for checking new file
    QString currentPID = "NULL";
    QString currentType = "NULL";
    qint64 byteToWrite;             // File Size per a block
    QByteArray outBlock;            // Block for sending
    int count = 0;
    qint64 loadSize;                // File Size
    QMap<QTcpSocket*, QString> fileSocketMap;       // <socket, SW or MODALITY>
    QString saveFileData;

    int sendFileFlag = 0; //0?????? pms???, 1?????? viewer
    QString type;



    QByteArray sendAllFile;



    void sendWaitingList(QTcpSocket*);

 int tmpCount = 0;
 QString currentFileName;

signals:
    void sendNewPID(QString);

};
#endif // MAINSERVER_H
