#include "imagemanager.h"
#include "ui_imagemanager.h"
#include <QDir>
#include <QListWidget>
#include <windows.h>

ImageManager::ImageManager(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::ImageManager)
{
    ui->setupUi(this);

    reloadImages("P00004");
}

ImageManager::~ImageManager()
{
    delete ui;
}

void ImageManager::reloadImages(QString id)
{
    qDebug() << "@@@@@@@@@@@@@";
    QDir dir(QString("./Image/%1").arg(id));

    qDebug() << dir;
    //QDir dir("./Image/P00002");

    //왜 dir(".")으로 해두고 실행시키면 폴더 안에 넣지 않았을 때 잘 뜨는데 폴더 안에만 넣어두면 안되는 건지 모르겠다...
    //QDir dir(".");
    QStringList filters;
    //filters << "*.png" << "*.jpg" << "*.bmp" << "*.gif";[
    filters << id + "*.*";
    QFileInfoList fileInfoList = dir.entryInfoList(filters, QDir::Files | QDir::NoDotAndDotDot);

    ui->imageListWidget->clear();
    for(int i = 0; i < fileInfoList.count(); i++) {
        QListWidgetItem* item = new QListWidgetItem(QIcon(dir.path() + "/" + fileInfoList.at(i).fileName()), NULL, ui->imageListWidget); //, QListWidgetItem::UserType);

        item->setStatusTip(fileInfoList.at(i).fileName());

        ui->imageListWidget->addItem(item);
        qDebug() << "##############";
    };
}

//안불러와짐 ㅠ
void ImageManager::PSEDataSended(QString id)
{
    ui->imageListWidget->clear();
    qDebug() << "id: " << id;
    pid = id;
    //Sleep(5);
    reloadImages(id);
}
