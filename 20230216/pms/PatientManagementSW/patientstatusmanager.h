#ifndef PATIENTSTATUSMANAGER_H
#define PATIENTSTATUSMANAGER_H

#include <QWidget>
#include <QTreeWidget>

#include <QtTest/QtTest>
#include <QTest>

namespace Ui {
class PatientStatusManager;
}

class PatientStatusManager : public QWidget
{
    Q_OBJECT

public:
    explicit PatientStatusManager(QWidget *parent = nullptr);
    ~PatientStatusManager();

private:
    Ui::PatientStatusManager *ui;
    QString treatPID, treatName, payPID, payName;

    QTreeWidgetItem* selectedTreatRow, *selectedPayRow;

    QMap<int,QString> oldList;
    QString header;

private slots:
    void waitInfoSended(QString, QString);
    void on_waitPaymentTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_waitTreatmentTreeWidget_itemClicked(QTreeWidgetItem *item, int column);
    void on_shootRequestPushButton_clicked();

    void statusRequestSended(QString);

    void on_paymentPushButton_clicked();

    void PIDsendedtoWaitList(QString);

    void delPIDSended(QString);

    void oldListSended(QString);

    void waitListClearSlot();

signals:
    void sendRequest(QString);
    void inWaitListSignal(int);
    void sendDelFlag(int);
    void sendPayInfo(QString);
};

#endif // PATIENTSTATUSMANAGER_H
