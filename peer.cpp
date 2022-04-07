#include <arpa/inet.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

int main(int argc, char const *argv[])
{
	int peer_sockfd = 0;
	if((peer_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Error!\n";
		exit(0);
	}

	char peer_ip[INET_ADDRSTRLEN];
	cout << "Enter IP address of current peer: ";
	cin >> peer_ip;

	string peer_port;
	cout << "Enter port no. of current peer: ";
	cin >> peer_port;

	struct sockaddr_in peer_addr;
	peer_addr.sin_family = AF_INET;
	peer_addr.sin_addr.s_addr = inet_addr(peer_ip);
	peer_addr.sin_port = htons(stoi(peer_port));

	if(bind(peer_sockfd, (struct sockaddr*) &peer_addr, sizeof(peer_addr)) < 0){
		cout << "Error!\n";
		exit(0);
	}

	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(argv[1]);
	server_addr.sin_port = htons(stoi(string(argv[2])));

	if(connect(peer_sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		exit(0);
	}

	char request[256] = "request:node";

	write(peer_sockfd, &request, sizeof(request));
	cout<<"Sent request to server.\n";

	close(peer_sockfd);

	return 0;
}