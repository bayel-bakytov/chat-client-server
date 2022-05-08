#include <iostream>
#include <WS2tcpip.h>
#include <string>
#include <sstream>
#include <vector>
#pragma comment (lib, "ws2_32.lib")
// # id
// @ nick

using namespace std;
class ClientsList {
public:
	string login;
	string password;
	int _id;
};

class roomsList {
public:
	int id;
	string chat;
};

vector<ClientsList> clientsList;

string mainChat = "";

int getId(string msg) {
	int start = 0;
	int end = msg.find("#");
	int id = stoi(msg.substr(start, end - start));
	return id;
}

//string getNick(SOCKET sock) {
//	string nick;
//	cout << sock;
//	for (int i = 0; i < clientsList.size(); i++) {
//		if (clientsList[i]._id == sock) {
//			return clientsList[i].login;
//		}
//	}
//	return nick;
//}

string getNick(string msg) {
	int start = msg.find("@")+1;
	string nick = msg.substr(start, msg.length());
	return nick;
}

string getLogin(string msg) {
	int start = msg.find("@") + 1;
	int end = msg.find("*");
	string login = msg.substr(start, end - start);
	return login;
}

string getMessage(string msg, bool isToAll) {
	int start = msg.find("#") + 1;
	if (isToAll) {
		start = 0;
	}
	int end = msg.find("@");
	string m = msg.substr(start, end - start);
	return m;
}

string getPassword(string msg) {
	int start = msg.find("*") + 1;
	string password = msg.substr(start, msg.length());
	return password;
}

void sendToAllClients(string msg, fd_set master) {
	for (int i = 0; i < master.fd_count; i++) {
		SOCKET outSock = master.fd_array[i];
		send(outSock, msg.c_str(), msg.size() + 1, 0);
	}
}

void sendToClient(SOCKET sock, string msg) {
	send(sock, msg.c_str(), msg.size() + 1, 0);
}

void syncListClients(SOCKET sock) {
	ostringstream ss;
	for (int i = 0; i < clientsList.size(); i++) {
		ss << "@" + clientsList[i].login;
	}
	sendToClient(sock, ss.str());
}

void newClient(string msg, SOCKET sock) {
	ClientsList cl;
	cl.password = getPassword(msg);
	cl.login = getLogin(msg);
	cl._id = sock;
	clientsList.push_back(cl);
}

bool checkAuth(string s) {
	string log = getLogin(s);
	string pass = getPassword(s);
	bool exist = false;
	for (int i = 0; i < clientsList.size(); i++) {
		if (clientsList[i].login == log && clientsList[i].password == pass) {
			exist = true;
			break;
		}
	}
	return exist;
}



