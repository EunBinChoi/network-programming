/*서버와 클라이언트 간의 통신 프로그램 - 클라이언트*/
#include<stdio.h>
#include<stdlib.h>
#include<string.h>
#include<strings.h>
#include<fcntl.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<sys/time.h>
#include<arpa/inet.h>
#include<unistd.h>
#include<time.h>

#define MAXLINE 1000 // 최대 라인 수 1000
#define NAME_LEN 20  // 이름 길이 20

char *EXIT_STRING = "/exit";  // 클라이언트의 종료 요청 문자열을 정의
char *GETTIME_STRING = "/hello"; // hello 메시지를 저장하는 문자열 정의
char *GETHELP_STRING = "/help"; // 프로그램 내에 정의된 모든 명령어를 보기 위한 명령어 문자열 정의
char *GETINFOMATION_STRING = "/whoami"; // 현재 접속된 클라이언트의 정보를 출력하기 위한 명령어 문자열 정의
char *GETCLIENTS_STRING = "/showclients"; // 현재 접속된 클라이언트의 이름을 출력하기 위한 명령어 문자열 정의 
char *GETIP_STRING = "/getip"; // 현재 접속된 클라이언트의 ip 주소를 출력하기 위한 명령어 문자열 정의
char *GETNAME_STRING = "/getname"; // 현재 접속된 클라이언트의 이름을 출력하기 위한 명령어 문자열 정의
char *GETTIMEPASSED_STRING = "/timepassed"; // 현재 접속된 클라이언트가 얼마나 접속되어있는지 출력하기 위한 명령어 문자열 정의

int tcp_connect(int af, char* servip, unsigned short port); // 소켓 생성 및 서버 연결, 생성된 소켓을 리턴하는 함수
void errquit(char* mesg){perror(mesg); exit(1);} // 오류 메시지를 출력하고 프로그램을 종료하는 함수

int i = 0; // 반복문 변수 정의
int j = 0; // 반복문 변수 정의


char buffer[1024] =""; // 서버로부터 온 메시지를 저장하는 문자형 배열 선언

