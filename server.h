#ifndef SERVER_H
#define SERVER_H

#include <QDialog>
#include <QAbstractSocket>
#include <QTcpServer>

class QTcpSocket;
class QFile;

QT_BEGIN_NAMESPACE
namespace Ui { class Server; }
QT_END_NAMESPACE

class Server : public QDialog
{
    Q_OBJECT

public:
    Server(QWidget *parent = nullptr);
    ~Server();

private:
    Ui::Server *ui;

    QTcpServer tcpServer;
    QTcpSocket *tcpServerConnection;
    qint64 totalBytes;     // Сохраняем информацию об общем размере
    qint64 bytesReceived;  // Размер полученных данных
    qint64 fileNameSize;   // информация о размере имени файла
    qint64 imageSize;      // размер картинки

    QString fileName;      // Сохраняем имя файла
    QFile *localFile;      // локальные файлы
    QByteArray inBlock;    // буфер данных
    QString imageContent;

    QImage image;//образ


private slots:
    void start();   // Мониторинг события
    void acceptConnection();    // После подключения клиентом создаем сокет, получаем данные, обрабатываем исключения и закрываем сервер
    void updateServerProgress();    // Получение и обработка отображаемого изображения
    void displayError(QAbstractSocket::SocketError socketError);    //Обработка ошибок

    // base64 строка для изображения
    QImage getImage(const QString &);

    void on_StartB_clicked();   // Отслеживать или отключать
};
#endif // SERVER_H
