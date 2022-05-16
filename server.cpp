#include "server.h"
#include "ui_server.h"

#include <QtNetwork>
#include <QCompleter>

Server::Server(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::Server)
{
    ui->setupUi(this);

    // Автозаполнение порта и запрос по умолчанию
    ui->PortLI->setPlaceholderText(tr("1234"));// Устанавливаем приглашение по умолчанию
    QStringList portWordList;
    portWordList << tr("1234");
    QCompleter* portCompleter = new QCompleter(portWordList, this);
    ui->PortLI->setCompleter(portCompleter);

    connect(&tcpServer, SIGNAL(newConnection()),this, SLOT(acceptConnection()));

    ui->ImageL->show();
}

Server::~Server()
{
    delete ui;
}

void Server::start()
{
    if (!tcpServer.listen(QHostAddress::LocalHost, ui->PortLI->text().toInt()))
    {
        qDebug() << tcpServer.errorString();
        close();
        return;
    }

    totalBytes = 0;
    bytesReceived = 0;
    imageSize = 0;
    ui->StatusLI->setText(tr("Ожидание"));
}

void Server::acceptConnection()
{
    // Получаем сокет ссылки
    tcpServerConnection = tcpServer.nextPendingConnection();

    // Получение данных
    // readyRead () при появлении новой полезной нагрузки сетевых данных в сетевом сокете
    connect(tcpServerConnection, SIGNAL(readyRead()), this, SLOT(updateServerProgress()));
    // Обработка исключения
    connect(tcpServerConnection, SIGNAL(error(QAbstractSocket::SocketError)), this, SLOT(displayError(QAbstractSocket::SocketError)));

    ui->StatusLI->setText(tr("Связь установлена"));
    // Закрываем сервер и больше не слушаем
    //tcpServer.close();
}


void Server::updateServerProgress()
{
    QDataStream in(tcpServerConnection);
    in.setVersion(QDataStream::Qt_5_6);

    // Если полученные данные меньше 16 байт, сохраняем структуру заголовка входящего файла
    if (bytesReceived <= sizeof(qint64)*2)
    {
        if((tcpServerConnection->bytesAvailable() >= sizeof(qint64)*2) && (imageSize == 0))
        {
            // Получение информации об общем размере данных и информации о размере имени файла
            in >> totalBytes  >> imageSize;
            bytesReceived += sizeof(qint64) * 2;

            if(imageSize == 0)
            {
                  ui->StatusLI->setText(tr("Показанная картинка пуста!"));
            }
              qDebug() <<"Точка привязки 0"<< endl;
        }
        if((tcpServerConnection->bytesAvailable() >= imageSize) && (imageSize != 0))
        {

            // Получение имени файла и создание файла
            in >> imageContent;

            //qDebug() << imageContent << endl;

            ui->StatusLI->setText(tr("Получаем файл"));

            QImage imageData = getImage(imageContent);

            QPixmap resImage = QPixmap::fromImage(imageData);
            QPixmap* imgPointer = &resImage;
            imgPointer->scaled(ui->ImageL->size(), Qt::IgnoreAspectRatio);// Изменим размер изображения по размеру окна
            //imgPointer-> scaled (ui-> imageLabel-> size (), Qt :: KeepAspectRatio); // Устанавливаем размер масштабирования растрового изображения

            ui->ImageL->setScaledContents(true);// Установить атрибут метки, можно увеличить растровое изображение, чтобы заполнить все доступное пространство.
            ui->ImageL->setPixmap(*imgPointer);

            bytesReceived += imageSize;

            qDebug() << "Обнаружение 1 полученного байта:" << bytesReceived << endl;

            if(bytesReceived == totalBytes){
                 ui->StatusLI->setText(tr("Файл успешно получен"));
                 totalBytes = 0;
                 bytesReceived = 0;
                 imageSize = 0;
            }

         }
     }
}

void Server::displayError(QAbstractSocket::SocketError socketError)
{
    qDebug() <<"errorString()" <<tcpServerConnection->errorString();
    tcpServerConnection->close();
    ui->StatusLI->setText(tr("Сервер готов"));
}

QImage Server::getImage(const QString &data)
{
    QByteArray imageData = QByteArray::fromBase64(data.toLatin1());
    QImage image;
    image.loadFromData(imageData);
    return image;
}

// Кнопка начала прослушивания
void Server::on_StartB_clicked()
{
    if(ui->StartB->text() == tr("Старт"))
    {
        ui->StartB->setText(tr("Конец"));
        start();
    }
    else
    {
        ui->StartB->setText(tr("Старт"));
        tcpServer.close();
        tcpServerConnection->disconnectFromHost();
    }
}

