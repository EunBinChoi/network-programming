/*서버와 클라이언트 간의 파일 전송 프로그램 - 클라이언트*/
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<time.h>
#include<dirent.h>
#include<pwd.h>
#include<grp.h>


#define FILENAMESIZE 256 //파일 이름 최대 길이를 256으로 정의 
#define FILEPATHSIZE 256 //파일 경로 최대 길이를 256으로 정의
#define SCANPATHSIZE 512 //파일 목록 찾기에서 사용되는 경로 최대 길이를 512으로 정의
#define FileUpReq  01 //파일 업로드 메세지 번호를 01으로 정의
#define FileDownReq 02 //파일 다운로드 메세지 번호를 02으로 정의
#define ScanDirReq 03 //파일 디렉토리 목록 스캔 메세지 번호를 03으로 정의
#define FileAck 11 //Ack 값을 11로 정의
#define ExitReq 12 //Exit 값을 12로 정의
#define BUFSIZE 512 //버퍼의 크기를 512으로 정의


void error_handling(char *message); //에러 출력 및 프로그램 종료 함수 정의
void FileUploadProcess(int sock, char* A_filePath, char* A_fileName);	
//파일 업로드 처리 함수 정의
void HandleFileDownload(int sock, char* A_filePath, char* A_fileName);	
//파일 다운로드 처리 함수 정의
void clear_buffer(); //키보드 입력 버퍼 초기화 함수 정의
void print_Info(struct sockaddr_in* sv); //개인정보 출력 함수 정의
void client_scanDir(char *cpath); //클라이언트의 파일/디렉토리 목록 탐색 함수 정의
void server_scanDir(int sock, char *spath); //서버의 파일/디렉토리 목록 탐색 함수 정의
void server_scanPath(int sock, char *spath); //해당 경로에 해당하는 서버의 파일/디렉토리 목록 탐색 함수 정의
char *strlwr(char *str); //문자열을 모두 소문자로 변경해주는 함수 정의