int main()
{
	// Initialze winsock
	WSADATA wsData;
	WORD ver = MAKEWORD(2, 2);

	int wsOk = WSAStartup(ver, &wsData);
	if (wsOk != 0)
	{
		cerr << "Can't Initialize winsock! Quitting" << endl;
		return 99;
	}

	// Create a socket
	SOCKET listening = socket(AF_INET, SOCK_STREAM, 0);
	if (listening == INVALID_SOCKET)
	{
		cerr << "Can't create a socket! Quitting" << endl;
		return 99;
	}

	// Bind the ip address and port to a socket
	sockaddr_in hint;
	hint.sin_family = AF_INET;
	hint.sin_port = htons(54000);
	hint.sin_addr.S_un.S_addr = INADDR_ANY; // Could also use inet_pton .... 

	bind(listening, (sockaddr*)&hint, sizeof(hint));

	// Tell Winsock the socket is for listening 
	listen(listening, SOMAXCONN);

	// Create the master file descriptor set and zero it
	fd_set master;
	FD_ZERO(&master);

	// Add our first socket that we're interested in interacting with; the listening socket!
	// It's important that this socket is added for our server or else we won't 'hear' incoming
	// connections 
	FD_SET(listening, &master);

	// this will be changed by the \quit command (see below, bonus not in video!)
	bool running = true;

	while (running)
	{
		// Make a copy of the master file descriptor set, this is SUPER important because
		// the call to select() is _DESTRUCTIVE_. The copy only contains the sockets that
		// are accepting inbound connection requests OR messages. 

		// E.g. You have a server and it's master file descriptor set contains 5 items;
		// the listening socket and four clients. When you pass this set into select(), 
		// only the sockets that are interacting with the server are returned. Let's say
		// only one client is sending a message at that time. The contents of 'copy' will
		// be one socket. You will have LOST all the other sockets.

		// SO MAKE A COPY OF THE MASTER LIST TO PASS INTO select() !!!

		fd_set copy = master;

		// See who's talking to us
		int socketCount = select(0, &copy, nullptr, nullptr, nullptr);

		// Loop through all the current connections / potential connect
		for (int i = 0; i < socketCount; i++)
		{
			// Makes things easy for us doing this assignment
			SOCKET sock = copy.fd_array[i];

			// Is it an inbound communication?
			if (sock == listening)
			{
				// Accept a new connection
				SOCKET client = accept(listening, nullptr, nullptr);

				// Add the new connection to the list of connected clients
				FD_SET(client, &master);

				// Send a welcome message to the connected client
				string welcomeMsg = "Hi! Welcome to general chat! " + to_string(client) + " is your id number!";
				sendToClient(client, welcomeMsg);
				
				syncListClients(client);
				sendToAllClients(mainChat, master);
				for (int i = 0; i < master.fd_count; i++) {
					cout << master.fd_array[i] << endl;
				}
				
				
			}
			else // It's an inbound message
			{
				char buf[4096];
				ZeroMemory(buf, 4096);

				// Receive message
				int bytesIn = recv(sock, buf, 4096, 0);
				if (bytesIn <= 0)
				{
					// Drop the client
					closesocket(sock);
					FD_CLR(sock, &master);
				}
				else
				{
					string s = string(buf, bytesIn);

					// if someone signs up
					if (buf[0] == '/') {
						newClient(s,sock);
						for (int i = 0; i < master.fd_count; i++) {
							syncListClients(master.fd_array[i]);
						}
					}
					//if someone signs in
					else if (buf[0] == '\\') {
						if (checkAuth(s)) {
							sendToClient(sock, "+@"+getLogin(s)+"*"+getPassword(s));
						}
						else {
							sendToClient(sock, "-");
						}
						
					}
					else if (buf[0] == '$') {
						for (int i = 0; i < clientsList.size(); i++) {
							if (clientsList[i].login == getLogin(s) && clientsList[i].password == getPassword(s)) {
								clientsList[i]._id == sock;
								sendToClient(sock, "%@"+clientsList[i].login);
								break;
							}
						}
						
					}
					else {
						// Send message to other clients, and definiately NOT the listening socket
						if (s.find("#") <= s.length()) {
							ostringstream ss;
							string msg = getMessage(s, false);
							int id = getId(s);
							string nick = getNick(s);
							ss <<nick << ": " << msg << "\r\n";
							string strOut = ss.str();
							
							sendToClient(id, strOut);
							
						}
						else {
							ostringstream ss;
							string msg = getMessage(s, true);
							string nick = getNick(s);
							ss << nick << ": " << msg << "\r\n";
							sendToAllClients(ss.str(), master);
							mainChat += "^" + ss.str();
							cout << mainChat;
						}
					}
				}
			}
		}
	}

	// Remove the listening socket from the master file descriptor set and close it
	// to prevent anyone else trying to connect.
	FD_CLR(listening, &master);
	closesocket(listening);

	// Message to let users know what's happening.
	string msg = "SERVER:Server is shutting down. Goodbye\r\n";

	while (master.fd_count > 0)
	{
		// Get the socket number
		SOCKET sock = master.fd_array[0];

		// Send the goodbye message
		send(sock, msg.c_str(), msg.size() + 1, 0);

		// Remove it from the master file list and close the socket
		FD_CLR(sock, &master);
		closesocket(sock);
	}

	// Cleanup winsock
	WSACleanup();

	system("pause");
	return 0;
}



