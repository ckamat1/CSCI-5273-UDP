
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <signal.h>
#include <stdio.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/time.h>
#include <stdlib.h>
#include <memory.h>
#include <errno.h>
#include <iostream>
#include <string>
#include <climits>

#define MAXBUFSIZE 1024

using namespace std;
/* You will have to modify the program below */
typedef struct fam
{
unsigned char data[1024];
}Packet;

typedef struct frame
{
int frame_id;
int seq_no;
int ack;
size_t length;
Packet packet;
}Frame;




void handleResponse(char* buffer);

int receive(int &sock, struct sockaddr_in* remote, string &fname)
{
    unsigned int remote_len = sizeof(remote);
    FILE* fp;
    Frame recv_frame, ack_frame;
    int recvbytes;
    if(!(fp = fopen(fname.c_str(),"wb")))
    {
        perror("fopen: ");
        return 0;
    }
    ack_frame.ack = -1;
    int nRead = 0;
    fcntl(sock,F_SETFL,O_NONBLOCK);
    while(1)
    {
        memset(&recv_frame,0,sizeof(recv_frame));
        cout << "receive frame initialized to " << recv_frame.seq_no << endl;
        recvbytes = recvfrom(sock,&recv_frame,sizeof(recv_frame),0,(struct sockaddr *)remote,&remote_len);
        if (recvbytes == -1)
            continue;
        cout << "Received frame is " << recv_frame.seq_no << endl;
        //cout << "Size of data is" << strlen(recv_frame.packet.data) << endl;
        //nRead += strlen(recv_frame.packet.data);
        if(recv_frame.seq_no == -10)
        {
            cout << "file does not exist! " << endl;
            return 0;
        }
        if(recvbytes == 0)
        {
            break;
        }


        cout << "Received bytes: " << recvbytes << endl;
        cout <<"ack+1 : " << ack_frame.ack + 1 << endl;
        if((recv_frame.seq_no == ack_frame.ack+1))
            {
                memset(&ack_frame,0,sizeof(ack_frame));
                fwrite(recv_frame.packet.data,1,recv_frame.length,fp);
                ack_frame.ack = recv_frame.seq_no;
                int n = sendto(sock, &ack_frame, sizeof(ack_frame),0,(struct sockaddr*)remote, remote_len);
                cout << "ACK sent to server is  " << ack_frame.ack << endl;
            }
        else
            {
                if(recv_frame.seq_no < ack_frame.ack+1 )
                    {
                        int nack_size = sendto(sock, &ack_frame, sizeof(ack_frame),0,
                                               (struct sockaddr*)remote, remote_len);
                        cout << "Resending ACK: " << ack_frame.ack << endl;
                    }
            }

    }
    memset(&recv_frame,'0',sizeof(recv_frame));
    cout << " File received from server: "<< fname << endl;// " and total bytes written is "<< nRead <<endl;
    fclose(fp);
    return 1;

}




