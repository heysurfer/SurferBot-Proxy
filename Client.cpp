#ifdef _WIN32
#define _CRT_SECURE_NO_WARNINGS
#endif
#ifndef _INC_STDIO
#include <stdio.h>
#endif
#include <enet.h>
#include <chrono>
#include <thread>
enum
{
    LOGIN_SUCCESS = 0x1,
    LOGIN_FAILED = 0x2,
};

int main()
{
    if (enet_initialize() != 0)
    {
        fprintf(stderr, "An error occurred while initializing ENet.\n");
        return EXIT_FAILURE;
    }
    auto createHost = [=]() {
        ENetHost* _client = enet_host_create(NULL, 1, 5, 0, 0);
        if (_client == NULL)
        {
            fprintf(stderr, "An error occurred while creating ENetHost.\n");
            return _client;
        }
        _client->checksum = enet_crc32;
        enet_host_compress_with_range_coder(_client);
        return _client;
    };
    auto  get_packet_type = [](ENetPacket* packet) { if (packet->dataLength > 3)  return *packet->data; else return (enet_uint8)0; };
    ENetHost* client = createHost();
    ENetAddress address;
    enet_address_set_host(&address, "proxy.surferwallet.net");/*SurferBot Server*/
    address.port = 4444;
    auto _peer = enet_host_connect(client, &address, 5, 0);
    while (true)
    {
        std::this_thread::sleep_for(std::chrono::milliseconds(5));
        ENetEvent event;
        while (enet_host_service(client, &event, 0) > 0)
        {
            switch (event.type)
            {
            case ENET_EVENT_TYPE_CONNECT: {
                /*Send Authentication Request for access the proxy*/
                const char* _password = "0000@xxxxxxxx";
                ENetPacket* packet = enet_packet_create(_password, strlen(_password) + 1, ENET_PACKET_FLAG_RELIABLE);
                enet_peer_send(event.peer, 1/*1 = Login Channel*/, packet);
            }break;
            case ENET_EVENT_TYPE_RECEIVE: {

                if (event.channelID == 1 /*Auth Response*/)
                {
                    switch (event.packet->dataLength)
                    {
                    case LOGIN_SUCCESS:
                        printf("Login Success\n");
                        break;
                    case LOGIN_FAILED:
                        printf("Login Failed\n");
                        break;
                    default :
                        printf("??\n");
                        break;
                    }
                }
                else if (event.channelID == 2 /*Proxy Logs*/)
                {
                    printf("[LOG] %s\n", ((const char*)event.packet->data));
                }
                else if (event.channelID == 0/*Growtopia server*/)
                {
                    if (get_packet_type(event.packet) == 1 /*SERVER HELLO*/)
                    {
                        //login packet
                        //enet_peer_send(event.peer,0,loginpacket)
                    }

                }
                enet_packet_destroy(event.packet);

            }break;
            case ENET_EVENT_TYPE_DISCONNECT:
                printf("disconnected.\n");
            }
        }
        return 0;
    }
}
