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

private slots:
    void newConnection();
    void newFileConnection();
    void socketDisconnected();
    void receiveData();
    bool writeData(QByteArray data);
    void receiveFile();
    void sendDataToClient(QString);
    void goOnSend(qint64);
    void sendFile();
    void on_quitPushButton_clicked();

private:
    Ui::MainServer *ui;
    QTcpServer *server;
    QTcpServer *fileServer;

    QString makeId();
    QString makeReportNo();
    QString makeImageNo();

    QSqlQuery *query;
    QSqlQuery *query2;
    QSqlQuery *query3;
    QSqlQuery *query4;

    QTcpSocket *socket;

    QTcpSocket *pmsSocket = nullptr;
    QTcpSocket *imagingSocket = nullptr;
    QTcpSocket *viewerSocket = nullptr;

    QTcpSocket *pmsFileSocket;
    QTcpSocket *imagingFileSocket;
    QTcpSocket *viewerFileSocket;

    QString saveData;

    QSqlTableModel *patientModel;
    QSqlTableModel *dentistModel;
    QSqlTableModel *imageModel;
    QSqlTableModel *reportModel;

    bool send_flag = false;

    QMap<QTcpSocket *, QString> sk; //socket
    QMap<QTcpSocket*, QString> fileSocketMap;       // <socket, SW or MODALITY>

    QString currentPID = "NULL";

    qint64 totalSize;
    qint64 byteReceived = 0;
    QFile* file;
    QByteArray inBlock;
    QString fileName;                           // Receiving FileName
    qint64 byteToWrite;             // File Size per a block
    QByteArray outBlock;            // Block for sending
    qint64 loadSize;                // File Size

    QString saveFileData;
    QString currentFileName;
    QString type;

    int sendFileFlag = 0; //0이면 pms로, 1이면 viewer로
    int count = 0;

    void sendWaitingList(QTcpSocket*);

signals:
    void sendNewPID(QString);

};
#endif // MAINSERVER_H
