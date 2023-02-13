#ifndef PATIENTINFOMANAGER_H
#define PATIENTINFOMANAGER_H

#include <QWidget>
#include <QtCore>
#include <QtNetwork>


namespace Ui {
class PatientInfoManager;
}

class PatientInfoManager : public QWidget
{
    Q_OBJECT

public:
    explicit PatientInfoManager(QWidget *parent = nullptr);
    ~PatientInfoManager();

private:
    Ui::PatientInfoManager *ui;

    QString pid, name, sex, birthdate, tel, address, memo, fileName;
    QString pidPhoto, currentFileName, changeFileName;

    QPixmap *pixmap;

    int waitSignal;
    int count =0;
    int fileSendedFlag =0;
    int searchButtonClicked=0;
    int patientInDB =0;
    int waitError=0;

private slots:
    void on_searchPushButton_clicked();
    void searchDataSended(QString, QString);
    void on_deletePushButton_clicked();
    void on_WaitPushButton_clicked();
    void on_modifyPushButton_clicked();
    void inWaitListSlot(int);
    void delFlagSended(int);
    void fileSendedSlot(int);
    void sendedNewDataForShow(QString, QString);
    void on_changePhotoPushButton_clicked();
    void sendedAWLRequest(QString);

signals:
    void sendSearchData(QString);
    void sendDelData(QString);
    void sendWaitInfo(QString);
    void sendModifyData(QString);
    void sendPIDtoWaitList(QString);
    void sendDelPID(QString);
    void downloadOrNotSignal();
    void cleanImageSignal();
    void sendTempData(QString, QString);
    void sendWaitInfoToWaitList(QString, QString);
};

#endif // PATIENTINFOMANAGER_H