int main(int argc, char*argv[]) // 메인 함수 정의
{
	/*명령 파라미터의 개수가 3인지 검사하는 부분*/
	if (argc != 3) { // 명령 파라미터의 개수가 3개가 아니라면
		printf("Usage : %s <Sever Address> <Sever Port>\n", argv[0]); // 사용하는 방법 출력
		exit(1); // 종료
	}

	char* servIP = argv[1];	// 서버 아이피(argv[1])를 char*형 변수에 정의
	char f_name[FILENAMESIZE]; // 파일 이름을 저장할 문자형 배열 정의
	char path[FILEPATHSIZE]; // 경로를 저장할 공간을 문자형 배열 정의

	/*socket(): 소켓 생성*/
	printf("%c[0m", 27); // 글자를 기본색으로 설정
	int sock = socket(PF_INET, SOCK_STREAM, IPPROTO_TCP); // 소켓 생성
	if (sock == -1) error_handling("socket error"); 
	// 소켓 생성 반환값이 -1이라면 소켓 생성 에러 출력

	/*connect() : 서버에 접속*/
	struct sockaddr_in serv_addr; // 서버 소켓 주소를 표현하는 구조체 정의
	memset(&serv_addr, 0, sizeof(serv_addr)); // 버퍼를 초기화함
	serv_addr.sin_family = AF_INET; // serv_addr의 sin_family 멤버를 AF_INET으로 설정
	serv_addr.sin_addr.s_addr = inet_addr(servIP);  // serv_addr.sin_addr의 s_addr 맴버를 servIP으로 설정
	serv_addr.sin_port = htons((unsigned short)atoi(argv[2])); // 메인함수 호출 시 전달된 인자 중 argv[2]를 serv_addr의 sin_port으로 설정
	if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) == -1) // connect 함수를 통해 연결을 요청함
		error_handling("connect() error"); // connect 실패시 에러 발생

	char command[20]; // 커맨드를 저장할 공간 정의
	char op; // 종료 여부 입력 변수 정의

	system("clear"); //터미널 창을 정리하기 위함
	
	while (1) { // 무한 루프를 돌면서

		print_Info(&serv_addr);	//개인정보 출력

		/*메뉴 출력*/
		printf("===============================================================================\n");
		printf("%c[1;32m", 27);	// 글자 색을 녹색으로 변경
		printf("\t\t\tPUT : File upload\n"); // 파일 업로드를 설명하는 메시지 출력
		printf("\t\t\tGET : File downLoad\n"); // 파일 다운로드를 설명하는 메시지 출력
		printf("\t\t\tCDIR : Scan directory in the client\n");	
		//클라이언트의 파일 / 디렉토리 탐색을 설명하는 메시지 출력
		printf("\t\t\tSDIR : Scan directory in the server\n");	
		//서버의 파일 / 디렉토리 탐색을 설명하는 메시지 출력
		printf("\t\t\tEXIT : Exit program\n");			
		//프로그램 종료를 설명하는 메시지 출력
		printf("%c[0m", 27); // 글자색을 기본색으로 복구
		printf("===============================================================================\n");
		printf("> 명령어 : "); // 메시지 출력

		scanf("%s", command); //사용자로부터 명령어를 입력 받음
		printf("---------------------------\n\n");
		strlwr(command); //모두 소문자로 변경함.
		system("clear");


		/*command 입력에 따라 분기하여 처리*/

		if (strcmp(command, "put") == 0) { //파일 업로드 처리
			print_Info(&serv_addr); // 개인 정보 출력
			
			printf("===============================================================================\n");
			printf("%c[1;32m", 27);	// 글자 색을 녹색으로 변경
			printf("\t\t\tPUT : File Upload\n"); 
			// 파일 업로드를 위한 메시지 설명
			printf("%c[0m", 27); // 글자색을 기본색으로 복구
			printf("===============================================================================\n");
			printf("> 파일 경로(현재 디렉토리 검색시 .을 입력) : ");
			scanf("%s", path);
			// 파일 경로 입력받음
			
			printf("> 파일 이름(확장자 포함) : ");
			scanf("%s", f_name); // 파일 이름 입력받음
			printf("===============================================================================\n");
		
			FileUploadProcess(sock, path, f_name);	
		//클라이언의 해당 경로에 있는 파일을 서버에 업로드 하는 함수 호출
		}

		else if (strcmp(command, "get") == 0) { // 파일 다운로드 처리
			print_Info(&serv_addr); // 개인 정보 출력
			printf("===============================================================================\n");
			printf("%c[1;32m", 27);	// 글자 색을 녹색으로 변경
			printf("\t\t\tGET : File DownLoad\n"); 
			// 클라이언트가 선택한 메뉴와 메뉴 설명 출력
			printf("%c[0m", 27); // 글자색을 기본색으로 복구
			printf("===============================================================================\n");
			printf("> 파일 경로(현재 디렉토리 검색시 .을 입력) : ");
			scanf("%s", path); // 파일 경로를 입력받음
			
			printf("> 파일 이름(확장자 포함) : ");
			scanf("%s", f_name); // 파일 이름을 입력받음
			HandleFileDownload(sock, path, f_name);	
			//서버의 해당 경로에 있는 파일을 다운로드 하는 함수

		}
		else if (strcmp(command, "cdir") == 0) { 
		//클라이언트 파일/디렉토리 탐색 처리
			print_Info(&serv_addr); // 개인 정보 출력
			printf("===============================================================================\n");
			printf("%c[1;32m", 27);	// 글자 색을 녹색으로 변경
			printf("\t\t\tCDIR : Scan directory in the client\n");
			// 클라이언트가 선택한 메뉴와 메뉴 설명 출력
			printf("%c[0m", 27); // 글자색을 기본색으로 복구
			printf("===============================================================================\n");
			printf("> 경로 : "); 
			scanf("%s", path); // 경로 입력받음

			
			client_scanDir(path);			
			//클라이언트에서 해당 경로의 파일/디렉토리 목록 탐색

		}
		else if (strcmp(command, "sdir") == 0) {	
		//서버 파일/디렉토리 탐색 처리
			print_Info(&serv_addr); // 개인 정보 출력
			printf("===============================================================================\n");
			printf("%c[1;32m", 27);	// 글자 색을 녹색으로 변경
			printf("\t\t\tSDIR : Scan directory in the server\n");
			// 클라이언트가 선택한 메뉴와 메뉴 설명 출력
			printf("%c[0m", 27); // 글자색을 기본색으로 복구
			printf("===============================================================================\n");
			printf("> 경로 : ");
			scanf("%s", path); // 경로 입력받음
			
			server_scanDir(sock, path);		
			//서버에서 해당 경로의 파일/디렉토리 목록 탐색

		}
		else if (strcmp(command, "exit") == 0) {
		// 서버 종료 처리
			uint8_t msgType = ExitReq; 
			// msgType의 값을 ExitReq을 대입
			ssize_t numBytesSent = send(sock, &msgType, sizeof(msgType), 0);	// 서버에 종료 메세지 전송
														
			/*비정상 송신시 에러 발생*/
			if (numBytesSent == -1) error_handling("send() error"); 
// send의 함수 반환값을 저장하는 numBytesSent이 -1이라면 send 에러 출력
			else if (numBytesSent != sizeof(msgType)) error_handling("sent unexpected number of bytes");
// send의 함수 반환값을 저장하는 numBytesSent과 msgType의 크기가 다르면 제대로된 메시지가 전송된 것이 아니라는 에러 출력
			break;
		}
		
		else {
			printf("%c[1;34m", 27);	//글자색 변경(파란색)		
			printf("> 없는 메뉴번호입니다.\n\n");
			printf("%c[0m", 27); // 글자색을 기본색으로 복구
		}
		// 없는 메뉴 번호라는 메시지 출력

		printf("\n> 메뉴로 돌아 가시겠습니까? (Y/N) : ");
		// 메뉴 출력을 위한 메시지 출력
		
		clear_buffer(); //키보드 입력 버퍼 제거
		scanf("%c", &op); //사용자로부터 종료 여부를 입력 받음
		clear_buffer();	//키보드 입력 버퍼 제거
		
		if (op == 'Y' || op == 'y') { // 만약 op이 Y이거나 y이라면
			system("clear"); // 터미널 창을 clear시킴
		}
		else {
			break; 
		}
	}

	/*close() : 소켓종료 */
	close(sock);
	return 0;
}

