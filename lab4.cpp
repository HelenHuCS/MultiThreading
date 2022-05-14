/*
 * @Description:
 * @Author: cc
 * @Date: 2021-05-28 23:02:34
 * @LastEditors: cc
 * @LastEditTime: 2021-05-28 23:09:40
 */
#include "udp_multicast.h"
#include <iostream>
using namespace std;
#define SIZE 1024

int main(int argc, char* argv[])
{
    // validate arguments
    if (argc < 3)
    {
        cerr << "usage: lab4 group port [message]" << endl;
        return -1;
    }
    char* group = argv[1];
    int port = atoi(argv[2]);
    if (port < 5001)
    {
        cerr << "usage: lab4 group port [message]" << endl;
        return -1;
    }
    char* message = (argc == 4) ? argv[3] : NULL;
    // if message is null, the program should behave as a server,
    // otherwise as a client.
    UdpMulticast udp(group, port);

    if (message != NULL)
    {
        // client
        // implement yourself
        udp.getClientSocket();
        udp.multicast(message);
    }
    else
    {
        udp.getServerSocket();
        while (true)
        {
            // server
            // implement yourself
            char buffer[SIZE];
            udp.recv(buffer, SIZE);
            printf("%s\n", buffer);
        }
    }
    return 0;
}
