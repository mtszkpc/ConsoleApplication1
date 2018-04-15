#pragma comment(lib, "ws2_32.lib")
#pragma warning(disable:4996) 
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <WinSock2.h>
#include <iostream>
#include <Ws2tcpip.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <cstdio>
#include <ctime>
#include <bitset>
#include <random>
#include <time.h>
#include <conio.h>

using namespace std;
//wektor polaczen
vector<SOCKET> Connections;
vector<int> sesyjki;
int nr_of_Connections = 0;

int id_sess_now;
SOCKET sListen;

int szukana_liczba;
int czas;
int ConnectionCounter = 0;
int winer = 0;

//struktura na ktorej opiera sie protokol
struct Prot {
	string operacja;
	string odpowiedz;
	int NSekwencyjny;
	string dane;
	int identyfikator;
	string czas;
};
//funkcja czasu
char * znacznik_czasu() {

	time_t czas;
	struct tm * ptr;
	time(&czas);
	ptr = localtime(&czas);
	char * data = asctime(ptr);

	return data;

}
//tworzenie wiadomosci serwera
char* Make_message(Prot p1, int& x) {
	string pom;


	if (p1.odpowiedz.size() > 0) {
		pom += p1.odpowiedz;
	}
	else if (p1.operacja.size() > 0) {
		pom += p1.operacja;
	}
	else {
		pom += p1.dane;
	}


	pom += "NSekwencyjny=" + to_string(p1.NSekwencyjny) + ";";
	pom = pom + "Identyfikator=" + to_string(p1.identyfikator) + ";";
	pom = pom + "Czas=" + p1.czas + ";";


	x = pom.length();

	char *tab = new char[(size(pom))];

	for (int i = 0; i < size(pom); i++) {
		tab[i] = pom[i];
	}

	return tab;
}
//tworzenie 2 wiadomosci serwera
char* Make_message2(Prot p1, int& x) {
	string pom;
	p1.NSekwencyjny = 1;

	string dlugosc;
	int dlugosc_int = p1.dane.size();
	if (dlugosc_int < 10) {
		dlugosc = "0" + to_string(dlugosc_int);
	}
	else {
		dlugosc = to_string(dlugosc_int);
	}
	pom = pom + dlugosc;
	pom = pom + to_string(p1.NSekwencyjny);
	pom = pom + p1.dane;
	pom = pom + to_string(p1.identyfikator);

	x = pom.length();

	char *tab = new char[(size(pom))];

	for (int i = 0; i < size(pom); i++) {
		tab[i] = pom[i];
	}

	return tab;
}
//wyznaczanie czasu rozgrywki
int wyznacz_czas() {
	return ((abs(sesyjki[0] - sesyjki[1]) * 74) % 90) + 25;
}

