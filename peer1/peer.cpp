#include <arpa/inet.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

void clientcall(int peer_sockfd)
{
	char request[256]={'\0'};
	int response_code = 0;
	read(peer_sockfd, &request, sizeof(request));

	ifstream in;

	in.open(string(request));

	if(in) {
		response_code=1;
		cout<<"File exists, copying file to client...\n";
		write(peer_sockfd, &response_code, sizeof(int));

		char buffer[256];

		int line_cnt = 0;
		while(in.getline(buffer, 256, '\n'))
			line_cnt ++;

		write(peer_sockfd, &line_cnt, sizeof(int));
		in.clear();
		in.seekg(0);

		while(in.getline(buffer, 256, '\n')){
			write(peer_sockfd, &buffer, sizeof(buffer));
		}

		in.close();
	}
	else {
		cout<<"File does not exist\n";
		write(peer_sockfd, &response_code, sizeof(int));
	}
}

int main(int argc, char const *argv[])
{
	int peer_sockfd = 0;
	if((peer_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Error!\n";
		exit(0);
	}

	// if(bind(peer_sockfd, (struct sockaddr*) &peer_addr, sizeof(peer_addr)) < 0){
	// 	cout << "Error!\n";
	// 	exit(0);
	// }

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
	
	char peer_ip[INET_ADDRSTRLEN] = {'\0'};
	cout << "Enter IP address of current peer: ";
	cin >> peer_ip;
	write(peer_sockfd, &peer_ip, sizeof(peer_ip));

	int peer_port;
	cout << "Enter port no. of current peer: ";
	cin >> peer_port;
	write(peer_sockfd, &peer_port, sizeof(peer_port));


	if(shutdown(peer_sockfd,0)<0) {
		cout<<"Cannot shut down\n";
		exit(0);
	}

	// close(peer_sockfd);

	//Peer acts as server

	int peer_server_sockfd = 0;
	if((peer_server_sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Socket Error!\n";
		exit(0);
	}

	struct sockaddr_in peer_server_addr;
	peer_server_addr.sin_family = AF_INET;
	peer_server_addr.sin_addr.s_addr = inet_addr(peer_ip);
	peer_server_addr.sin_port = htons(peer_port);

	if(bind(peer_server_sockfd, (struct sockaddr*) &peer_server_addr, sizeof(peer_server_addr)) < 0){
		cout << "Error!\n";
		// perror("Unable to bind!");
		exit(0);
	}

	if(listen(peer_server_sockfd, 5) < 0){
		cout << "Listening Error!\n";
		exit(0);
	}
	else{
		cout << "Peer is listening on port " << peer_port << "\n";
	}

	listen:
	int client_sockfd = 0, connection_cnt = 0;
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	
	if((client_sockfd = accept(peer_server_sockfd, (struct sockaddr*) &client_addr, &client_addr_size)) < 0){
		cout << "Accept failed!\n";
		exit(0);
	}
	else{
		connection_cnt ++;
		cout << "Connection " << connection_cnt << " accepted from client.\n";
	}

	// Function thread that takes care of further incoming connections
	int pid = fork();
	if(pid == 0)
		// Decide which operation to do based on the type of request incoming from the clients
		clientcall(client_sockfd);
	else
		goto listen;

	close(peer_sockfd);
	return 0;
}