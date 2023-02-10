#include <iostream>
#include "../../server/include/net_common.h"
#include "../../server/include/net_message.h"

enum class CustomMsgTypes : uint32_t
{
    DirectMessage
};

int main()
{
    olc::net::message<CustomMsgTypes> msg;
    msg.header.id = CustomMsgTypes::DirectMessage;

    int a = 1;
    bool b = true;
    float c = 3.1459f;

    struct 
    {
        float x;
        float y;
    } d[5];

   

    msg << a << b << c << d;

    a =  99;
    b = false;
    c = 99.0f;

    msg >> d >> c >> b >> a;

    return 0;
}