//개인정보 출력 함수
void print_Info(struct sockaddr_in* sv) {
	char buf[20]; //서버 IP를 저장하기위한 임시 공간
	inet_ntop(AF_INET, &sv->sin_addr, buf, sizeof(buf));	
	//서버 IP를 문자열에 저장
	printf("===============================================================================\n");
	printf("%c[1;35m", 27); // 터미널에 출력되는 메시지를 연보라 색으로 설정
	printf("\t\t\t@이름 : 최은빈\n"); // 이름 출력
	printf("\t\t\t@학번 : 2014136129\n"); // 학번 출력
	printf("\t\t\t@과목 : 네트워크프로그래밍\n"); // 과목 출력
	printf("\t\t\t@교수님 : 서희석 교수님\n");  // 교수님 이름 출력
	printf("%c[0m", 27);// 기본 색으로 설정
	printf("===============================================================================\n");
	printf("\n\n> FTP 서버 (%s:%d) 접속 중입니다.  \n", buf, ntohs(sv->sin_port)); //연결한 서버의 IP와 포트번호를 출력
}

//서버의 파일/디렉토리 목록 탐색 함수
//현재 서버에 어떠한 파일, 디렉토리들이 있는지 모르기 때문에 해당 기능을 통해 내가 받고 싶은 파일의 정확한 경로를 확인함
//해당 함수는 반복적으로 파일을 하향,상향 탐색이 가능함
void server_scanDir(int sock, char* spath) {

	char scanPath[SCANPATHSIZE]; //탐색할 디렉토리 경로를 저장할 공간
	char addPath[256]; //추가 탐색할 디렉토리의 이름을 저장할 공간 
	int i; //반복문을 돌기 위한 반복문 변수
	
	strcpy(scanPath, spath); // spath의 문자형 배열을 scanPath에 복사
	
	while (1) { // 무한 루프를 돌면서
		server_scanPath(sock, scanPath);
		//해당 경로에 존재하는 파일,디렉토리 목록 출력

		printf("> 탐색할 디렉토리(나가기 : quit) : ");	
		//추가 탐색을 하기 위함 (하향 또는 상향)
		scanf("%s", addPath);

		if (strcmp(addPath, "quit") == 0 || strcmp(addPath, "QUIT") == 0 || strcmp(addPath, "Quit") == 0) {	//탐색 종료 명령 수행
			break;
		}
		else if (strcmp(addPath, ".") == 0) //현재 디렉토리 유지
		{
			continue;
		}
		else if (strcmp(addPath, "..") == 0) //상위 디렉토리로 이동
		{
			 
			 if (strcmp(scanPath, "/home") != 0)
			{
				//저장되어 있는 경로에서 제일 마지막 디렉토리 부분을 삭제
				for (i = strlen(scanPath) - 1; i >= 0; i--) {
					scanPath[i] = '\0'; 
					// 마지막 인덱스에 '\0'문자열 대입
					if (i >= 1 && scanPath[i - 1] == '/') {
				// 만약 i이 1보다 크거나 작고, scanPath의 i-1의 값이 '/'이라면
						scanPath[i - 1] = '\0';
						// i-1 인덱스에 '\0'문자열 대입
						break;
					}
				}
			}
			else{
				printf("%c[1;31m", 27); // 터미널에 출력되는 메시지를 빨간색으로 설정
				 printf("\n> 최상위 디렉토리이기 때문에 더이상 상위 디렉토리로 접근할 수 없습니다. 현재 디렉토리를 반환합니다.\n");
				printf("%c[0m", 27); // 기본색으로 설정
	}
		}
		else { // 하향 탐색
			sprintf(scanPath, "%s/%s", scanPath, addPath);	
			// 현재 경로에 추가된 디렉토리를 추가함	
		}
	}

}
//서버로부터 해당 경로에 있는 파일,디렉토리 목록을 모두 전송 받음
void server_scanPath(int sock, char* spath) {

	/*msgType 전송 : 필드 크기는 1bytes(uint8_t)로 고정*/
	uint8_t msgType = ScanDirReq; //파일, 디렉토리 탐색 메세지 타입 지정
	ssize_t numBytesSent = send(sock, &msgType, sizeof(msgType), 0);	// 메세지 타입 전송

																		
	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	// numBytesSent의 값이 -1이라면 send 에러 출력
	else if (numBytesSent != sizeof(msgType)) error_handling("sent unexpected number of bytes");
	// numBytesSent와 msgType의 크기가 다르면 제대로된 메시지가 send 되지 않았다는 의미이므로 에러 출력

	/*스캔 경로를 서버에 전송 : 필드 크기를 512byte로 고정*/
	char path_str[SCANPATHSIZE]; //경로를 저장할 공간
	memset(path_str, 0, SCANPATHSIZE); //초기화시킴
	strcpy(path_str, spath); //경로를 복사함

	numBytesSent = send(sock, path_str, SCANPATHSIZE, 0); 
	//서버에 클라이언트가 확인하기를 원하는 경로를 전송함

														  
	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	// numBytesSent의 값이 -1이라면 send 에러 출력
	else if (numBytesSent != SCANPATHSIZE) {
	// numBytesSent와 SCANPATHSIZE 크기가 다르면 제대로된 메시지가 send 되지 않았다는 의미이므로 에러 출력
		error_handling("sent unexpected number of bytes");
	}
	

	/*하위 파일/디렉토리 개수를 수신 : 필드 크기는 uint32_t로 고정*/
	uint32_t netNumFile; //네트워크 바이트오더의 파일 개수 저장 변수
	uint32_t numFile; //호스트 바이트오더의 파일 개수 저장 변수
	ssize_t numBytesRcvd = recv(sock, &netNumFile, sizeof(netNumFile), MSG_WAITALL);	//서버로 부터 파일 개수 수신

																						
	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	// numBytesSent의 값이 -1이라면 recv 에러 출력
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	// numBytesSent의 값이 0이라면 연결 에러 출력
	else if (numBytesRcvd != sizeof(netNumFile)) error_handling("recv unexpected number of bytes");
	// numBytesSent와 netNumFile 크기가 다르면 제대로된 메시지가 recv 되지 않았다는 의미이므로 에러 출력
	
	numFile = ntohl(netNumFile); //수신된 파일 개수를 호스트 바이트오더로 변환

	/*파일, 디렉토리 목록을 수신 : 필드 크기는 위 uint32_t 내용*/
	uint32_t rcvdNumFile = 0; //수신된 파일 목록 개수

	//파일 정보 카테고리 출력
	printf("\n===============================================================================\n");
	printf("%-6s%-25s\t\t%5s\n", "Type", "File name", "File size");
	printf("===============================================================================\n");
	while (rcvdNumFile < numFile) {	//수신한 파일, 디렉토리 개수만큼 반복
		char infoBuf[SCANPATHSIZE]; 
		//파일, 디렉토리 정보를 임시 저장할 공간
		numBytesRcvd = recv(sock, infoBuf, SCANPATHSIZE, 0);	
		//서버로부터 파일, 디렉토리 정보를 수신 받음

		/*비정상 수신시 에러 발생*/
		if (numBytesRcvd == -1) error_handling("recv() error");
		// numBytesSent의 값이 -1이라면 recv 에러 출력
		else if (numBytesRcvd == 0) error_handling("peer connection closed");
		// numBytesSent의 값이 0이라면 연결 에러 출력

		rcvdNumFile++;	//수신된 파일 개수 증가


		if (infoBuf[0] == 'D') { 
		//수신된 파일 정보가 D로 시작한다면 디렉토리
			printf("%c[1;36m", 27);	
			// 디렉토리는 글자를 파란색으로 표시
			printf("%s", infoBuf);	//수신된 정보 출력
			printf("%c[0m", 27); // 기본색으로 설정
		}
		else {
			printf("%s", infoBuf);	//일반 파일 수신 정보 출력

		}
		usleep(5000); // 반복문에서 메시지가 제대로 recv 하게 하기 위해 딜레이해줌
	}
	printf("===============================================================================\n\n");

	/*파일,디렉토리 목록 수신 성공 메시지(msgType:FileAck)를 서버에게 전송*/
	msgType = FileAck;
	numBytesSent = send(sock, &msgType, sizeof(msgType), 0);

	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	else if (numBytesSent != sizeof(msgType)) error_handling("sent unexpected number of bytes");
}

