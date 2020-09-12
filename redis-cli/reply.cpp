#include "reply.h"

using namespace Redis;

Reply::Reply(Reply::Types type, QVariant value) :
    m_type(type),
    m_value(value)
{
    ;
}

Reply::Types Reply::type()
{
    return m_type;
}

QVariant Reply::value()
{
    return m_value;
}
