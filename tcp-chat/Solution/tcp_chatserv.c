/*서버와 클라이언트 간의 통신 프로그램 - 서버*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<sys/file.h>
#include<netinet/in.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>
#define MAXLINE 511 // 최대 라인 수 511
#define MAX_SOCK 1024 // 솔라리스의 경우 64

char *START_STRING = "서버와 연결이 되었습니다.\n"; // 클라이언트 환영 메시지를 정의
char *EXIT_STRING = "/exit"; // 클라이언트의 종료 요청 문자열을 정의
char *GETTIME_STRING = "/hello"; // hello 메시지를 저장하는 문자열 정의
char *GETHELP_STRING = "/help"; // 프로그램 내에 정의된 모든 명령어를 보기 위한 명령어 문자열 정의
char *GETINFOMATION_STRING = "/whoami"; // 현재 접속된 클라이언트의 정보를 출력하기 위한 명령어 문자열 정의
char *GETCLIENTS_STRING = "/showclients"; // 현재 접속된 클라이언트의 이름을 출력하기 위한 명령어 문자열 정의 
char *GETIP_STRING = "/getip"; // 현재 접속된 클라이언트의 ip 주소를 출력하기 위한 명령어 문자열 정의
char *GETNAME_STRING = "/getname"; // 현재 접속된 클라이언트의 이름을 출력하기 위한 명령어 문자열 정의
char *GETTIMEPASSED_STRING = "/timepassed"; // 현재 접속된 클라이언트가 얼마나 접속되어있는지 출력하기 위한 명령어 문자열 정의
char *LINE_STRING = "********************************************************\n\n"; // 라인 구분선 문자열 정의

int maxfdql; // 최대 소켓번호 + 1 값을 정의하는 정수 
int num_chat = 0; // 채팅 참가자 수를 정의하는 정수
int clisock_list[MAX_SOCK]; // 채팅에 참가자 소켓번호를 저장하는 정수형 배열 정의
char* cliname_list[100]; // 채팅에 참가자 이름을 저장하는 char*형 배열 정의
int listen_sock; // 서버의 리슨 소켓의 값을 정의하는 정수

				 
void addClient(int s, struct sockaddr_in *newcliaddr); // 새로운 채팅 참가자 처리하는 함수
int getmax(); // 최대 소켓번호를 반환하는 함수
void removeClient(int s); // 채팅 탈퇴를 처리하는 함수
int tcp_listen(int host, int port, int backlog); // 소켓 생성 및 listen 하는 함수
void errquit(char *mesg) { perror(mesg); exit(1); } // 오류 메시지를 출력하고 프로그램을 종료하는 함수
void show(); // 접속자의 이름 및 접속시간, 아이피주소를 출력하는 함수


char format[128][300]; // 접속 시간을 저장하기 위한 문자형 이차원 배열 선언
char ipAddress[128][300]; // 아이피 주소를 저장하기 위한 문자형 이차원 배열 선언 
char name[128][300]; // 클라이언트 이름을 저장하기 위한  문자형 이차원 배열 선언
int times[128]; // 시간을 저장하는 정수형 배열 선언
char buffer[1024] = ""; // 클라이언트에서 받은 메시지를 저장할 문자형 배열 선언
char t_buffer[1024] = ""; // 클라이언트에서 받은 메시지를 저장할 문자형 배열 선언

int h = 0; // name 배열의 인덱스를 저장해주기 위한 정수 선언
int n = 0; // ipAddress 배열의 인덱스를 저장해주기 위한 정수 선언
int l = 0; // times 배열의 인덱스를 저장해주기 위한 정수 선언
int end = 0; // 삭제된 클라이언트의 인덱스를 저장해주기 위한 정수 선언

int main(int argc, char *argv[]) { // 메인 함수
	struct sockaddr_in cliaddr; // 클라이언트 소켓 주소를 표현하는 구조체 정의
	char buf[MAXLINE + 1]; // 클라이언트에게 받은 메시지를 저장하는 문자형 배열 선언
	int i; // 반복문를 돌기 위한 정수형 변수
	int j; // 반복문를 돌기 위한 정수형 변수
	int k; // 반복문를 돌기 위한 정수형 변수
	int nbyte; // 클라이언트에서 보내오는 데이터를 읽는 recv 함수의 반환값을 저장하는 변수
	int accp_sock; // 클라이언트와 설정된 연결들을 실제로 받아들이는 소켓 번호
	int addrlen = sizeof(struct sockaddr_in); // 소켓 주소 구조체의 길이를 저장하는 정수형 변수
	
	fd_set read_fds; // 읽기를 감지할 fd_set 구조체를 정의
	h = 0; // name 배열의 인덱스를 저장하는 변수를 0으로 초기화함
	

	if (argc != 2) { // 만약 메인 함수에 전달된 인자의 개수가 2개가 아니라면
		printf("사용법 : %s port\n", argv[0]); // 사용하는 방법 출력
		exit(0); // 종료
	}

	//tcp_listen(host, port, backlog) 함수 호출
	listen_sock = tcp_listen(INADDR_ANY, atoi(argv[1]), 5);
	// 클라이언트의 연결을 기다림


	while (1) { // 무한 루프를 돌면서 (서버는 항상 돌아가야하므로)
		FD_ZERO(&read_fds); // read_fds의 모든 비트를 지움
		FD_SET(listen_sock, &read_fds); // read_fds 중 소켓 listen_sock에 해당하는 비트를 1로 함
		for (i = 0; i < num_chat; i++) // 클라이언트의 수만큼 반복문을 돌면서 
			FD_SET(clisock_list[i], &read_fds); // read_fds 중 소켓 clisock_list[i]에 해당하는 비트를 1로 함

		int maxfdpl = getmax() + 1; // maxfdpl 재계산함
		// maxfdpl은 최대 소켓 번호에 1을 더한 값임
		puts("\nServer >> 클라이언트를 기다리고 있습니다....\n");
		// 서버가 클라이언트를 기다리고 있다는 메시지 출력
		if (select(maxfdpl, &read_fds, NULL, NULL, NULL) < 0)
			errquit("select fail");
		/* 소켓에서 발생하는 I/O변화를 기다리다가 지정된 I/O 변화가 발생하면 리턴함
		만약 변화가 발생하지 않으면 오류를 출력
		
		maxfdpl : 최대 파일(및 소켓)번호 크기 + 1
		read_fds : 읽기 상태 변화를 감지할 소켓 지정
		*/

		if (FD_ISSET(listen_sock, &read_fds)) { // read_fds 중 listen_sock에 해당하는 비트가 세트되어있으면 양수값을 리턴
			accp_sock = accept(listen_sock, (struct sockaddr*)&cliaddr, &addrlen); // 클라이언트와 설정된 연결들을 실제로 받아들이기 위한 accept 호출

			if (accp_sock == -1) errquit("accept fail"); // 만약 accept 함수에서 반환된 값(accp_sock)이 -1이라면 오류를 출력

			/*최초 접속 시간을 가져오는 방법*/
			time_t seconds; // time_t형 seconds를 선언 
			time(&seconds); // time 함수에 seconds의 주소값을 인자로 해서 넘겨줌
			strftime(format[l], 300, "%Y %B %d %A, %I:%M:%S %p\n", localtime(&seconds));
			// strftime 함수는 struct tm 값으로 포맷에 맞춘 시간 문자열을 구함
			/*
			format[l] : 문자열을 받을 버퍼 포인터
			300 : 버퍼의 크기
			%Y %B %d %A, %I:%M:%S %p\n : 문자열 포맷
			localtime(&seconds) : 날짜와 시간 정보
			*/
			times[l] = seconds; // times 배열의 l의 인덱스에 seconds의 값을 대입
			l++; // l의 값을 증가


			/*클라이언트의 이름을 받는 방법*/
			int numByte; // 클라이언트에게 받은 메시지의 반환값을 저장하는 정수형 변수
			if ((numByte = recv(accp_sock, name[h], 100, 0)) > 0) { // 만약 클라이언트에게 클라이언트의 이름을 받았으면 
			/*
			accp_sock : 소켓 번호
			name[h] : 수신 데이터를 저장할 버퍼
			100 : 버퍼의 길이
			0 : 플래그의 값, 보통 0
			*/
				name[h][numByte] = 0; // 받은 이름의 가장 끝에 '\0'문자 대입(문자열의 마지막 처리)
				h++; // h 값 증가
			}

			addClient(accp_sock, &cliaddr); // 새로운 채팅 참가자를 처리하는 addClient 함수 호출
			send(accp_sock, START_STRING, strlen(START_STRING), 0); // 클라이언트에게 START_STRING 문자열을 보내줌
			/*
			accp_sock : 소켓번호
			START_STRING : 전송할 데이터
			strlen(START_STRING) : 전송할 데이터의 크기
			0 : 플래그의 값, 보통 0
			*/
			show(); // 접속자의 이름, 접속시간, 아이피 주소를 출력해주는 show 함수 호출
			printf("\nServer >> %d번째 사용자가 서버에 접속하였습니다 !!\n", num_chat);
			// 서버측에 사용자가 서버에 접속했다는 메시지 출력
		}


		// 클라이언트가 보낸 메시지를 모든 클라이언트에게 발송
		for (i = 0; i < num_chat; i++) { // 접속된 클라이언트 수만큼 반복문을 돌면서
			if (FD_ISSET(clisock_list[i], &read_fds)) { 
			// 만약 read_fds 중 소켓 clisock_list[i]에 해당하는 비트가 세트되어
				nbyte = recv(clisock_list[i], buf, MAXLINE, 0); 
				// 만약 클라이언트에게 메시지를 받았으면
				
				if (nbyte <= 0) { // 만약 클라이언트에게 메시지를 받지 못했으면
					end = i; // 종료할 클라이언트의 인덱스 값을 end에 저장한 뒤
					removeClient(i); // i번째 클라이언트를 탈퇴시킴
					l--; h--; n--; // 접속된 클라이언트가 한명 감소하였으며 인덱스를 모두 1 감소시킴
					continue; 
				}

				buf[nbyte] = 0; // 클라이언트에게 받은 메세지의 마지막에 '\0'문자 대입(문자열 마지막 처리)
				
				// 클라이언트에게 받은 문자열이 /exit 라면
				if (strstr(buf, EXIT_STRING) != NULL) {
					end = i; // 종료할 클라이언트의 인덱스 값을 end에 저장한 뒤
					removeClient(i); // i번째 클라이언트를 탈퇴시킴

					l--; h--; n--; // 접속된 클라이언트가 한명 감소하였으며 인덱스를 모두 1 감소시킴
					continue;
				}


				// hello 메시지를 받으면 모든 클라이언트에게 서버 최초 접속시간 전송			
				if (strstr(buf, GETTIME_STRING) != NULL) { // 클라이언트에게 받은 문자열이 /hello 라면
					for (k = 0; k < num_chat; k++) { // 접속된 클라이언트 수만큼 반복문을 돌면서
						sprintf(buffer, "%s클라이언트(%s)의 서버 최초 접속 시간은\n\t%s 입니다.\n\n%s", LINE_STRING, name[k], format[k], LINE_STRING);
						// 각 클라이언트의 서버 최초 접속 시간을 문자열 형태로 buffer에 저장함 
						send(clisock_list[k], buffer, strlen(buffer), 0);
						// 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
					
					}	
				}
				else if (strstr(buf, GETIP_STRING) != NULL) { // 클라이언트에게 받은 문자열이 /getip 라면

					sprintf(buffer, "%s\n\n\t접속 아이피 주소 : %s\n\n%s", LINE_STRING, ipAddress[i], LINE_STRING);
					// 각 클라이언트의 접속 아이피 주소 문자열 형태로 buffer에 저장함
					send(clisock_list[i], buffer, strlen(buffer), 0); // 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
				}
				else if (strstr(buf, GETNAME_STRING) != NULL) { // 클라이언트에게 받은 문자열이 /getname 라면

					sprintf(buffer, "%s\n\n\t접속자 이름 : %s\n\n%s", LINE_STRING, name[i], LINE_STRING);
					// 각 클라이언트의 이름을 문자열 형태로 buffer에 저장함
					send(clisock_list[i], buffer, strlen(buffer), 0); // 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
				}
				else if (strstr(buf, GETINFOMATION_STRING) != NULL) { // 클라이언트에게 받은 문자열이 /whoami 라면

					sprintf(buffer, "%s\t접속자 이름 : %s\n\n\t접속시간 : %s\n\t접속 아이피 주소 : %s\n\n%s", LINE_STRING, name[i], format[i], ipAddress[i], LINE_STRING);
					// 각 클라이언트의 이름, 접속시간, 접속 아이피 주소를 문자열 형태로 buffer에 저장함
					send(clisock_list[i], buffer, strlen(buffer), 0); // 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
				}
				else if (strstr(buf, GETHELP_STRING) != NULL) { // 클라이언트에게 받은 문자열이 /help 라면

					// 모든 명령어와 이에 대한 설명을 buffer에 저장함
					sprintf(buffer, "%s\t%s : 명령어 목록을 출력합니다. \n\t%s : 클라이언트의 서버 최초 접속시간을 출력합니다.\n\t%s : 현재 클라이언트의 정보를 출력합니다.\n\t%s : 현재 클라이언트의 이름을 출력합니다.\n\t%s : 현재 클라이언트의 아이피 주소를 출력합니다.\n\t%s : 접속된 클라이언트의 이름 목록을 출력합니다.\n\t%s : 클라이언트가 서버에 연결된 후, 얼마나 시간이 흘렀는지를 출력합니다.\n\t%s : 클라이언트를 종료합니다.\n\n%s", LINE_STRING, GETHELP_STRING, GETTIME_STRING, GETINFOMATION_STRING,  GETNAME_STRING, GETIP_STRING,GETCLIENTS_STRING,GETTIMEPASSED_STRING,EXIT_STRING,LINE_STRING);
					send(clisock_list[i], buffer, strlen(buffer), 0); // 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
	
				}	 
				else if (strstr(buf, GETCLIENTS_STRING) != NULL) { // 클라이언트에게 받은 문자열이 /showclients 라면
					
					sprintf(buffer, "%s",LINE_STRING); // buffer에 라인을 구분하는 문자열을 저장함		
					
					for (k = 0; k < num_chat; k++) { // 접속된 클라이언트 수만큼 반복문을 돌면서
						sprintf(t_buffer, "%s\n", name[k]); // 접속된 클라이언트의 이름을 t_buffer에 저장하고
						strcat(buffer, t_buffer); // t_buffer를 buffer 뒤에 저장함						
					}
					sprintf(t_buffer, "\n%s",LINE_STRING); // 그리고 라인을 구분하는 문자열을 t_buffer에 저장하고
					strcat(buffer, t_buffer); // t_buffer를 buffer 뒤에 저장함
					send(clisock_list[i], buffer, strlen(buffer), 0); // 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
				}

				else if (strstr(buf, GETTIMEPASSED_STRING) != NULL) { // 클라이언트에게 받은 문자열이 /timepassed 라면
				
					/*시간을 가져오는 방법*/
					time_t seconds_cli; // time_t형 seconds_cli 선언 
					time(&seconds_cli); // time 함수에 seconds_cli의 주소값을 인자로 해서 넘겨줌
					
					int tmp = difftime(seconds_cli, times[i]); // 정수형 tmp에 seconds_cli와 times[i] 시간 차이(초)를 계산함
					
					if(tmp < 60) // 시간 차이 tmp가 60보다 작으면
						sprintf(buffer,"접속시간으로 부터 %d초 지났습니다.\n", tmp); // tmp의 값을 buffer에 저장
					else if (tmp < 3600) // 시간 차이 tmp가 3600보다 작으면
						sprintf(buffer,"접속시간으로 부터 %d분 %d초 지났습니다.\n", tmp/60 , tmp%60); // tmp의 값으로 분, 초를 계산해서 buffer에 저장
					else // 위의 조건에 걸리지 않으면
						sprintf(buffer,"접속시간으로 부터 %d시 %d분 %d초 지났습니다.\n", tmp/3600, (tmp%3600)/60 , tmp%60);
						// tmp의 값으로 시, 분, 초를 계산해서 buffer에 저장
						
					send(clisock_list[i],buffer, strlen(buffer), 0); // 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
				
				}			
				else { // 만약 위의 명령어가 아니고 일반 메시지라면
					// 모든 채팅 참가자에게 메시지 발송
					for (j = 0; j < num_chat; j++) { // 접속된 클라이언트 수만큼 반복문을 돌면서
						send(clisock_list[j], buf, nbyte, 0); // 접속된 클라이언트에게 buffer에 저장된 메시지를 전송함
					}
					printf("\n%s\n", buf); // buf 값을 출력
				}
			
				memset(buffer,0,sizeof(buffer)); // buffer를 초기화함
			}

		}

	}

	return 0; // end of while
}