//클라이언트의 파일/디렉토리 목록 탐색 함수
//현재 클라이언트에 어떠한 파일, 디렉토리들이 있는지 모르기 때문에 해당 기능을 통해 내가 받고 싶은 파일의 정확한 경로를 확인 함.
//해당 함수는 반복적으로 파일을 하향,상향 탐색이 가능함
void client_scanDir(char *cpath) {

	struct dirent **namelist; //파일 / 디렉토리 정보 저장
	int n; //파일 / 디렉토리 개수 저장
	int i = 0; // 반복자를 정의하는 정수형 변수
	char scanPath[SCANPATHSIZE]; //탐색 경로를 저장할 공간		
	char addPath[256]; //추가 탐색 경로를 입력받아 저장할 공간
	strcpy(scanPath, cpath);
	//함수 인자로 들어온 변수를 탐색 경로에 복사

	n = scandir(scanPath, &namelist, NULL, alphasort); 
	//해당 경로의 파일/디렉토리를 스캔하고 개수를 반환
	if (n == -1) {	//개수가 -1 이면 오류
		printf("> 경로를 찾을 수 없습니다.\n"); 
		// 경로를 찾을 수 없다는 메시지 출력
	}
	struct stat attrib;	
	//개별 파일, 디렉토리의 상세 정보를 가져오기 위한 구조체 변수

	while (1) { // 무한 루프를 돌면서
		//파일 정보 카테고리 출력
		printf("\n===============================================================================\n");
		printf("%-6s%-25s\t\t%5s\n", "Type", "File name", "File size");
		printf("===============================================================================\n");
		while (n--) {	//n개의 파일에 대해서 모두 출력함
			char temp[BUFSIZE] = ""; 
			// temp의 배열을 BUFSIZE 만큼 생성하여 초기화시킴
			sprintf(temp, "%s/%s", scanPath, namelist[n]->d_name); //파일 경로+파일 이름으로 전체 경로를 생성

			if (stat(temp, &attrib) != -1)	
			//해당 파일의 상세 정보를 읽어 옴
			{
				if (S_ISDIR(attrib.st_mode))	
				//디렉토리라면
				{
					printf("%c[1;36m", 27);	//글자색 변경
					printf("%-6s%-25s\t\t%-5d\n", "D", namelist[n]->d_name, (int)attrib.st_size); //파일 타입과 이름, 크기를 출력
					printf("%c[0m", 27);	//글자색 복구
				}
				else {
					printf("%-6s%-25s\t\t%-5d\n", "F", namelist[n]->d_name, (int)attrib.st_size); //파일 타입과 이름, 크기를 출력
				}
			}
			free(namelist[n]); //해당 공간은 메모리 해제
		}
		free(namelist);//최종 메모리 해제

		printf("===============================================================================\n\n");
		printf("> 탐색할 디렉토리(나가기 : quit) : ");
		scanf("%s", addPath);	// 추가 탐색 (하향 또는 상향)

		if (strcmp(addPath, "quit") == 0 || strcmp(addPath, "QUIT") == 0 || strcmp(addPath, "Quit") == 0) {	// 탐색 종료 명령 수행
			break;
		}
		else if (strcmp(addPath, "..") == 0) // 상위 디렉토리로 이동
		{

			 if (strcmp(scanPath, "/home") != 0){
				
			// 저장되어 있는 경로에서 제일 마지막 디렉토리 부분을 삭제함
				for (i = strlen(scanPath) - 1; i >= 0; i--) {

					scanPath[i] = '\0';

					if (i >= 1 && scanPath[i - 1] == '/') {
						scanPath[i - 1] = '\0';
						break;
					}
				}
			}
			else{
			printf("%c[1;31m", 27); // 터미널에 출력되는 메시지를 빨간색으로 설정
				 printf("\n> 최상위 디렉토리이기 때문에 더이상 상위 디렉토리로 접근할 수 없습니다. 현재 디렉토리를 반환합니다.\n");
				printf("%c[0m", 27); // 기본색으로 설정
						
			}
			n = scandir(scanPath, &namelist, NULL, alphasort);

		}
		else if (strcmp(addPath, ".") == 0) // 현재 디렉토리 유지
		{
			n = scandir(scanPath, &namelist, NULL, alphasort);
		}
		else {	// 하향 탐색
			sprintf(scanPath, "%s/%s", scanPath, addPath); 
			// 현재 경로에 추가된 디렉토리를 추가함	

			n = scandir(scanPath, &namelist, NULL, alphasort);
			// 탐색 수행
			if (n == -1) { // scandir의 반환값인 n이 -1 이면 오류
				printf("> 경로를 찾을 수 없습니다.\n");
				break;
			}
		}
	}
}

