#ifndef ENROLLMANAGER_H
#define ENROLLMANAGER_H

#include <QWidget>

#include <QtTest/QtTest>
#include <QTest>


namespace Ui {
class EnrollManager;
}

class EnrollManager : public QWidget
{
    Q_OBJECT

public:
    explicit EnrollManager(QWidget *parent = nullptr);
    ~EnrollManager();

private slots:
    void enrollFinished();
    void newPIDSended(QString);

    void on_selectFilePushButton_clicked();

private:
    Ui::EnrollManager *ui;
    QString newPID;


    QString pidPhoto;
    QString fileName;


    QPixmap *pixmap;

signals:
    void sendNewData(QString);
    void sendNewDataForShow(QString, QString);

};

#endif // ENROLLMANAGER_H
