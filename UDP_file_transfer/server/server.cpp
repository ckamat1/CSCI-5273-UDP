#include <sys/types.h>
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
#include <string.h>
#include <iostream>
#include <dirent.h>
#include <fstream>
/* You will have to modify the program below */

#define MAXBUFSIZE 100

using namespace std;


typedef struct fam
{
 char data[1024];
}Packet;

typedef struct frame
{
 int frame_id;
 int seq_no;
 int ack;
 Packet packet;
}Frame;





string listDirectory(){
DIR *dir;
struct dirent *ent;
std::string message;
if ((dir = opendir ("/home/user")) != NULL) {
  /* print all the files and directories within directory */
  while ((ent = readdir (dir)) != NULL) {
    message += ent->d_name;
    message += " ";
  }
  closedir (dir);
  return message;
}

return "no files";
}

void handleRequest(int sock, char *buffer, struct sockaddr_in* remote, unsigned int remote_length){
cout << "handling request" << endl;
int nbytes;
std::string request = std::string(buffer);
std::string delimiter = " ";
std::string command;
Frame frame,ack_frame,recv_frame;
bzero(&ack_frame,sizeof(ack_frame));
Packet packet;
int frme_id = 0;
std::string default_message("your request could not be understood. Please retry with a valid request");



if(request.find(delimiter) == std::string::npos){
    if(request == "ls"){
        std::cout << "Checking my local folder..." << endl;
        std::string message = listDirectory();
        sendto(sock,&message[0],message.length(),0,(struct sockaddr*)remote,remote_length);
        return;
    }
    else if(request == "exit"){
        cout << "Server terminating.. cya soon!" << endl;
        exit(0);
    }
    else{
        sendto(sock,&default_message[0],default_message.length(),0,(struct sockaddr*)remote, remote_length);
    }
}
else{
    std::size_t pos = request.find(delimiter);
    std::string token_command = request.substr(0, pos);
    std::string fname = request.substr(pos+1);
    FILE* file;
    int recvbytes;
    if(token_command == "put"){
        if((file = fopen(fname.c_str(), "wb")) == NULL){
            perror("Error:");
            return;
        }
        cout << "File opened for writing" << endl;
        ack_frame.ack = -1;
        while(1)
          {
                    memset(&recv_frame,0,sizeof(recv_frame));
                    cout << "receive frame initialized to " << recv_frame.seq_no << endl;
                    recvbytes = recvfrom(sock, &recv_frame,sizeof(recv_frame),0,(struct sockaddr *)remote, &remote_length);
                    if(recvbytes == 0)
                        break;
                    cout << "received bytes: " << recvbytes;
                    cout << "Received frame is " <<  recv_frame.seq_no << endl;
                    //printf("ack+1 : %d", (ack_frame.ack) + 1);
                          if(recvbytes > 0 && (recv_frame.seq_no == ack_frame.ack+1))
                          {
                                memset(&ack_frame,0,sizeof(ack_frame));
                                fwrite(recv_frame.packet.data,1,(sizeof(recv_frame.packet.data)),file);
                                cout << "Bytes received from client is " <<  recvbytes << endl;
                                ack_frame.ack = recv_frame.seq_no;
                                int n = sendto(sock, &ack_frame, sizeof(ack_frame),0,(struct sockaddr*)remote, remote_length);
                                cout << "ACK sent to client is " << ack_frame.ack << endl;
                           }
                           else{
                                if(recv_frame.seq_no < ack_frame.ack+1 ){
                                        sleep(2);
                                        if(recv_frame.seq_no == ack_frame.ack+1)
                                            continue;
                                        int nack_size = sendto(sock, &ack_frame, sizeof(ack_frame),0,(struct sockaddr*)remote, remote_length);
                                        cout << "Resending ACK:  " << ack_frame.ack << endl;

                                }


                           }

             }
                            memset(&recv_frame,'0',sizeof(recv_frame));
                            cout << "File received from client" << endl;
                            fclose(file);




    }
    else if(token_command == "get")
    {
        if(!(file = fopen(fname.c_str(),"rb")))
               perror("Error:");
        cout << "opened file successfully!" << endl;
        fseek(file, 0, SEEK_END);
        int bsize = ftell(file);
        cout << "Total file size is " << bsize << endl ;
        fseek(file, 0, SEEK_SET);
         while(!feof(file))
         {

              frame.seq_no = frme_id;
              fread(frame.packet.data,1,1024,file);
              int numbytes = sendto(sock, &frame,sizeof(frame),0,(struct sockaddr*)remote, remote_length);
              cout << "Frame sent: " << frame.seq_no << endl;
              cout << "Sent " << numbytes << " bytes" << endl;
              memset(&frame,'0',sizeof(frame));

              //sleep(1);
              int recvbytes = recvfrom(sock, &recv_frame, sizeof(recv_frame),0,(struct sockaddr*)remote,&remote_length);
                 if (recvbytes > 0){
                    if(recv_frame.ack == frme_id){
                        cout << "ack received: " << recv_frame.ack << endl;
                        frme_id++;
                    }
                    else
                        {
                        frme_id = recv_frame.ack + 1;
                        fseek(file,frme_id * 1024,SEEK_SET);
                        }

                    }

                 }


         int numbytes = sendto(sock, &frame,0,0,(struct sockaddr*)remote, remote_length);
         cout << "File sent from client is: " << fname << endl;
         fclose(file);
    }
    else if(token_command == "delete")
    {
        string message;
        if(!(remove(fname.c_str())))
        {
            message = "File " + fname + "was deleted successfully!";
        }
        else
        {
            message = "File" + fname + "could not be deleted or does not exist!";
        }
        sendto(sock,&message[0],message.length(),
                   0,(struct sockaddr*)remote,remote_length);

    }
    else
    {
        sendto(sock,&default_message[0],default_message.length(),0,(struct sockaddr*)remote, remote_length);
    }


}

}

