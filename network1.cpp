#include <iostream>

#include <QtCore>
#include <QApplication>
#include <QTimer>
#include <QtNetwork/QtNetwork>


int main(int argc, char** argv)
{
    // Required => First line need to start QT Framework
    QApplication app(argc, argv);
    std::cout << " [INFO] Starting application" << std::endl;

    // ===============================================================//
    std::cout << " [===== EXPERIMENT 1 - Simple HTTP GET Request ======]\n"
              << std::endl;

    //QString url1 = "http://www.httpbin.org/get";
    QUrl url;
    url.setUrl("http://www.httpbin.org/get");
    QNetworkAccessManager qnam;

    QNetworkReply* reply = qnam.get(QNetworkRequest(url));

    // Callback called while the network request is in progress
    QObject::connect(reply, &QNetworkReply::downloadProgress, [&]{
        std::cout << " [TRACE] Waiting reply ... " << std::endl;
    });

    // Callback claled when the network request is finished
    QObject::connect(reply, &QNetworkReply::finished, [=]{
       std::cout << " [TRACE] Download finished. OK " << std::endl;
       std::cout << " [INFO]  reply->isOpen()     = "
                 << reply->isOpen() << std::endl;
       std::cout << " [INFO]  reply->isRunning()  = "
                 << reply->isRunning() << std::endl;
       std::cout << " [INFO]  reply->isFinished() = "
                 << reply->isFinished() << std::endl;

       QVariant statusCode = reply->attribute(QNetworkRequest::HttpStatusCodeAttribute);
      // QVariant contentSize = reply->attribute(QNetworkRequest::ContentLengthHeader);

       std::cout << " [INFO] Status code  = " << statusCode.toInt() << std::endl;
    //  std::cout << " [INFO] Content Size = " << contentSize.toInt() << std::endl;

       std::cout << "\n Output: ";
       std::cout << reply->readAll().toStdString() << std::endl;

       // Force exiting QT event loop shuttind down this app
       QApplication::exit(0);
    });

    //=====================================================================//

    // Start Event Loop blocking main thread
    return app.exec();
}
