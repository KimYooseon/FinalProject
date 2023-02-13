#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>


namespace Ui { class MainWindow ; }

class EnrollManager;
class ImageManager;
class MedicalRecordManager;
class PatientInfoManager;
class PatientStatusManager;
class NetworkManager;
class ChoosePatientManager;

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();



private slots:
    void enrollClient();


private:
    Ui::MainWindow *ui;

    EnrollManager *enrollManager;
    ImageManager *imageManager;
    MedicalRecordManager *medicalRecordManager;
    PatientInfoManager *patientInfoManager;
    PatientStatusManager *patientStatusManager;
    NetworkManager *networkManager;
    ChoosePatientManager *choosePatientManager;


signals:
    void requestPID(QString);
};
#endif // MAINWINDOW_H
