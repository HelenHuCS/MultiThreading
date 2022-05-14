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

// ------------------------------------ Client.cpp -------------------------------------------------
#include<iostream>
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<netdb.h>
#include<sys/uio.h>
#include<chrono>

using namespace std;
using namespace chrono;

int main(int argc,char** argv){
    if (argc!=7){
        printf("usage: client server_name port repetition nbufs bufsize type");
        return -1;
    }

    char* serverName=argv[1]; // the name of the server
    int port=atoi(argv[2]);// the IP port number used by server (use the last 5 digits of your student id)
    int repetition=atoi(argv[3]);// the repetition of sending a set of data buffers
    int nbufs=atoi(argv[4]);// the number of data buffers
    int bufsize=atoi(argv[5]);// the size of each data buffer (in bytes)
    int type=atoi(argv[6]);// the type of transfer scenario: 1, 2, or 3
    int total=0;// total bytes it sends

    char databuf[nbufs][bufsize];// initialize the data we need to send
    memset(&databuf,0,nbufs*bufsize);

    // struct addrinfo
    addrinfo hints;
    addrinfo *res,*p;
    memset(&hints,0,sizeof(hints));// set to 0
    hints.ai_family=AF_UNSPEC;// Allow IPv4 or IPv6
    hints.ai_socktype=SOCK_STREAM;// initialize ai_socktype
   
    hints.ai_flags=AI_CANONNAME; // For wildcard IP address 
    // This field specifies the protocol for the returned socket addresses. 
    // Specifying 0 in this field indicates that socket addresses with any protocol can be returned by getaddrinfo().
    hints.ai_protocol=0;

    // the getaddrinfo() system call to get information about the server being used
    // error checking first when try to access the address info
    // if fail, print error message
    if (getaddrinfo(argv[1],argv[2],&hints,&res)!=0) {
        cout<<getaddrinfo(argv[1],argv[2],&hints,&res)<<endl;
        perror("Error: getaddrinfo");
        return -1;
    }

    // if succeed, need to iterate through all the elements stored in res
    for( p=res;p!=NULL;p=p->ai_next){
        int sockfd;// initialize socketfd
        struct sockaddr_in *addr=(struct sockaddr_in*)p->ai_addr;// struct address pointer

        // error checking first 
        if ((sockfd=socket(p->ai_family,p->ai_socktype,p->ai_protocol))==-1){
            perror("Error: socket");
            continue;
        }

        // error checking when initializing sockfd
        // if fail, print error message
        if (connect(sockfd,(struct sockaddr*)addr,sizeof(sockaddr_in))<0){
            perror("Error: connect");
            return -1;
        }
        char buf[100]="";// initialize buffer

        // if succeed, print the server ip address and corresponding port number
        cout<<"Connected to server: "<<inet_ntop(AF_INET,&addr->sin_addr,buf,16)<<":"<<ntohs(addr->sin_port)<<endl;
        memset(buf,0,sizeof(buf));// set to 0
        snprintf(buf,sizeof(buf),"%d",repetition);// change repetition from int type to string type and store it in the buffer
        cout<<"Send repetition: "<<buf<<endl;// signal send repetition
        write(sockfd,buf,sizeof(buf));// send the data back to the server
        
        auto start=steady_clock::now(); // record the time since start reading
        cout<<"Transport data..."<<endl;// signal transport data

        for (int i=0;i<repetition;i++){
            /* Multiple writes: invokes the write() system call for each data buffer, 
            *  thus resulting in calling as many write()s as the number of data buffers, 
            *  (i.e., nbufs).
            */
            if (type==1){
                for (int j=0;j<nbufs;j++){
                    total+=write(sockfd,databuf[j],bufsize);
                }
            }

            /* writev: allocates an array of iovec data structures, each having its* iov_base field point 
            *  to a different data buffer as well as storing the buffer size in its iov_len field;
            *  and thereafter calls writev() to send all data buffers at once.
            */ 
            else if(type==2){
                struct iovec vector[nbufs];
                for (int j=0;j<nbufs;j++){
                    vector[j].iov_base=databuf[j];
                    vector[j].iov_len=bufsize;
                }
                total+=writev(sockfd,vector,nbufs);
            }
            /* single write : allocates an nbufs - sized array of data buffers, 
             * and thereafter calls write() to send this array, (i.e., all data buffers) at once.
             */ 
            else if(type==3){
                total+=write(sockfd,databuf,nbufs*bufsize);
            }
        }
        
        cout<<"Transport complete."<<endl; // signal transport is complete.

        memset(buf,0,sizeof(buf));
        read(sockfd,buf,100);
        auto end = steady_clock::now();// record the time after reading

        int reads=atoi(buf);

        auto duration = duration_cast<milliseconds>(end - start);
       // print information about the test 
        cout << "Test (" << type << "): time = " << duration.count() << " usec, #reads = " << buf << ", throughput= " << total * 8 / 1000000.0 / duration.count() << " Gbps" << endl;
        close(sockfd);// close the socket
    }
    freeaddrinfo(res);// free the address info pointer
    
}
