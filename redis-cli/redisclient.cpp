#include <QDebug>

#include "redisclient.h"

RedisClient::RedisClient(QObject *parent) : QObject(parent),
    subscribing(false)
{
    initConnectionHandler();
}

void RedisClient::connectToServer(QString hostName, int port)
{
    connection.connectToHost(hostName, (quint16)port);
}

void RedisClient::disconnectFromServer()
{
    connection.disconnectFromHost();
}

void RedisClient::sendCommand(Command *command)
{
    if (subscribing)
        return;

    sentCommands.enqueue(command);

    sendRequest(command->getArguments());

    subscribing = command->getCommandName() == "SUBSCRIBE";

    if (subscribing) {
        qDebug() << "subscribling ...";
        connect(&connection, SIGNAL(readyRead()), SLOT(onParseReply()));

        this->connect(this, SIGNAL(messageReceived(const QString&, const QVariant& )), this->parent(), SLOT(messageReceived(const QString&, const QVariant&)));
    }

    delete command;
}

void RedisClient::initConnectionHandler()
{
    connect(&connection, SIGNAL(connected()), SIGNAL(connected()));
    connect(&connection, SIGNAL(disconnected()), SIGNAL(disconnected()));
    connect(&connection, SIGNAL(disconnected()), SLOT(clearState()));
    connect(&connection, SIGNAL(error(QAbstractSocket::SocketError)), SLOT(handleConnectionError(QAbstractSocket::SocketError)));


    this->connect(this, SIGNAL(onReply(const QString&, Reply)), this->parent(), SLOT(onReply(const QString&, Reply)));
    this->connect(this, SIGNAL(connected()), this->parent(), SLOT(connected()));
    this->connect(this, SIGNAL(disconnected()), this->parent(), SLOT(disconnected()));
}

void RedisClient::sendRequest(QVariantList lines)
{

    if (connection.state() == QTcpSocket::ConnectedState && !lines.isEmpty()) {
        QByteArray commandLine = QString("*%1\r\n").arg(QString::number(lines.count())).toUtf8();

        QString cmd = "";
        foreach (QVariant line, lines) {
            QString bulkString = line.toString();

            cmd += bulkString + " ";
            commandLine.append(QString("$%1\r\n").arg(QString::number(bulkString.length())).toUtf8());
            commandLine.append(QString("%1\r\n").arg(bulkString).toUtf8());
        }
//        qDebug() << "command: " << commandLine;
        connection.write(commandLine);
        connection.waitForBytesWritten();
        connection.waitForReadyRead();

        parseReply(cmd);
    }
}

void RedisClient::handleConnectionError(QAbstractSocket::SocketError socketError)
{
    Q_UNUSED(socketError)
}

void RedisClient::clearState()
{
    subscribing = false;
}

void RedisClient::onParseReply() {
    parseReply("");
}

void RedisClient::parseReply(const QString& cmd)
{
    Reply::Types type = Reply::SimpleString;
    QVariant value;

    bool isArray = false;
    int arrayLength = 0;
    QVariantList array;

    while (connection.canReadLine()) {
        QByteArray line = connection.readLine().trimmed(),
                   firstByte = line.left(1);

        if (firstByte == "*") { // Array
            line.remove(0, 1);

            isArray = true;
            arrayLength = line.toInt();

            continue;
        } else if (firstByte == "+") { // Simple String
            line.remove(0, 1);

            type = Reply::SimpleString;
            value = QString(line);
        } else if (firstByte == "-") { // Error
            line.remove(0, 1);

            type = Reply::Error;
            value = QString(line);
        } else if (firstByte == ":") { // Integer
            line.remove(0, 1);

            type = Reply::Integer;
            value = line.toInt();
        } else if (firstByte == "$") { // Bulk String
            type = Reply::BulkString;

            continue;
        } else if (type == Reply::BulkString) {
            value = QString(line);
        }

        if (isArray)
            array << value;
    }

    if (isArray) {
        type = Reply::Array;
        value = array;
    }

    if (subscribing)
        messageReceived(array[1].toString(), array[2]);
    else {
        onReply(cmd, Reply(type, value));
    }
    Q_UNUSED(arrayLength)
}
