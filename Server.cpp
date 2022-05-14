// ------------------------------ Program 4: Client-Server Model --------------------------------
// Helen Hu
// CSS 503B
// Creation Date: 6/1/2021
// Date of Last Modification: 6/6/2021
//
/*
 * Purposes: (1) to utilize various socket - related system calls,
 *           (2) to create a multi - threaded server and
 *           (3) to evaluate the throughput of different mechanisms when using TCP / IP
 *           do point - to - point communication over a network.
*/
/*
 * Description: In this program I will use the client-server model where a client process
 *              establishes a connection to a server, sends data or requests, and closes the connection.
 *              The server will accept the connection and create a thread to service the request and
 *              then wait for another connection on the main thread.  Servicing the request consists of
 *              (1) reading the number of iterations the client will perform, (2) reading the data sent by the client,
 *              and (3) sending the number of reads which the server performed.
*/

// ------------------------------------ Server.cpp -------------------------------------------------
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<pthread.h>
#include<unistd.h>
#include<fcntl.h>
#define BUFSIZE 1500

using namespace std;

// this thread function accept a new connection.
// and create a thread to service that request
void* threadFunc(void *arg){
    int *conn=(int*)arg;// change void type to int type
    char databuf[BUFSIZE];// stores the data it receives 
    char buf[100];// initialize buf
    int repetition; // the repetition of receiving a set of data buffers
    int reads=0;// initialize reads as 0
    int total=0;// total bytes it receives
    int flag;// check if the loop should be terminated

    memset(&buf,0,sizeof(buf));// initialize buffer as 0
    read(*conn,buf,sizeof(buf));//
    repetition=atoi(buf);
    cout<<"Get repetition: "<<repetition<<endl;// print get repetition
    cout<<"Receive data..."<<endl;// print receive data
    flag=repetition;

    // check the repetition and total number of bytes it receives
    while (flag-- && total<repetition*BUFSIZE){
        int size=0;// # of bytes it reads everytime 
        int t=1;

        // keep reading until the full size of the buffer
        while(size<BUFSIZE && t>0){
            t=read(*conn,databuf,BUFSIZE);
            size+=t;
            reads++;// increment by 1
           
            if (total+size==repetition*BUFSIZE) break;
        }
        total+=size;
        
    }
    cout<<"Receive compelete."<<endl;// signal receive complete
    memset(buf,0,sizeof(buf));
    snprintf(buf,sizeof(buf),"%d",reads);// put reads in the buffer
    cout<<"Read times: "<<reads<<endl;
    write(*conn,buf,sizeof(buf));// sent the data to the client
    close(*conn);// close the thread
    return 0;
}

// create the socket
void startServer(int port){
    // initialize listenfd, including domain, type and protocol
    int listenfd=socket(AF_INET,SOCK_STREAM,IPPROTO_TCP);
    if (listenfd==-1){
        perror("Error: socket");
        return;
    }

    // use the Set the SO_REUSEADDR option.This option is useful to prompt OS to release the server port 
    // as soon as your server process is terminated
    const int on=1;
    setsockopt(listenfd,SOL_SOCKET,SO_REUSEADDR,(char*)&on,sizeof(int));

    sockaddr_in addr;
    memset(&addr,0,sizeof(addr));
    addr.sin_family=AF_INET;
    addr.sin_port=htons(port);
    addr.sin_addr.s_addr=INADDR_ANY;

    // bind this socket to a local address 
    if (bind(listenfd,(sockaddr*)&addr,sizeof(addr))==-1){
        perror("Error: bind");
        return;
    }

    // tell the server how many concurrent connections to listen for 
    if (listen(listenfd,5)==-1){
        perror("Error: listen");
        return;
    }

    int conn;// intialize conn to record the connections from the server
    char clientIP[INET_ADDRSTRLEN]="";// initialize the client IP
    sockaddr_in clientAddr;// initialize the client address
    socklen_t clientAddrLen=sizeof(clientAddr);// intialize the client address length
    char buf[BUFSIZE];

    cout<<"listening on port: "<<port<<endl;// signal listening on port

    // accept() system call will accept the call from the client and return a socket from which to read and write
    while (true){
        cout<<"listening..."<<endl;// signal starting listening
        conn=accept(listenfd,(sockaddr*)&clientAddr,&clientAddrLen);

        // error checking
        if (conn<0){
            perror("Error: accept");
            continue;
        }
        
        inet_ntop(AF_INET,&clientAddr.sin_addr,clientIP,INET_ADDRSTRLEN);
        cout<<"accept cllient "<<clientIP<<":"<<ntohs(clientAddr.sin_port)<<endl;// print accept connection 
        
        pthread_t pid;// create a pthread
        int ret;
        ret=pthread_create(&pid,NULL,threadFunc,(void*)&conn);
        if (ret!=0){
            perror("Error: pthread");
            continue;
        }
        pthread_detach(pid); // terminate the thread
    }
}

// only need to accept port number, therefore , # of argc is 2
int main(int argc,char** argv){
    // check if argc equal to 2, if not, print error message
    if (argc!=2){
        printf("usage:server port\n");
        return -1;
    }
    int port=atoi(argv[1]);// change the port to integer type
    
    startServer(port);// start the server
    return 0;
}