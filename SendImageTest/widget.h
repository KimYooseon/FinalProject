#ifndef WIDGET_H
#define WIDGET_H

#include <QWidget>
#include <QtNetwork>

class QLineEdit;
class QTextEdit;


QT_BEGIN_NAMESPACE
namespace Ui { class Widget; }
QT_END_NAMESPACE

class Widget : public QWidget
{
    Q_OBJECT

public:
    Widget(QWidget *parent = nullptr);
    ~Widget();
    const int PORT_NUMBER = 8001;

private:
    Ui::Widget *ui;

    QTcpSocket *fileClient;
    qint64 byteToWrite;
    QByteArray outBlock;
    qint64 totalSize;
    QFile *file;
    qint64 loadSize;
    //QLineEdit *name;                // ID(이름)을 입력하는 창
    //QTextEdit *message;             // 서버에서 오는 메세지 표시용
    QLineEdit* serverAddress;
    QLineEdit* serverPort;
    bool isSent;

private slots:
    void goOnSend(qint64 numBytes);
    void sendFile();
};
#endif // WIDGET_H
