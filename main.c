#include <sstream>
#include <string>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <stdlib.h>
#include <stdio.h>
#include <fstream>
#include <iostream>
#include <ios>
#include <mutex>
#include <thread>
#include <queue>

#pragma comment(lib, "wsock32.lib")
#pragma comment(lib, "Ws2_32.lib")

using std::cin;
using std::cout;
using std::endl;
using namespace std;
bool _rnd;
SYSTEMTIME st;
std::mutex srv1;
stringstream ss;
std::queue<string> OutputStr;

void Help()
    {
        cout<<"USAGE:"<<endl;
        cout<<"[NetTest s port] for server mode."<<endl;
        cout<<"[NetTest c port IP string] for client mode."<<endl;
    }

void  ServerMode (int pn)
    {
        cout<<"Server mode port:"<<pn<<endl;
        SOCKET s;
        struct sockaddr_in server, si_other;
        int slen , recv_len,packetn,packetr;
        char buf[512];
        WSADATA wsa;

        slen = sizeof(si_other) ;

        //Initialise winsock
        printf("\nInitialising Winsock...");
        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
            printf("Failed. Error Code : %d",WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        printf("Initialised.\n");

        //Create a socket
        if((s = socket(AF_INET , SOCK_DGRAM , 0 )) == INVALID_SOCKET)
        {
            printf("Could not create socket : %d" , WSAGetLastError());
        }
        printf("Socket created.\n");

        //Prepare the sockaddr_in structure
        server.sin_family = AF_INET;
        server.sin_addr.s_addr = INADDR_ANY;
        server.sin_port = htons(pn);

        //Bind
        if( bind(s ,(struct sockaddr *)&server , sizeof(server)) == SOCKET_ERROR)
        {
            printf("Bind failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        puts("Bind done");

        //keep listening for data
        packetn = 0;
        while(1)
        {
            printf("Waiting for data...");
            fflush(stdout);

            //clear the buffer by filling null, it might have previously received data
            memset(buf,'\0', 512);

            //try to receive some data, this is a blocking call
            if ((recv_len = recvfrom(s, buf, 512, 0, (struct sockaddr *) &si_other, &slen)) == SOCKET_ERROR)
            {
                printf("recvfrom() failed with error code : %d" , WSAGetLastError());
                exit(EXIT_FAILURE);
            }

            //print details of the client/peer and the data received
            printf("Received packet from %s:%d\n", inet_ntoa(si_other.sin_addr), ntohs(si_other.sin_port));
            printf("Data: %s\n" , buf);
            if (strcmp(buf,"0 RND") == 0)
            {
                _rnd = true;
                cout<<"Entering random packet mode..."<<endl;
            }
            packetr = std::stoi(buf);
            if (packetn != packetr)
            {
                cout<<"Error!"<<packetn<<" ! "<<packetr<<endl;
                GetSystemTime (&st);// toLOG
/*                std::ofstream fe;
                fe.open(std::to_string(st.wDay) + std::to_string(st.wMonth) + std::to_string(st.wYear)+"err.txt", ios::app);
                fe<<to_string(st.wHour+3) + ":" + to_string(st.wMinute) + ":" + to_string(st.wSecond) + " " + to_string(st.wDay) + "." + to_string(st.wMonth) + "." + to_string(st.wYear) << " expected "<<to_string(packetn)<<" recieved "<<to_string(packetr)<<endl;
                fe.close(); */
                srv1.lock();
                ss<<to_string(st.wHour+3) + ":" + to_string(st.wMinute) + ":" + to_string(st.wSecond) + " " + to_string(st.wDay) + "." + to_string(st.wMonth) + "." + to_string(st.wYear) << " expected "<<to_string(packetn)<<" recieved "<<to_string(packetr)<<endl;
                OutputStr.push(ss.str());
                srv1.unlock();
                packetn=packetr;
            }
            //now reply the client with the same data
/*			if (sendto(s, buf, recv_len, 0, (struct sockaddr*) &si_other, slen) == SOCKET_ERROR)
            {
                printf("sendto() failed with error code : %d" , WSAGetLastError());
                exit(EXIT_FAILURE);
            } */
            packetn++;
            if (packetn == 30000) packetn = 0;
        }

        closesocket(s);
        WSACleanup();
    }

void ClientMode (char* pn,char* ip, char* sendstring)
    {
        if (strcmp(sendstring,"RND") == 0) _rnd = true;
        if (_rnd) cout<<"Entering random packet mode..."<<endl;
        cout<<"Client mode port:"<<pn<<" Addres:"<<ip<<" Str:"<<sendstring<<endl;
        struct sockaddr_in si_other;
        int l,s, slen=sizeof(si_other),packetn;
//        char buf[512];

        WSADATA wsa;

        // ??????? ?????????
        //printf("\nConecting to ");printf(argv[1]);printf(":");printf(argv[2]);
        cout<<"Conecting to "<<ip<<":"<<pn<<endl;

        //????????????? ?????? winsock
        printf("\nInitialising Winsock...");
        if (WSAStartup(MAKEWORD(2,2),&wsa) != 0)
        {
            printf("Failed. Error Code : %d",WSAGetLastError());
            exit(EXIT_FAILURE);
        }
        printf("Initialised.\n");

        //create socket
        if ( (s=socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP)) == SOCKET_ERROR)
        {
            printf("socket() failed with error code : %d" , WSAGetLastError());
            exit(EXIT_FAILURE);
        }

        //setup address structure
        memset((char *) &si_other, 0, sizeof(si_other));
        si_other.sin_family = AF_INET;
        si_other.sin_port = htons(atoi(pn));
        si_other.sin_addr.S_un.S_addr = inet_addr(ip);

        //start communication
        packetn = 0;
        while(1)
        {
            //		printf("Enter message : ");
            //		gets(message);

            // ????????? ????????????? ????????
            //char message[] = "FF95478754328794237623SF148794358935482D345FG82348734FE58935DA4902319872138734585682348723485438920189038FFFE45FGD";
            std::string sm = std::to_string(packetn) + " " + sendstring;
            const char *message = sm.c_str();
            l = strlen(message);
            //		printf("%d", l);


            //send the message
            if (sendto(s, message, strlen(message) , 0 , (struct sockaddr *) &si_other, slen) == SOCKET_ERROR)
            {
                printf("sendto() failed with error code : %d" , WSAGetLastError());
                exit(EXIT_FAILURE);
            }

            //receive a reply and print it
            //clear the buffer by filling null, it might have previously received data
/*			memset(buf,'\0', 512);
            //try to receive some data, this is a blocking call
            if (recvfrom(s, buf, 512, 0, (struct sockaddr *) &si_other, &slen) == SOCKET_ERROR)
            {
                printf("recvfrom() failed with error code : %d" , WSAGetLastError());
                exit(EXIT_FAILURE);
            }
            if (strcmp(sendstring,buf) == 0)
            {
                puts(sendstring);
                puts(buf);
                puts("OK");
                // ????? ??????
            }
            else
            {
            //			puts(l);
            //			puts(strlen(buf);
                puts(buf);
                puts(sendstring);
            }
*/
            // ?????? ????? ????? ?????????
            Sleep(1000);
            packetn++;
            if (packetn == 30000) packetn = 0;
        }

        closesocket(s);
        WSACleanup();
    }


int main(int argc, char** argv)
{
    _rnd = false;
    if (argc == 1)
    {
        cout<< "ERROR: No arguments"<<endl;
        Help();
        return 0;
    }
    if (atoi(argv[2]) < 0 || atoi(argv[2]) > 65000)
    {
        cout<<"ERROR: Port must be 1-65000"<<endl;
        return 0;
    }
    if (!strcmp(argv[1],"s"))
    {
        if (argc > 2)
        {
            std::thread thr1(ServerMode,atoi(argv[2]));
            std::thread thr2(ServerMode,atoi(argv[2])+1);
            thr1.detach();
            thr2.detach();
            while (1)
            {
                Sleep (1000);
                if  (!OutputStr.empty())
                {
                    srv1.lock();
                    std::ofstream fe;
                    fe.open(std::to_string(st.wDay) + std::to_string(st.wMonth) + std::to_string(st.wYear)+"err.txt", ios::app);
                    while (OutputStr.empty())
                    {
                        fe<<OutputStr.front();
                        OutputStr.pop();
                    }
                    fe.close();
                    srv1.unlock();
                }
            }
        }
        else
        {
            cout<<"ERROR: No port parameter for server mode"<<endl;
            return 0;
        }
    } else if (!strcmp(argv[1],"c"))
    {
        if (argc > 4) ClientMode (argv[2],argv[3],argv[4]); else
        {
            cout<<"ERROR: Not enough parameters for client mode"<<endl;
            return 0;
        }
    } else
    {
        cout<<"ERROR: Wrong argument"<<endl;
        Help ();
    }
    /*
    char input = 0;
    cout << "Hello! This is a console application." <<argc<< endl;
    cout << "Press q to quit, press a to execute foo." << endl;
    while(1) {
        cin >> input;
        if(input == 'a') {
            cout<<argv[2]<<endl;
        } else if(input == 'q') {
            break;
        } else if(input != '\n') {
            cout << "Unknown command '" << input << "'! Ignoring...\n";
        }*/

    return 0;
}


