#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <sys/sendfile.h>
#include <fstream>
#include <iostream>
#include <cstring>

using namespace std;

static const string SERVER_IP = "127.0.0.1";
static const string LIST_PEER = "peer.txt";

void relay2client(int sockfd, struct sockaddr_in *client_addr){
	// Send the peer.txt file to client
	// Sending response to client after fetching it's IP and PORT
	char ip[INET_ADDRSTRLEN];
	uint16_t port;

	inet_ntop(AF_INET, &(client_addr->sin_addr), ip, sizeof(ip));
	port = htons(client_addr->sin_port);

	cout << "Client " << ip << ":" << port << " accepted\n";

	char buffer[256];
	ifstream in(LIST_PEER, ifstream::in);

	int line_cnt = 0;
	while(in.getline(buffer, 256, '\n'))
		line_cnt ++;

	write(sockfd, &line_cnt, sizeof(int));
	in.clear();
	in.seekg(0);

	while(in.getline(buffer, 256, '\n')){
		write(sockfd, &buffer, sizeof(buffer));
	}

	in.close();

	cout << "Peer data transmitted!\n";
	close(sockfd);	
}

void relay2peer(int sockfd, struct sockaddr_in *peer_addr){
	// Writing the IP and PORT of the peer to the peer.txt file
	ofstream out(LIST_PEER, ofstream::out | ofstream::app);
	
	// Fetching the IP and PORT from the peer sockaddr_in
	char ip[INET_ADDRSTRLEN];
	uint16_t port;

	inet_ntop(AF_INET, &(peer_addr->sin_addr), ip, sizeof(ip));
	port = htons(peer_addr->sin_port);

	char peer_ip[INET_ADDRSTRLEN] = {'\0'};
	int peer_port = 0;

	read(sockfd, &peer_ip, sizeof(peer_ip));
	read(sockfd, &peer_port, sizeof(peer_port));

	// Writing the same
	out << peer_ip << " " << peer_port << "\n";
	out.close();

	// // Sending the response to the peer that connection is successfull and the IP and PORT is saved
	// string response = "response:server\nstatus:connected\nport:" + to_string(port);
	// int bytes_send = 0;
	// if((bytes_send = send(sockfd, response.c_str(), response.length()*sizeof(char), 0)) < 0){
	// 	cout << "Error in sending response to peer!\n";
	// 	exit(0);
	// }

	cout << "Peer socket " << peer_ip << ":" << peer_port << " registered in relay server.\n";

	close(sockfd);
}

void decide(int sockfd, struct sockaddr_in *client_addr){
	char buffer[256];

	// Receiving the message from the client
	read(sockfd, buffer, sizeof(buffer));

	// request:node -> request is coming from peer
	// request:client -> request is coming from client
	if(strcmp(buffer,"request:node") == 0)
		relay2peer(sockfd, client_addr);
	else if(strcmp(buffer,"request:client") == 0)
		relay2client(sockfd, client_addr);
	else
		cout << "Foreign request!\n";
}

int main(int argc, char *argv[]){
	// Create socket
	int sockfd = 0;
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
		cout << "Error!\n";
		exit(0);
	}
	else{
		cout << "Socket created successfully.\n";
	}

	// Binding the socket to IP(localhost) and PORT(provided by user)
	// Setting up the address and getting the port from argument list
	int server_port = stoi(string(argv[1]));
	struct sockaddr_in server_addr;
	server_addr.sin_family = AF_INET;
	server_addr.sin_addr.s_addr = inet_addr(SERVER_IP.c_str());
	server_addr.sin_port = htons(server_port);

	if(bind(sockfd, (struct sockaddr*) &server_addr, sizeof(server_addr)) < 0){
		cout << "Binding Error!\n";
		exit(0);
	}
	
	// Listen for incoming messages
	if(listen(sockfd, 5) < 0){
		cout << "Listening Error!\n";
		exit(0);
	}
	else{
		cout << "Relay server is listening on port " << server_port << "\n";
	}

	listen:
	int client_sockfd = 0, connection_cnt = 0;
	struct sockaddr_in client_addr;
	socklen_t client_addr_size = sizeof(client_addr);
	
	if((client_sockfd = accept(sockfd, (struct sockaddr*) &client_addr, &client_addr_size)) < 0){
		cout << "Accept failed!\n";
		exit(0);
	}
	else{
		connection_cnt ++;
		cout << "Connection " << connection_cnt << " accepted from peer.\n";
	}

	
	// Function thread that takes care of further incoming connections
	int pid = fork();
	if(pid == 0)
		// Decide which operation to do based on the type of request incoming from the clients
		decide(client_sockfd, &client_addr);
	else
		goto listen;

	// Free socket
	close(sockfd);
	return 0;
}