void addClient(int s, struct sockaddr_in *newcliaddr) { // 새로운 채팅 참가자 처리하는 함수
	char buf[20]; // 문자열 buf 변수 정의
	inet_ntop(AF_INET, &newcliaddr->sin_addr, buf, sizeof(buf)); // 2진수 IP주소를 10진수 IP주소로 변환하는 함수 호출
	
	clisock_list[num_chat] = s; // clisock_list 배열의 마지막 인덱스에 s 대입
	strcpy(ipAddress[n], buf); // buf에 저장된 값을 ipAddress 배열에 저장함
	n++; // n(ipAddress 배열 인덱스)를 증가함 
	num_chat++; // num_chat(접속된 클라이언트 수)를 증가
}


void show() { // 접속자의 이름, 접속시간, 접속 아이피 주소를 출력하는 함수 

	printf("********************************************************\n\n"); // 라인 출력
	printf("총 접속자 수 : %d명\n\n", num_chat); // 현재 총 접속자 수 출력

	int i = 0; // 반복문 변수 초기화
	for (i = 0; i < num_chat; i++) { // 접속된 클라이언트 수만큼 반복문을 돌면서
		printf("%d) 접속자 이름 : %s\n", i + 1, name[i]); // 접속자 이름 출력
		printf("\t접속시간 : %s", format[i]); // 최초 접속 시간 출력
		printf("\t접속 아이피 주소 : %s\n\n", ipAddress[i]); // 접속 아이피 주소 출력

	}
	printf("\n********************************************************\n"); // 라인 출력

}