//nawiazywanie sesji
void send_sess_info(int i) {
	Prot p1;
	int kurde_dzialaj;

	p1.operacja = "Operacja=nawiazanie sesji;";
	kurde_dzialaj = (rand() % 89) + 10;
	sesyjki.push_back(kurde_dzialaj);
	p1.identyfikator = kurde_dzialaj;
	//p1.dane = "POLACZONO, TWOJE ID: " + to_string(kurde_dzialaj) + " ";
	p1.NSekwencyjny = 0;
	p1.czas = znacznik_czasu();


	int x;

	char *tabelka = Make_message(p1, x);

	//cout << "NAWIAZYWANIE SESJI Z " << i << " GRACZEM: " << tabelka << endl;


	send(Connections[i], tabelka, x, NULL);

}
//wysylanie do gracza o danym id wiadomosci
void send_message(int i, string s, int id) {

	Prot p1;

	p1.odpowiedz = s;
	p1.identyfikator = id;
	p1.NSekwencyjny = 0;
	p1.czas = znacznik_czasu();
	//p1.dlugosc_danych = s.size();



	int z, y;

	char *tabelka = Make_message(p1, z);


	//cout << "1. " << tabelka << " 2. " << " KONIEC DANYCH" << endl;

	send(Connections[i], tabelka, z, NULL);


}
//zakonczenie gry
void zakoncz_gre() {

	string wiad = "Operacja=koniec gry, przegrales;";

	for (int i = 0; i < ConnectionCounter; i++) {


		send_message(i, wiad, sesyjki[i]);
		//ExitProcess(i);
		//	closesocket(Connections[i]);
		//	WSACleanup();
	}

	if (winer == 0) {
		cout << endl << "koniec gierki lamusy" << endl;
	}
	else {
		cout << endl << "koniec gierki, wygral klient: " << winer << endl;
	}


	cin.ignore(2);
	exit(0);

}
//wysylanie czasu do klienta
void wyslij_czas(int &x, int i) {

	Prot p1;

	p1.operacja = "Operacja=czas pozostaly do konca gry;";

	//p1.dane = "POLACZONO, TWOJE ID: " + to_string(kurde_dzialaj) + " ";
	p1.NSekwencyjny = 1;
	p1.czas = znacznik_czasu();
	p1.identyfikator = sesyjki[i];

	int z;

	char *tabelka = Make_message(p1, z);

	//cout << endl << "INFO O CZASIE DO " << i + 1 << " GRACZA: ";
	//cout << x << endl << endl;
	send(Connections[i], tabelka, z, NULL);

	Prot p2;

	p2.dane = "Dane=" + to_string(x) + ";";

	p2.NSekwencyjny = 0;
	p2.czas = znacznik_czasu();
	p2.identyfikator = sesyjki[i];

	tabelka = Make_message(p2, z);

	send(Connections[i], tabelka, z, NULL);

}
//przetrzymywanie watku klienta
void ClientHandlerThread(int index) //index = the index in the SOCKET Connections array
{

	int bufferlength = 1024; //Holds the length of the message a client sent
	while (true)
	{

		char * buffer = new char[bufferlength]; //Allocate buffer
		recv(Connections[index], buffer, bufferlength, NULL); //get buffer message from client

		if (buffer == "0") {
			closesocket(Connections[index]);
			WSACleanup();
			cout << endl << "Problem z polaczeniem, zamykam sie." << endl;
			cin.ignore(2);
			exit(0);
		}

		string pom;
		int counter = 0;
		for (int i = 0; i < 1024; i++) {
			if (counter == 4) {
				break;
			}
			if (buffer[i] == ';') {
				counter++;
			}
			pom = pom + buffer[i];
		}
		//cout << pom << endl;

		//cout << "ODEBRANE OD " << index + 1 << " GRACZA: " << pom;

		int operacja;
		string pom3;

		for (int i = 0; i < pom.size(); i++) {
			if (pom[i] != '=') {
				pom3 += pom[i];
			}
			else {
				break;
			}
		}

		if (pom3 == "Operacja") {

		}
		if (pom3 == "Dane") {


			string pom2;

			for (int i = 0; i < pom.size(); i++) {
				if (pom[i] == '=') {
					for (int j = i + 1; j < pom.size(); j++) {
						if (pom[j] != ';') {
							pom2 += pom[j];
						}
						else {
							break;
						}
					}
				}
			}


			int dane_int = atoi(pom2.c_str()) / 10;
			cout << dane_int << "DANE INT" << endl;
			string wiad;
			//sprawdzenie danej liczby
			if (dane_int < szukana_liczba) {
				wiad = "Odpowiedz=twoja liczba jest za mala;";
				//cout << endl << "WYSYLANIE do " << index << " gracza: ";

				send_message(index, wiad, sesyjki[index]); //send the chat message to client at index i
			}
			else if (dane_int == szukana_liczba) {
				wiad = "Odpowiedz=wygrales;";

				//cout << endl << "WYSYLANIE do " << index << " gracza: ";
				winer = index + 1;
				send_message(index, wiad, sesyjki[index]); //send the chat message to client at index i
				zakoncz_gre();
				std::cin.ignore(2);
			}
			else {
				wiad = "Odpowiedz=twoja liczba jest za duza;";

				//cout << endl << "WYSYLANIE do " << index << " gracza: ";

				send_message(index, wiad, sesyjki[index]); //send the chatmessage to client at index i
			}


		}
		delete[] buffer; //Deallocate buffer to stop from leaking memory

	}

}



int main() {
	srand(time(NULL));
	szukana_liczba = (std::rand() % 98) + 1;; //losowanie szukanej liczby
	cout << szukana_liczba << endl;
	clock_t begin; //rozpoczecie odmierzania czasu



	WSAData wsaData;
	WORD DllVersion = MAKEWORD(2, 1);
	if (WSAStartup(DllVersion, &wsaData) != 0) //If WSAStartup returns anything other than 0, then that means an error has occured in the WinSock Startup.
	{
		MessageBoxA(NULL, "WinSock startup failed", "Error", MB_OK | MB_ICONERROR);
		return 0;
	}

	SOCKADDR_IN addr;
	inet_pton(AF_INET, "127.0.0.1", &(addr.sin_addr));
	int addrlen = sizeof(addr);
	addr.sin_addr = addr.sin_addr;
	addr.sin_port = htons(1111);
	addr.sin_family = AF_INET;

	sListen = socket(AF_INET, SOCK_STREAM, NULL);
	bind(sListen, (SOCKADDR*)&addr, sizeof(addr));
	listen(sListen, SOMAXCONN);

	SOCKET newConnection;


	cout << endl << "Ile graczy chcesz polaczyc?" << endl;
	int gracze;
	cin >> gracze;
	cout << endl << "Oczekiwanie na graczy..." << endl;
	// przynajmniej 2 graczy, przy 1 graczu wyskoczy error
	while (1) {
		//dodawanie nowych klientow
		newConnection = accept(sListen, (SOCKADDR*)&addr, &addrlen);
		std::cout << "Klient nr " << ConnectionCounter + 1 << " polaczony.";
		Connections.push_back(newConnection);
		send_sess_info(ConnectionCounter);
		cout << endl << "Jego nr sesji: " << sesyjki[ConnectionCounter] << endl;
		CreateThread(NULL, NULL, (LPTHREAD_START_ROUTINE)ClientHandlerThread, (LPVOID)(ConnectionCounter), NULL, NULL);
		ConnectionCounter += 1;
		newConnection = NULL;
		if (ConnectionCounter == gracze) {
			czas = wyznacz_czas();
			break;
		}

	}
	for (int i = 0; i < ConnectionCounter; i++) {
		wyslij_czas(czas, i); //wysylanie czasu
	}
	Sleep(15000);
	while (1) {
		if (czas > 15) {
			Sleep(15000);
			czas -= 15;
			for (int i = 0; i < ConnectionCounter; i++) {
				wyslij_czas(czas, i);
			}

		}
		else {
			Sleep(czas * 1000);
			zakoncz_gre(); //konczenie gry
		}
	}

	cin.ignore(2);
	return 0;
}