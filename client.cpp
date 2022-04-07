#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char *argv[]){
	int sockfd = 0;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Error!\n";
		exit(0);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(stoi(string(argv[2])));

	if(connect(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		exit(0);
	}

	char request[256] = "request:client";

	write(sockfd, &request, sizeof(request));

	char buffer[256]={'\0'};
	ofstream out("test_recv.txt", ofstream::out | ofstream::app);
	// out << fflush;

	int line_cnt;
	read(sockfd, &line_cnt, sizeof(int));

	while(line_cnt --){
		read(sockfd, buffer, sizeof(buffer));
		out << buffer << "\n";
	}

	cout << "File received!\n";
	close(sockfd);
	return 0;
}