int main (int argc, char * argv[] )
{


	int sock;                           //This will be our socket
	struct sockaddr_in sin, remote;     //"Internet socket address structure"
	unsigned int remote_length;         //length of the sockaddr_in structure
	int nbytes;                        //number of bytes we receive in our message
	char buffer[1024];             //a buffer to store our received message
	if (argc != 2)
	{
		cout <<  "USAGE:  <port>" << endl;
		exit(1);
	}

	/******************
	  This code populates the sockaddr_in struct with
	  the information about our socket
	 ******************/
	bzero(&sin,sizeof(sin));                    //zero the struct
	sin.sin_family = AF_INET;                   //address family
	sin.sin_port = htons(atoi(argv[1]));        //htons() sets the port # to network byte order
	sin.sin_addr.s_addr = INADDR_ANY;           //supplies the IP address of the local machine


	//Causes the system to create a generic socket of type UDP (datagram)
	if (((sock = socket(AF_INET,SOCK_DGRAM,0)) < 0))
	{
		cout << "unable to create socket" << endl;
	}


	/******************
	  Once we've created a socket, we must bind that socket to the
	  local address and port we've supplied in the sockaddr_in struct
	 ******************/
	if (bind(sock, (struct sockaddr *)&sin, sizeof(sin)) < 0)
	{
		cout << "unable to bind socket" << endl;
		perror("bind failed");
		exit(-1);
	}



	//waits for an incoming message
	remote_length = sizeof(remote);

	for(;;)
    {
    bzero(buffer,sizeof(buffer));
	std::cout<<"\nWaiting for an incoming connection"<<std::endl;
	nbytes = recvfrom(sock,buffer,sizeof(buffer),0,(struct sockaddr*)&remote,&remote_length);

    cout<<"The length of the message received is "<< nbytes <<" bytes and " << "the client says " << buffer << endl;
	handleRequest(sock, buffer, &remote, remote_length);
	}
    close(sock);
return 0;
}