int main(int argc, char *argv[]){ // 메인 함수
	char bufall[MAXLINE + NAME_LEN], // 이름 + 메시지를 저장하기 위한 버퍼 생성
	 *bufmsg; // bufall에서 메시지 부분의 포인터를 저장할 char* 변수 생성
	
	int maxfdpl, // 최대 소켓 디스크립터를 저장하는 정수형 변수 생성
	 s, // 소켓을 저장하는 정수형 변수 생성
	 namelen; // 이름의 길이를 저장하는 변수 생성
	
	fd_set read_fds; // 읽기를 감지할 fd_set 구조체를 정의 
	
	
	if(argc != 4){ // 만약 메인 함수에 전달된 인자의 개수가 4개가 아니라면
		printf("사용법 : %s server_ip port name\n",argv[0]); // 사용하는 방법 출력
		exit(0); // 종료
	}
	
		
	sprintf(bufall, "[%s] : ", argv[3]); // bufall의 앞 부분에 클라이언트의 이름을 저장
	namelen = strlen(bufall); // bufall의 길이를 namelen에 저장함
	bufmsg = bufall + namelen; // bufall값과 namelen을 더해 메시지 시작 부분을 지정함
	s = tcp_connect(AF_INET, argv[1],atoi(argv[2])); // 소켓 생성 및 서버 연결, 생성된 소켓을 리턴하는 함수 호출

	if(s == -1) errquit("tcp connect fail"); // 만약 tcp_connect 함수의 반환값이 -1이라면 오류를 출력하고 프로그램 종료시킴
	puts("\n서버에 접속되었습니다."); // 위에서 프로그램이 종료되지 않았더라면 반환값이 제대로 나왔다는 의미이므로 서버에 접속하였다는 메시지를 출력함
	maxfdpl = s+1; // tcp_connect 함수의 반환값을 저장하는 s의 값에 1을 증가하여 maxfdpl에 저장함
	FD_ZERO(&read_fds); // read_fds의 모든 비트를 지움
	

	if(s != -1) send(s,argv[3], strlen(argv[3]), 0);
	// 만약 s가 -1이 아니라면 서버에게 클라이언트 이름을 전송함
	
	while(1){ // 무한 루프를 돌면서
		
		FD_SET(0,&read_fds); // 키보드 입력 데이터를 서버로 전송함
		FD_SET(s, &read_fds); // 서버가 보내오는 메시지를 수신하여 출력함
	
		if(select(maxfdpl, &read_fds, NULL,NULL,NULL) < 0) 
			errquit("select fail");
		/* 소켓에서 발생하는 I/O변화를 기다리다가 지정된 I/O 변화가 발생하면 리턴함
		만약 변화가 발생하지 않으면 오류를 출력
		
		maxfdpl : 최대 파일(및 소켓)번호 크기 + 1
		read_fds : 읽기 상태 변화를 감지할 소켓 지정
		*/

	
		if(FD_ISSET(s, &read_fds)){ // read_fds 중 s에 해당하는 비트가 세트되어있으면 양수값을 리턴
			int nbyte; // recv 함수의 반환값을 저장할 정수형 변수 선언			
			if((nbyte = recv(s, bufmsg, MAXLINE, 0)) > 0){ // 만약 서버로 부터 메시지가 왔다면 이를 bufmsg 배열에 저장함
				bufmsg[nbyte] = 0; // bufmsg 배열의 가장 끝에 '\0'문자를 추가(문자열 마지막 처리)
				printf("\n%s\n", bufmsg); // bufmsg 배열에 저장된 문자열을 출력
				puts("\n명령어에 대한 설명을 보려면 /help 을 입력하시오....\n");
				// 명령어에 대한 설명을 보기 위해 /help 를 입력하라는 메시지 출력	
	
				
			}else{ // 만약 서버로 부터 메시지가 오지 않았다면 
				puts("서버가 종료되었습니다."); // 서버가 종료되었다는 메시지를 출력
				return 0; // 프로그램 종료
			}

		}
		
		if(FD_ISSET(0, &read_fds)){ // read_fds 중 0에 해당하는 비트가 세트되어있으면 양수값을 리턴	
			
			if(fgets(bufmsg, MAXLINE, stdin)){ // 문자열을 읽어들여 bufmsg 배열에 저장
				
				// 소켓 s에게 bufall 문자열을 보냄  
				if(send(s,bufall, namelen+strlen(bufmsg),0) < 0)
					puts("Error : Write error on socket");
				// 만약 send 함수의 반환값이 0보다 작으면 Write error라는 메시지를 출력
				
				if(strstr(bufmsg,EXIT_STRING) != NULL){ // 만약 bufmsg 배열에 저장된 문자열이 EXIT_STRING와 일치한다면
					printf("클라이언트(%s)가 접속을 종료하였습니다.\n\n", argv[3]);
					// 클라이언트가 접속을 종료했다는 메시지를 출력
					close(s); // s를 종료시킴
					exit(0); // 프로그램 종료
				}	
				// 만약 bufmsg 배열이 GETHELP_STRING, GETCLIENTS_STRING, GETTIME_STRING, GETINFOMATION_STRING, GETIP_STRING, GETNAME_STRING, GETTIMEPASSED_STRING 중에 하나라도 해당되는 문자라면
				else if( strstr(bufmsg,GETHELP_STRING) != NULL || strstr(bufmsg,GETCLIENTS_STRING) != NULL ||strstr(bufmsg,GETTIME_STRING) != NULL || strstr(bufmsg,GETINFOMATION_STRING) != NULL || strstr(bufmsg,GETIP_STRING) != NULL || strstr(bufmsg,GETNAME_STRING) != NULL||strstr(bufmsg,GETTIMEPASSED_STRING) != NULL){
									
					recv(s, buffer,strlen(buffer),0);
					// s로부터 buffer의 값을 받음
					printf("%s",buffer);
					// buffer 출력
								
					memset(buffer,0,sizeof(buffer)); // buffer를 초기화함
				}

			}
		}
		
		
	} // end of while
}
		

int tcp_connect(int af, char* servip, unsigned short port){ // tcp 연결 요청을 하는 함수
	struct sockaddr_in servaddr; // 서버 소켓 주소를 표현하는 구조체 정의 
	
	int s; // socket 함수의 반환값을 저장하는 정수형 변수 

	// 클라이언트가 서버와 통신하기 위해서 소켓을 생성함
	// 만약 생성한 소켓 반환값이 0보다 작다면 프로그램 종료시킴
	if((s = socket(af,SOCK_STREAM,0)) < 0) return -1;
	
	// 채팅 서버의 소켓 주소 구조체 servaddr 초기화
	bzero((char*)&servaddr, sizeof(servaddr)); // bzero 함수를 통하여 servaddr을 '\0'으로 초기화함
	servaddr.sin_family = af; // servaddr의 sin_family 멤버를 af으로 설정
	inet_pton(AF_INET, servip, &servaddr.sin_addr); // 10진수 IP주소를 2진수 Ip주소로 변환하는 함수 호출
	servaddr.sin_port = htons(port); // 메인함수 호출 시 저장된 문자열 중 port으로 포트 번호를 설정

	// connect 함수를 통해 연결을 요청함
	if(connect(s, (struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) return -1;
	return s; // s를 반환

}