int main (int argc, char * argv[])
{

	int nbytes;                             // number of bytes send by sendto()
	int sockfd;                               //this will be our socket
	char buffer[MAXBUFSIZE];
    Frame recv_frame, frame;
	struct sockaddr_in remote;
	            //"Internet socket address structure"

	if (argc < 3)
	{
		cout << "USAGE:  <server_ip> <server_port>" << endl;
		exit(1);
	}

	/******************
	  Here we populate a sockaddr_in struct with
	  information regarding where we'd like to send our packet
	  i.e the Server.
	 ******************/
	bzero(&remote,sizeof(remote));               //zero the struct
	remote.sin_family = AF_INET;                 //address family
	remote.sin_port = htons(atoi(argv[2]));      //sets port to network byte order
	remote.sin_addr.s_addr = inet_addr(argv[1]); //sets remote IP address

	//Causes the system to create a generic socket of type UDP (datagram)
	if ((sockfd = socket(AF_INET,SOCK_DGRAM,0)) < 0)
	{
		perror("unable to create socket");
	}

	/******************
	  sendto() sends immediately.
	  it will report an error if the message fails to leave the computer
	  however, with UDP, there is no error if the message is lost in the network once it leaves the computer.
	 ******************/
	string command;
	struct sockaddr_in from_addr;
	char *c;
	unsigned int addr_length = sizeof(struct sockaddr);
	unsigned int len = sizeof(remote);
	string delimiter = " ";
    size_t pos;
    string token_command ;
    string fname ;
    int bsize;
    char buff[1024];
    FILE* file;
	for(;;){
        cout<<"Enter a request to send to the server"<<std::endl;
        cin.clear();
        getline(cin, command);
        c = &command[0];
        pos = command.find(delimiter);
    if(pos == string::npos)
        {
            if ((nbytes = sendto(sockfd,c,command.length(),0,(struct sockaddr*)&remote,sizeof(remote))) == -1)
            {
                perror("Error sending command, please try again!");
            }

            else
                {
                    cout<< "Command sent to server and command is " << command <<endl;
                    cout << "Number of bytes sent:" << command.length() << endl;
                    if(command == "exit")
                    {
                        cout << "Server has been terminated. Please restart the server before sending any command!" << endl;

                    }
                    cout << "Waiting for a response from server" << endl;
                    bzero(buffer,sizeof(buffer));
                    nbytes = recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr*)&from_addr,&addr_length);
                    handleResponse(buffer);
                }
        }

        else{
        token_command = command.substr(0, pos);
        fname = command.substr(pos+1);
        //:cout << "token command is " << token_command << "and filename is" << fname;
        if(token_command == "put"){
                if(!(file = fopen(fname.c_str(),"rb")))
            {
                perror("Error:");
                continue;
            }
            if ((nbytes = sendto(sockfd,c,command.length(),0,(struct sockaddr*)&remote,sizeof(remote))) == -1){
                perror("Error sending command, please try again!");
        }
            memset(buff, '0',1024);



            cout << "opened file successfully!";
        fseek(file, 0, SEEK_END);
        bsize = ftell(file);
        cout << "Total file size is " << bsize << endl;
             fseek(file, 0, SEEK_SET);
             int frme_id = 0;
             size_t n;
             fcntl(sockfd,F_SETFL,O_NONBLOCK);
             while(1)
             {
                  frame.seq_no = frme_id;
                  cout << "frame sequence number: " << frame.seq_no << endl;
                  n = fread(frame.packet.data,1,1024,file);
                  cout << "Bytes read from read: " << n  << endl;
                  frame.length = n;
                  if(n < 1024)
                  {
                    resend: int numbytes = sendto(sockfd, &frame,sizeof(frame),0,(struct sockaddr*)&remote, len);
                            cout << "Frame sent: " << frame.seq_no << endl;
                  //cout << "Sent " << strlen(frame.packet.data) << " bytes" << endl;
                  int recvbytes = recvfrom(sockfd, &recv_frame, sizeof(recv_frame),0,(struct sockaddr*)&remote,&len);
                  if (recvbytes > 0){
                    if(recv_frame.ack == frme_id){
                        cout << "ack received: " << recv_frame.ack << endl;
                        int numbytes = sendto(sockfd, &frame,0,0,(struct sockaddr*)&remote, len);
                        cout << "File sent from client is: " << fname << endl;
                        fclose(file);
                        break;
                    }}
                    else
                        {
                        sleep(0.25);
                        goto resend;
                        }


                  }
                  int numbytes = sendto(sockfd, &frame,sizeof(frame),0,(struct sockaddr*)&remote, len);
                  cout << "Frame sent: " << frame.seq_no << endl;
                  //cout << "Sent %d bytes from file" << numbytes << endl;
                  memset(&frame,'0',sizeof(frame));
                  int recvbytes;
                  while((recvbytes = recvfrom(sockfd, &recv_frame, sizeof(recv_frame),0,(struct sockaddr*)&remote,&len))==-1);
                     if (recvbytes > 0)
                        {
                        if(recv_frame.ack == frme_id)
                            {
                                cout << "ack received: " << recv_frame.ack << endl;
                                frme_id++;
                            }
                            else
                            {
                                frme_id = recv_frame.ack + 1;
                                cout << "came to else block and ack is" << recv_frame.ack;
                                fseek(file,frme_id * 1024,SEEK_SET);
                            }
                        }

            }

        }
        else if(token_command == "get" || token_command == "delete")
            {
                int ret;
                if ((nbytes = sendto(sockfd,c,command.length(),0,(struct sockaddr*)&remote,sizeof(remote))) == -1)
                    {
                        perror("Error sending command, please try again!");
                        continue;
                    }
                if(token_command == "get")
                {
                    if(!(ret = receive(sockfd,&remote,fname)))
                        {
                            continue;
                        }
                        else
                        {
                            cout << "Successfully received file: " << fname << endl;
                        }
                }
                else
                {
                    bzero(buffer,sizeof(buffer));
                    nbytes = recvfrom(sockfd,buffer,sizeof(buffer),0,(struct sockaddr*)&remote,&len);
                    handleResponse(buffer);
                }
            }
        }
	}
close(sockfd);
}
	// Blocks till bytes are received




void handleResponse(char* buffer)
{
    cout << "Server says - " << buffer << endl;
}
