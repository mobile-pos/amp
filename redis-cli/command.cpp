#include "command.h"

using namespace Redis;

Command::Command(QString commandName, QObject *parent) : QObject(parent),
    commandName(commandName)
{
    arguments << commandName;
}

void Command::addArgument(QVariant argument)
{
    arguments << argument;
}

void Command::addArgumentStrings(QStringList arguments)
{
    foreach (QString argument, arguments) {
        addArgument(argument);
    }
}

QString Command::getCommandName()
{
    return arguments[0].toString();
}

QVariantList Command::getArguments()
{
    return arguments;
}
Command* Command::HKEYS(QString key) {
    Command *command = new Command("HKEYS");

    command->addArgument(key);

    return command;
}
Command *Command::Auth(QString auth) {
    Command *command = new Command("AUTH");

    command->addArgument(auth);

    return command;
}
Command *Command::HGETALL(QString key) {
    Command *command = new Command("HGETALL");

    command->addArgument(key);

    return command;
}
Command *Command::HGET(QString key, QString field)
{
    Command *command = new Command("HGET");

    command->addArgument(key);
    command->addArgument(field);

    return command;
}
Command *Command::HSET(QString key, QString field, QString value) {

    Command *command = new Command("HSET");

    command->addArgument(key);
    command->addArgument(field);
    command->addArgument(value);

    return command;
}
Command *Command::HDEL(QString key, QString field) {

    Command *command = new Command("HDEL");

    command->addArgument(key);
    command->addArgument(field);

    return command;
}

Command *Command::SET(QString key, QString value) {
    Command *command = new Command("SET");

    command->addArgument(key);
    command->addArgument(value);

    return command;
}
Command *Command::GET(QString key)
{
    Command *command = new Command("GET");

    command->addArgument(key);

    return command;
}

Command *Command::DEL(QStringList keys)
{
    Command *command = new Command("DEL");

    command->addArgumentStrings(keys);

    return command;
}

Command *Command::EXPIRE(QString key, int seconds)
{
    Command *command = new Command("EXPIRE");

    command->addArgument(key);
    command->addArgument(seconds);

    return command;
}

Command *Command::KEYS(QString pattern)
{
    Command *command = new Command("KEYS");

    command->addArgument(pattern);

    return command;
}

Command *Command::LPUSH(QString key, QStringList values)
{
    Command *command = new Command("LPUSH");

    command->addArgument(key);
    command->addArgumentStrings(values);

    return command;
}

Command *Command::RPOP(QString key)
{
    Command *command = new Command("RPOP");

    command->addArgument(key);

    return command;
}

Command *Command::SUBSCRIBE(QStringList channels)
{
    Command *command = new Command("SUBSCRIBE");

    command->addArgumentStrings(channels);

    return command;
}

Command* Command::PUBLISH(QString channel, QString value) {
    Command *command = new Command("PUBLISH");

    command->addArgument(channel);
    command->addArgument(value);

    return command;
}