void removeClient(int s) { // 채팅 탈퇴 처리하는 함수
	close(clisock_list[s]); // 함수 인자로 들어온 인덱스 s 위치의 clisock_list 배열값을 close 시킴  
	if (s != num_chat - 1) clisock_list[s] = clisock_list[num_chat - 1];
	num_chat--; // num_chat(접속된 클라이언트 수)를 감소 
	printf("Server >> 채팅 참가자 중 1명이 탈퇴하였습니다,\n\n현재 참가자 수는 %d명입니다.\n\n\n", num_chat);
	// 서버측에 채팅 참가자 중 한명이 탈퇴하였다는 메시지를 출력

	// 삭제된 인덱스의 배열 값을 초기화	 
	strcpy(name[end], ""); // name 배열 초기화
	strcpy(format[end], ""); // format 배열 초기화
	strcpy(ipAddress[end], ""); // ipAddress 배열 초기화

	int i; // 반복문 변수를 정의함
	for (i = end; i < l - 1; i++) { // end 부터 l-2까지 반복문을 돌면서
		strcpy(format[i], format[i + 1]); // format[i + 1]에 해당하는 값을 format[i]에 복사함
	}
	for (i = end; i < h - 1; i++) { // end 부터 h-2까지 반복문을 돌면서
		strcpy(name[i], name[i + 1]); // name[i + 1]에 해당하는 값을 name[i]에 복사함
	}
	for (i = end; i < n - 1; i++) { // end 부터 n-2까지 반복문을 돌면서
		strcpy(ipAddress[i], ipAddress[i + 1]); // ipAddress[i + 1]에 해당하는 값을 ipAddress[i]에 복사함
	}

	show(); // 접속자의 이름, 접속시간, 접속 아이피 주소를 출력하는 함수 호출
}


