#ifndef REPLY_H
#define REPLY_H

#include <QVariant>

namespace Redis {
    class Reply
    {
    public:
        enum Types {
            SimpleString,
            Error,
            Integer,
            BulkString,
            Array
        };

        Reply(Types type, QVariant value);

        Reply::Types type();
        QVariant value();

    private:
        Reply::Types m_type;
        QVariant m_value;
    };
}

#endif // REPLY_H