//파일 업로드 처리 함수
void FileUploadProcess(int sock, char* A_filePath, char* A_fileName) {
	/*msgType 전송 : 필드 크기가 1bytes(uin8_t)로 고정*/
	uint8_t msgType = FileUpReq;	//파일 업로드 메세지 타입 생성
	ssize_t numBytesSent = send(sock, &msgType, sizeof(msgType), 0);	//생성한 메세지타입을 서버에 전송함

		
	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	else if (numBytesSent != sizeof(msgType)) error_handling("sent unexpected number of bytes");

	/*파일이름을 서버에 전송 : 필드 크기는 256bytes로 고정*/
	char fileName[FILENAMESIZE]; //파일 이름 저장 공간
	memset(fileName, 0, FILENAMESIZE); //배열 초기화
	strcpy(fileName, A_fileName); //이름 복사
	numBytesSent = send(sock, fileName, FILENAMESIZE, 0);	
	//업로드할 파일이름을 서버에 전송

				
	/*비정상 수신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	// 만약 numBytesSent이 -1이라면 send 에러 출력
	else if (numBytesSent != FILENAMESIZE) error_handling("sent unexpected number of bytes");
	// 만약 numBytesSent과 FILENAMESIZE이 다르다면 제대로된 메시지가 전송되지 않았다는 메시지 출력

	/*파일 크기를 서버에 전송: 필드 크기는 uint32_t으로 고정*/
	struct stat sb;	//파일 정보 저장 공간
	char filePath[FILEPATHSIZE]; //파일 경로 저장 공간
	memset(filePath, 0, FILEPATHSIZE); //배열 초기화
	sprintf(filePath, "%s/%s", A_filePath, fileName);
	//파일경로와 파일이름으로 파일의 절대경로 생성
	printf("\n> %s 파일 업로드\n",filePath);				
	//업로드할 파일의 절대경로+이름정보 출력
	
	if (stat(filePath, &sb) < 0) error_handling("stat() error");	
	//해당 파일의 정보를 가져오고, 오류 발생시 오류 출력후 종료

	uint32_t fileSize = sb.st_size;	
	//파일 크기를 호스트 바이트오더 변수에 저장
	uint32_t netFileSize = htonl(fileSize);	
	//파일 크기를 네트워크 바이트오더로 변경 후 저장
	numBytesSent = send(sock, &netFileSize, sizeof(netFileSize), 0);	//서버에 파일 크기 전송

																		
	/*비정상 전송시 오류 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	// 만약 numBytesSent이 -1이라면 send 에러 출력
	else if (numBytesSent != sizeof(netFileSize)) error_handling("sent unexpected number of bytes");
	// 만약 numBytesSent와 netFileSize의 크기가 다르면 제대로된 메시지가 보내지지 않았으므로 에러 출력

	/*파일내용을 서버에 전송 : 필드 크기를 위 fileSize  변수 값*/
	if (access(filePath, R_OK | W_OK) != 0)	
	// 읽기 쓰기가 가능한지 확인 (권한을 확인하기 위함)
	{
		error_handling("파일 접근 권한이 없습니다.");
	}
	FILE *fp = fopen(filePath, "r"); //서버로 전송할 파일 오픈
	if (fp == NULL) error_handling("fopen() error");	
	//오픈 실패시 에러 발생
	uint32_t i = 0;	//반복자를 정의하는 정수형 변수
	double percent = 0.0; //업로드 진행률 계산 변수
	time_t start_time; //시간 측정 변수 (시작 시각)
	time(&start_time); //시각 획득
	uint32_t sentFileSize = 0; //보낸 파일 크기 저장 변수
	printf("\n");
	while (!feof(fp)) { //파일의 끝까지 반복하면서
		char fileBuf[BUFSIZE];	
		//전송할 파일에서 정보를 가져와서 임시로 저장해 둘 공간
		size_t numBytesRead = fread(fileBuf, sizeof(char), BUFSIZE, fp);		//파일로부터 데이터를 읽어옴
		if (ferror(fp)) error_handling("fread() error");	
		//파일 읽기 에러시 에러 출력후 종료

		numBytesSent = send(sock, fileBuf, numBytesRead, 0);	
		//서버로 send 함수를 통해 데이터 전송함

																
		/*비정상 전송시 에러 발생*/
		if (numBytesSent == -1) error_handling("send() error");
		// send 함수의 반환값 numBytesSent이 -1이라면 send 에러 출력
		else if (numBytesSent != numBytesRead) error_handling("sent unexpected number of bytes");
		// send 함수의 반환값 numBytesSent과 numBytesRead이 다르면 제대로된 메시지가 아니라는 의미이므로 에러 메시지 출력

		sentFileSize += numBytesSent;	
		//전송한 바이트 수만큼 전송한 크기 추가

		percent = (sentFileSize / (double)fileSize) * 100.0;	
		//전송률 계산

		//시각적으로 진행 정도를 확인할 수 있도록 막대그래프로 표시
		printf("> progress : ["); 
		for (i = 0; i <20; i++) { // 반복문을 돌면서
			printf("%c[1;33m", 27); // 글자색을 노랑으로 설정
			if (i < (int)percent / 5) 
			// 전송률을 5으로 나눠 이 수만큼 i가 작으면 
				
				printf("■ "); // ■ 을 출력
			else // 아니면
				printf("□ "); // □ 을 출력
		}
		printf("%c[0m", 27); //글자색 복구
		printf("] %.2lf%%\r", percent);	//전송률 출력
		fflush(stdout);	//출력 버퍼 비우기
		usleep(100000);	//딜레이

	}
	printf("\n");
	fclose(fp); //파일 닫기

	/*파일 전송완료*/
	/*서버로부터의 ack 메시지 수신후 화면에 성공여부를 출력*/
	ssize_t numBytesRcvd = recv(sock, &msgType, sizeof(msgType), MSG_WAITALL); 

	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	else if (numBytesRcvd != sizeof(msgType)) error_handling("recv unexpected number of bytes");

	if (msgType == FileAck) { //수신된 메세지 타입이 File Ack인 경우
		printf("\n> %s 업로드 성공!!\n\n", fileName); 
		// 업로드 성공 메시지 출력	
		printf("> 파일의 크기는 %u Bytes 입니다\n",fileSize);
		// 파일 크기를 출력
		time_t end_time; //종료 시각 저장 변수
		time(&end_time); //종료 시각 저장
		int tmp = difftime(end_time, start_time);	
		//업로드 경과 시간 계산


		//계산된 시간에 따라 업로드 시간 출력
		if (tmp <= 60)
			printf("> 업로드 시간 : %d초\n\n", tmp);
		else if (tmp <= 3600)
			printf("> 업로드 시간 : %d분 %d초\n", tmp / 60, tmp % 60);
		else
			printf("> 업로드 시간 : %d시 %d분 %d초\n", tmp / 3600, (tmp % 3600) / 60, tmp % 60);
	}
	else
		printf("> %s 파일 업로드 실패!!\n", fileName);
		 //업로드 실패시 실패 메세지 출력

}