int getmax() { // 최대 소켓번호를 반환하는 함수
	// MIninum 소켓번호는 가장 먼저 생성된 listen_sock
	int max = listen_sock; // listen_sock을 max에 대입함
	int i; // 반복문 변수를 정의함
	for (i = 0; i < num_chat; i++) { // 0부터 num_chat - 1만큼 반복문을 돌면서
		if (clisock_list[i] > max) max = clisock_list[i]; 
		// 만약 clisock_list[i]에 해당하는 값이 현재 max 값보다 크면 max 값을 clisock_list[i]으로 업데이트함

	}
	return max; // 최대 소켓번호를 반환함
}



int tcp_listen(int host, int port, int backlog) { // listen 소캣 생성 및 listen하는 함수

	int sd; // 정수형 변수 sd를 정의함
	struct sockaddr_in servaddr; // 서버 소켓 주소를 표현하는 구조체 정의
	sd = socket(AF_INET, SOCK_STREAM, 0); // 서버가 클라이언트와 통신하기 위해서 소켓을 생성함
	
	if (sd == -1) { // 만약 생성한 소켓 반환값이 -1이라면
		perror("socket fail"); // 소켓 생성 실패라는 오류를 출력
		exit(1); // 종료
	}

	// servaddr 구조체의 내용 셋팅
	bzero((char*)&servaddr, sizeof(servaddr)); // bzero 함수를 통하여 servaddr을 '\0'으로 초기화함
	servaddr.sin_family = AF_INET; // servaddr의 sin_family 멤버를 AF_INET으로 설정
	servaddr.sin_addr.s_addr = htonl(host); 
	// 소켓을 주소와 연결해주는 bind() 함수를 호출하는 경우, 서버의  소켓 주소 구조체를 넣어줄 때 SOCKADDR_IN.sin_addr.s_addr 멤버를 지정할 때 쓸 수 있음
	servaddr.sin_port = htons(port);
	// 메인함수 호출 시 저장된 문자열 중 port으로 포트 번호를 설정

	/* 프로그램이 컴퓨터 외부와 통신하기 위해
	소켓번호와 소켓주소(ip주소 + 포트번호)를 연결해주기 위해 bind() 호출*/
	if (bind(sd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0)
	{
	/*
	sd : 소켓번호
 	(struct sockaddr*)&servaddr : 서버 자신의 소켓주소 구조체 포인터
	sizeof(servaddr) : *addr 구조체의 크기
	*/
		perror("bind fail"); // 오류를 출력
		exit(1);  // 종료
	}

	// 클라이언트로부터 연결요청 기다림
	listen(sd, backlog);
	/*
	sd : 소켓번호
	backlog : 연결을 기다리는 클라이언트의 최대 수
	*/
	
	return sd; // 소켓번호 반환
}