//파일 다운로드 처리 함수
void HandleFileDownload(int sock, char* A_filePath, char* A_fileName) {

	/*msgType 전송 : 필드 크기는 1bytes(uint8_t)로 고정*/
	uint8_t msgType = FileDownReq;	//파일 다운로드 메세지 타입 생성
	ssize_t numBytesSent = send(sock, &msgType, sizeof(msgType), 0);	//서버에 메세지 타입 전송

																		
	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	else if (numBytesSent != sizeof(msgType)) 	error_handling("sent unexpected number of bytes");

	/*파일이름을 서버에 전송 : 필드 크기를 256byte로 고정*/
	char fileName[FILENAMESIZE];	//다운로드할 파일 이름을 저장할 공간
	memset(fileName, 0, FILENAMESIZE); //배열 초기화
	strcpy(fileName, A_fileName); //다운로드할 파일 이름을 복사

	char filePath[FILEPATHSIZE]; 
	//다운로드할 파일의 경로를 저장할 공간		
	memset(filePath, 0, FILEPATHSIZE); //배열 초기화
	sprintf(filePath, "%s/%s", A_filePath, fileName);	
	//경로와 이름을 합쳐서 절대 경로를 만듦
	numBytesSent = send(sock, filePath, FILENAMESIZE, 0);	
	//절대 경로를 서버로 전송

							
	/*비정상 전송시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	else if (numBytesSent != FILEPATHSIZE) 	error_handling("sent unexpected number of bytes");

	/*파일 크기를 수신 : 필드 크기는 uint32_t로 고정*/
	printf("\n>  %s 파일 다운로드\n", filePath);	
	//다운로드하는 파일 정보 출력

	uint32_t netFileSize; //네트워크 바이트오더의 파일 크기 저장 변수
	uint32_t fileSize; //호스트 바이트오더의 파일 크기 저장 변수
	ssize_t numBytesRcvd = recv(sock, &netFileSize, sizeof(netFileSize), MSG_WAITALL);	//서버로부터 다운로드할 파일의 크기를 수신 받음.

						
	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	else if (numBytesRcvd != sizeof(netFileSize)) error_handling("recv unexpected number of bytes");
	fileSize = ntohl(netFileSize);	//호스트 바이트 오더로 파일 크기 변환

	/*파일 내용을 수신 : 필드 크기는 위 uint32_t 내용*/
	FILE *fp = fopen(fileName, "w");	
	//수신 받을 파일 이름으로 파일 생성
	if (fp == NULL) error_handling("fopen() error");	
	//파일 생성 오류 발생 시 오류 처리

	uint32_t rcvdFileSize = 0; //수신 받은 파일 크기 저장 변수
	uint32_t i = 0;	//반복자
	double percent = 0.0; //수신률 계산
	time_t start_time; //수신 완료 시간 계산을 위한 시각 저장 변수(시작 시각)
	time(&start_time); //시작 시각 저장
	printf("\n");
	while (rcvdFileSize < fileSize) {
	//수신된 파일크기 정보가 서버로부터 받은 파일 크기와 동일해질 때 까지 반복
		char fileBuf[BUFSIZE];		
		//수신한 파일 데이터를 임시로 저장할 공간
		numBytesRcvd = recv(sock, fileBuf, BUFSIZE, 0);		
		//서버로부터 파일 데이터 수신

					
		/*비정상 수신시 에러 발생*/
		if (numBytesRcvd == -1) error_handling("recv() error");
		else if (numBytesRcvd == 0) error_handling("peer connection closed");

		fwrite(fileBuf, sizeof(char), numBytesRcvd, fp);	
		//수신한 파일 데이터를 새로 생성한 파일에 작성
		if (ferror(fp)) error_handling("fwrite() error");	
		//파일 작성 에러 발생  처리

		rcvdFileSize += numBytesRcvd;	//수신된 파일크기를 업데이트

		percent = (rcvdFileSize / (double)fileSize) * 100.0;	
		//수신률 계산
						
		//시각적으로 진행 정도를 확인할 수 있도록 막대그래프로 표시
		printf("> progress : [");
		for (i = 0; i < 20; i++) { // 반복문을 돌면서
			printf("%c[1;33m", 27); // 글자색을 노랑으로 설정
			if (i < (int)percent / 5)
			// 전송률을 5으로 나눠 이 수만큼 i가 작으면 
				printf("■ "); // ■ 을 출력
			else // 아니면
				printf("□ "); // □ 을 출력
		}
		printf("%c[0m", 27); //글자색 복구
		printf("] %.2lf%%\r", percent);	//수신률 출력
		fflush(stdout);	//출력 버퍼 비우기
		usleep(100000);	//딜레이
	}
	printf("\n");

	fclose(fp); //파일 닫기

	/*파일 수신 완료*/
	/*파일 수신 성공 메시지(msgType:FileAck)를 서버에게 전송*/
	msgType = FileAck;
	numBytesSent = send(sock, &msgType, sizeof(msgType), 0);

	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	else if (numBytesSent != sizeof(msgType)) error_handling("sent unexpected number of bytes");
	else {	//메세지가 정상적으로 송신 되었다면
		printf("\n> %s 다운로드 성공!!\n\n", fileName);	//업로드한 파일 정보를 출력
		printf("> 파일의 크기는 %u Bytes 입니다\n",fileSize);
		time_t end_time; //종료 시각 저장 변수
		time(&end_time); //종료 시각 저장
		int tmp = difftime(end_time, start_time);	
		//다운로드 시간 계산

		//계산된 시간을 출력
		if (tmp <= 60)
			printf("> 다운로드 시간 : %d초\n\n", tmp);
		else if (tmp <= 3600)
			printf("> 다운로드 시간 : %d분 %d초\n\n", tmp / 60, tmp % 60);
		else
			printf("> 다운로드 시간 : %d시 %d분 %d초\n\n", tmp / 3600, (tmp % 3600) / 60, tmp % 60);

	}
}

//에러 출력 및 프로그램 종료 함수
void error_handling(char *message) {
	printf("%c[1;31m", 27);	//글자색 변경(빨간색)
	printf("\n> %s\n", message);//에러 메세지 출력
	printf("> 프로그램이 종료됩니다.\n"); //프로그램 종료 메시지
	printf("%c[0m", 27);	//기본색으로 복귀
	exit(1);//프로그램 에러 종료
}

//문자열을 모두 소문자로 변경해주는 함수
char *strlwr(char *str)
{
	unsigned char *p = (unsigned char *)str;

	while (*p) {
		*p = tolower((unsigned char)*p); // 각 문자를 소문자로 변경
		p++; // p 위치 증가
	}

	return str;
}
//키보드 입력 버퍼 초기화 함수
void clear_buffer()
{
	int ch; 
	while ((ch = getchar()) != '\n' && ch != EOF);
}

