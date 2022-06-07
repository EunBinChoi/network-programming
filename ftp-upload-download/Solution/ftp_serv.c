/*서버와 클라이언트 간의 파일 전송 프로그램 - 서버*/
#include<time.h>
#include<string.h>
#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<sys/stat.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<netinet/in.h>
#include<dirent.h>


#define FILENAMESIZE 256 //파일 이름 최대 길이를 256으로 정의 
#define FILEPATHSIZE 256 //파일 경로 최대 길이를 256으로 정의
#define SCANPATHSIZE 512 //파일 목록 찾기에서 사용되는 경로 최대 길이를 512으로 정의
#define FileUpReq  01 //파일 업로드 메세지 번호를 01으로 정의 
#define FileDownReq 02 //파일 다운로드 메세지 번호를 02으로 정의
#define ScanDirReq 03 //파일 디렉토리 목록 스캔 메세지 번호를 03으로 정의
#define FileAck 11 //Ack 값을 11로 정의
#define ExitReq 12 //Exit 값을 12로 정의
#define BUFSIZE 512 //버퍼의 크기를 512으로 정의

void error_handling(char *message); //에러 출력 및 프로그램 종료하는 함수 정의
void HandleFileUpload(int clnt_sock); //파일 업로드 요청시 호출되는 함수 정의
void FileDownloadProcess(int clnt_sock); //파일 다운로드 요청 시 호출되는 함수 정의
void scanDir_server(int clnt_sock); //서버의 파일/디렉토리 목록을 반환해주는 함수 정의
void print_Info(); //개인 정보 출력하는 함수 정의

int main(int argc, char*argv[]) //메인 함수 정의
{
	/*명령 파라미터의 개수가 2인지 검사하는 부분*/
	if (argc != 2) { // 명령 파라미터의 개수가 2개가 아니라면
		printf("Usage : %s <port>\n", argv[0]); // 사용하는 방법 출력
		exit(1);// 종료
	}
	system("clear"); //터미널 창을 정리하기 위함

	/*socket(): tcp 서버 소켓 생성*/
	int serv_sock = socket(PF_INET, SOCK_STREAM, 0); // 서버가 클라이언트와 통신하기 위해 소켓을 생성함
	if (serv_sock == -1) error_handling("socket error"); // 만약 소켓이 제대로 생성되지 않았으면 에러 출럭

	/*bind() : 서버가 사용할 포트 주소를 서버 소켓과 묶음*/
	struct sockaddr_in serv_addr; // 서버 소켓 주소를 표현하는 구조체 변수 선언
	memset(&serv_addr, 0, sizeof(serv_addr)); // 버퍼를 초기화함
	serv_addr.sin_family = AF_INET; // serv_addr의 sin_family 멤버를 AF_INET으로 설정
	serv_addr.sin_addr.s_addr = htonl(INADDR_ANY); // serv_addr.sin_addr의 s_addr 맴버를 INADDR_ANY으로 설정
	serv_addr.sin_port = htons(atoi(argv[1])); // 메인함수 호출 시 전달된 인자 중 argv[1]를 serv_addr의 sin_port으로 설정
	if (bind(serv_sock, (struct sockaddr*)&serv_addr, sizeof(serv_addr)) == -1) // 소켓번호와 소켓주소를 연결해주기 위해 bind() 함수 호출
		error_handling("bind() error"); // 만약 바인딩이 제대로 되지 않았으면 에러 출력

	/*개인정보 출력*/
	print_Info();

	/*서버 시작 메세지 출력*/
	printf("\n\n> FTP 서버가 시작되었습니다!\n");


	/*listen() : 서버 소켓을 리슨 소켓으로 변경*/
	if (listen(serv_sock, 5) == -1) 
	// 클라이언트로부터 요청을 기다리는 listen 함수 호출
		error_handling("listen() error"); // 만약 위의 조건문에서 -1이 반환되면 에러를 출력


	/*accept() : 연결후 생성된 클라이언트 소켓을 리턴*/
	struct sockaddr_in clnt_addr; // 클라이언트 소켓 주소를 표현하는 구조체 변수 선언
	socklen_t clnt_addr_len = sizeof(clnt_addr); // clnt_addr_len에 클라이언트 주소의 길이를 저장해줌 
	int clnt_sock = accept(serv_sock, (struct sockaddr*)&clnt_addr, &clnt_addr_len); // 클라이언트와 설정된 연결들을 실제로 받아들이기 위한 accept 함수 호출
	if (clnt_sock == -1) // 만약 accept의 함수의 반환값이 -1이라면
		error_handling("accept() error"); // 에러 출력

	/*접속한 클라이언트 정보를 화면에 출력*/
	printf("> 클라이언트 (%s:%d) 가 접속하였습니다.\n", inet_ntoa(clnt_addr.sin_addr), ntohs(clnt_addr.sin_port));

	while (1) { // 서버는 항상 떠있어야하므로 무한 루프 

		/*msgType 수신 : 필드 크기는 1bytes(uint8_t)로 고정*/
		uint8_t msgType; // 메시지를 저장하는 변수 선언
		ssize_t numBytesRcvd = recv(clnt_sock, &msgType, sizeof(msgType), MSG_WAITALL); // 해당 메시지를 클라이언트로부터 받음
		

		/*msgType 값에 따라 분기처리*/
		if (numBytesRcvd == -1) error_handling("recv() error");
		// 만약 recv 함수 반환값이 -1이라면 에러 출력
		else if (numBytesRcvd == 0) error_handling("peer connection closed");
		// 만약 recv 함수 반환값이 0이라면 연결이 끊겼다는 에러 메시지 출력
		else if (numBytesRcvd != sizeof(msgType)) error_handling("recv unexpected number of bytes"); // 만약 recv 함수 반환값과 메시지의 크기가 같지 않으면 서버가 제대로된 메시지를 recv 한 것이 아니므로 에러 메시지 출력

	
		/*파일 업로드 처리*/
		if (msgType == FileUpReq) HandleFileUpload(clnt_sock);
		// 만약 msgType의 값이 FileUpReq과 같으면 파일을 업로드하는 함수 호출

		/*파일 다운로드 처리*/
		else if (msgType == FileDownReq) FileDownloadProcess(clnt_sock);
		// 만약 msgType의 값이 FileDownReq과 같으면 파일을 다운로드하는 함수 호출
		
		/*해당 경로에 존재하는 디렉토리/파일 목록을 반환하는 명령 처리*/
		else if (msgType == ScanDirReq) scanDir_server(clnt_sock);
		// 만약 msgType의 값이 ScanDirReq과 같으면 서버의 디렉토리를 검사하는 함수 호출

		/*연결 종료*/
		else if (msgType == ExitReq) break;
		// 만약 msgType의 값이 ExitReq과 같으면 서버 종료

		/*메세지 타입 에러*/
		else printf("> 명령어를 찾을 수 없습니다.\n");

	}

	/*close() : 소켓을 닫음*/
	close(clnt_sock);
	close(serv_sock);
	return 0;


}

/*개인 정보를 출력해주는 함수*/
void print_Info() {
	printf("===============================================================================\n");
	printf("%c[1;35m", 27); // 터미널에 출력되는 메시지를 연보라 색으로 설정
	printf("\t\t\t@이름 : 최은빈\n"); // 이름 출력
	printf("\t\t\t@학번 : 2014136129\n"); // 학번 출력
	printf("\t\t\t@과목 : 네트워크프로그래밍\n"); // 과목 출력
	printf("\t\t\t@교수님 : 서희석 교수님\n"); // 교수님 이름 출력
	printf("%c[0m", 27); // 기본 색으로 설정
	printf("===============================================================================\n");

}

//서버의 파일/디렉토리 목록을 반환해주는 함수
void scanDir_server(int clnt_sock) {
	char scanPath[SCANPATHSIZE];	//경로 저장하기 위한 문자형 배열 선언
	ssize_t numBytesRcvd = recv(clnt_sock, scanPath, SCANPATHSIZE, MSG_WAITALL);	
	// 클라이언트로부터 요청 경로를 받아와 해당 반환값을 numBytesRcvd에 대입
															
	/*비정상 읽기시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	// 만약 numBytesRcvd의 값이 -1이라면 recv 함수 에러 출력
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	// 만약 numBytesRcvd의 값이 0이라면 연결이 종료되었다는 메시지 출력
	else if (numBytesRcvd != SCANPATHSIZE) error_handling("recv unexpected number of bytes");
	// 만약 numBytesRcvd의 값과 SCANPATHSIZE이 동일하지 않으면 잘못된 메시지를 recv했다는 의미이므로 이에 해당하는 에러 출력

	struct dirent **namelist;
	//파일 / 디렉토리 정보 저장하는 구조체 정의
	int n; //파일 / 디렉토리 개수 저장하는 정수형 변수 선언

	n = scandir(scanPath, &namelist, NULL, alphasort);	
	//해당 경로의 파일/디렉토리를 스캔하고 개수를 반환

	if (n == -1) {	// 만약 해당 파일/디렉토리의 개수가 -1 이면 오류
		printf("> 경로를 찾을 수 없습니다."); // 경로를 찾을 수 없다는 에러 출력
	}

	// 만약 해당 파일/디렉토리의 개수가 제대로 된 값이라면
	uint32_t numFile = n; // 파일 개수를 저장
	uint32_t netNumFile = htonl(numFile);	
	//바이트 오더를 네트워크 타입으로 변환
	ssize_t numBytesSent = send(clnt_sock, &netNumFile, sizeof(netNumFile), 0); //파일 개수를 클라이언트에게 전송

																		
	/*비정상 전송시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	// 만약 전송된 파일의 개수의 반환값인 numBytesSent이 -1이라면 send 함수 에러 출력
	else if (numBytesSent != sizeof(netNumFile)) error_handling("sent unexpected number of bytes");
	// 만약 전송된 파일의 개수의 반환값인 numBytesSent과 netNumFile의 크기가 맞지 않는다면 예기치 않은 메시지가 클라이언트에게 전송되었다는 의미이므로 에러 출력

	struct stat attrib; 
	//개별 파일, 디렉토리의 상세 정보를 가져오기 위한 구조체 변수

	while (n--) { // n개의 파일에 대해서 클라이언트에게 파일 정보를 전송함
		char temp[BUFSIZE]; // BUFSIZE 크기의 문자형 배열 선언
		char tempBuf[BUFSIZE]; // BUFSIZE 크기의 문자형 배열 선언
		sprintf(temp, "%s/%s", scanPath, namelist[n]->d_name);	
		// 파일 경로+파일 이름으로 전체 경로를 생성하여 temp에 저장

		if (stat(temp, &attrib) != -1)	
		// stat 함수를 통해 해당 파일의 상세 정보를 읽어 옴
		{
			if (S_ISDIR(attrib.st_mode)) // 디렉토리라면
			{

				sprintf(tempBuf, "%-6s%-25s\t\t%-5d\n", "D", namelist[n]->d_name, (int)attrib.st_size); 
		// 파일 타입과 이름, 크기를 문자열화 시켜 tempBuf에 저장함
			}
			else { //일반 파일이라면
				sprintf(tempBuf, "%-6s%-25s\t\t%-5d\n", "F", namelist[n]->d_name, (int)attrib.st_size); 
		// 파일 타입과 이름, 크기를 문자열화 시켜 tempBuf에 저장함
			}
			numBytesSent = send(clnt_sock, tempBuf, BUFSIZE, 0);		// 문자열화 된 데이터를 send 함수를 통해 클라이언트로 전송함
		}

		/*비정상 전송시 에러 발생*/
		if (numBytesSent == -1) error_handling("send() error");
		// send 함수의 반환값인 numBytesSent의 값이 -1이라면
		// send 함수 에러 출력
		else if (numBytesSent != BUFSIZE) error_handling("sent unexpected number of bytes");
		// send 함수의 반환값인 numBytesSent의 값과 BUFSIZE이 다르면
		// 제대로된 메시지를 send 하지 않았다는 것이므로 에러 출력

		free(namelist[n]);	// 해당 공간은 메모리 해제
	}
	free(namelist);	// 최종 메모리 해제

	/*클라이언트로부터의 ack 메시지 수신후 화면에 성공여부 출력*/
	uint8_t msgType;
	numBytesRcvd = recv(clnt_sock, &msgType, sizeof(msgType), MSG_WAITALL);
	// 클라이언트에게 메시지를 recv 함수를 통해 받음

	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	// recv 함수의 반환값이 -1이라면 recv 함수 에러 출력
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	// recv 함수의 반환값이 0이라면 연결이 종료되었다는 에러 출력
	else if (numBytesRcvd != sizeof(msgType)) error_handling("recv unexpected number of bytes");
	// recv 함수의 반환값인 numBytesRcvd과 msgType의 크기가 다르다면 제대로된 메시지를 recv 하지 못했다는 것이므로 에러 출력

	/*수신한 메세지 타입이 FileAck이 맞는지 확인 함.*/
	if (msgType == FileAck) printf("\n> %s 하위 목록 전송 성공!!(%u개)\n", scanPath, numFile); // msgType와 수신한 메시지 타입인 FileAck이 같으면 하위 목록 전송이 성공되었다는 메시지 출력 
	else printf("> %s 하위 목록 전송 실패!!\n", scanPath);
	// msgType와 수신한 메시지 타입인 FileAck이 다르면 하위 목록 전송이 실패되었다는 메시지 출력 

}

// 파일 업로드 요청시 대응 함수
void HandleFileUpload(int clnt_sock) {
	/*파일 이름을 수신 : 필드 크기는 256Bytes로 고정*/
	char fileName[FILENAMESIZE]; 
	// 파일 이름을 저장하기 위해 FILENAMESIZE 크기의 문자형 배열 선언 
	ssize_t numBytesRcvd = recv(clnt_sock, fileName, FILENAMESIZE, MSG_WAITALL);	
	// 클라이언트로부터 파일 이름을 recv 함수로 전송받아 반환값을 numBytesRcvd에 저장함
																			
	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	// 만약 numBytesRcvd의 값이 -1이라면 recv 함수 에러 출력
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	// 만약 numBytesRcvd의 값이 0이라면 연결 종료되었다는 메시지 출력
	else if (numBytesRcvd != FILENAMESIZE) error_handling("recv unexpected number of bytes");
	// 만약 numBytesRcvd의 값과 FILENAMESIZE이 다르면 제대로된 메시지를 recv 하지 않았다는 것이므로 에러 출력

	/*업로드 파일 정보 출력*/
	printf("\n> 클라이언트에서 업로드 요청 중 ....\n");
	printf("> %s 파일이 업로드 됩니다.\n", fileName);

	/*파일 크기를 수신 : 필드 크기를 uint32_t로 고정*/
	uint32_t netFileSize; // 네트워크 바이트오더의 파일 크기를 정의할 변수
	uint32_t fileSize; // 호스트 바이트 오더의 파일 크기를 정의할 변수
	numBytesRcvd = recv(clnt_sock, &netFileSize, sizeof(netFileSize), MSG_WAITALL); // 클라이언트로부터 파일크기를 recv 함수를 통해 전송 받음
																	
	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	// recv 함수 반환값인 numBytesRcvd이 -1이면 recv 함수 에러 출력
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	// recv 함수 반환값인 numBytesRcvd이 0이면 연결이 종료되었다는 메시지 출력
	else if (numBytesRcvd != sizeof(netFileSize)) error_handling("recv unexpected umber of bytes");
	// recv 함수 반환값인 numBytesRcvd과 netFileSize의 크기가 다르면 제대로된 메시지를 recv 하지 않았다는 것이므로 에러 출력

	fileSize = ntohl(netFileSize);	
	// 전송받은 네트워크 바이트오더의 파일 크기 값을 호스트 바이트 오더로 변환
	printf("\n> 파일 크기 : %u Bytes\n", fileSize); // 파일 크기를 출력

										
	/*파일 내용을 수신 : 필드 크기는 위 uint32_t 내용*/
	FILE *fp = fopen(fileName, "w");		
	// 수신한 파일이름으로 파일을 생성함
	if (fp == NULL) error_handling("fopen() eorr");	
	// 파일 생성 실패시 에러 발생

	uint32_t rcvdFileSize = 0; // 수신한 파일 크기를 저장할 변수를 0으로 설정
	while (rcvdFileSize < fileSize)	// 클라이언트가 보낸 파일 크기가 수신된 파일 크기와 같을 때까지 반복문을 돌면서
	{
		char fileBuf[BUFSIZE];	//클라이언트로부터 받을 파일 정보를 저장할 임시 공간인 fileBuf을 문자형 배열로 정의
		numBytesRcvd = recv(clnt_sock, fileBuf, BUFSIZE, 0);	
		// 클라이언트로부터 파일을 recv 함수를 통해 전송 받음
														
		/*비정상 수신시 에러 발생*/
		if (numBytesRcvd == -1) error_handling("recv() error");
		// recv의 반환값인 numBytesRcvd이 -1이라면 recv 함수 에러 출력
		else if (numBytesRcvd == 0) error_handling("peer connection closed"); 
		// recv의 반환값인 numBytesRcvd이 0이라면 연결 종료 출력


		fwrite(fileBuf, sizeof(char), numBytesRcvd, fp);	
		//클라이언트로부터 전송받은 파일 정보를 파일에 작성함

							
		/*파일 쓰기 에러 확인*/
		if (ferror(fp)) error_handling("fwrite() error");

		//수신된 바이트 크기 만큼 수신된 파일 크기 증가
		rcvdFileSize += numBytesRcvd;
	}
	fclose(fp); //파일 닫기

	/*파일 수신 완료*/
	/*파일 수신 성공메시지 (msgTypeFileAck)를 클라이언트에게 전송*/
	uint8_t msgType = FileAck; // msgType의 변수에 FileAck를 대입함
	ssize_t numBytesSent = send(clnt_sock, &msgType, sizeof(msgType), 0);
	// 클라이언트에게 send 함수를 통해 메시지를 보냄

	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	// send 함수의 반환값이 -1이면 send 함수 에러 출력
	else if (numBytesSent != sizeof(msgType)) error_handling("sent unexpected number of bytes");
	// send 함수 반환값인 numBytesSent msgType 크기가 다르면 제대로된 메시지를 send 하지 않았다는 것이므로 에러 출력
	else printf("> %s 파일 업로드 성공!!\n", fileName);	
	// 업로드된 파일 정보 출력

}


/* 파일 다운로드 요청 시 대응 함수 정의*/
void FileDownloadProcess(int clnt_sock) {
	char fileName[FILENAMESIZE]; 
	// 클라이언트가 요청한 파일 이름을 저장할 배열 선언
	ssize_t numBytesRcvd = recv(clnt_sock, fileName, FILENAMESIZE, MSG_WAITALL);	
	//클라이언트로부터 다운로드 요청할 파일 이름(경로 포함)을 전송 받음
																					
	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	// recv 함수 호출시 반환된 값을 저장하는 numBytesRcvd이 -1이라면 recv 함수 에러 출력
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	// recv 함수 호출시 반환된 값을 저장하는 numBytesRcvd이 0이라면 연결 종료 메시지 출력
	else if (numBytesRcvd != FILENAMESIZE) error_handling("recv unexpected number of bytes");
	// numBytesRcvd와 FILENAMESIZE이 다르면 제대로된 메시지를 recv 하지 않았다는 것이므로 에러 출력
	
	struct stat sb; // stat 구조체 변수를 sb으로 정의

	/*현재 상태 출력*/
	printf("\n> 클라이언트에서 다운로드 요청 중 ....\n");
	printf("> %s 파일이 클라이언트에게 전송 됩니다.\n", fileName);

	//파일 정보를 가져오고, 실패시 에러 출력
	if (stat(fileName, &sb) < 0) { // stat 함수의 반환값이 0보다 작으면
		error_handling("stat() error"); // stat 함수 에러 출력 
	}


	uint32_t fileSize = sb.st_size;	
	// 호스트 바이트오더의 파일 사이즈 저장
	uint32_t netFileSize = htonl(fileSize);	
	// 네트워크 바이트오더의 파일 사이즈 저장

	ssize_t numBytesSent = send(clnt_sock, &netFileSize, sizeof(netFileSize), 0); // 클라이언트에게 파일 크기 전송

																					
	/*비정상 송신시 에러 발생*/
	if (numBytesSent == -1) error_handling("send() error");
	// send의 반환값인 numBytesSent이 -1이라면 send 함수 에러 출력
	else if (numBytesSent != sizeof(netFileSize)) error_handling("sent unexpected number of bytes");
	// send의 반환값인 numBytesSent과 netFileSize의 크기가 다르면 제대로된 메시지를 send 하지 않았다는 것이므로 에러 출력


	printf("\n> 파일 크기 : %u Bytes\n", fileSize); 
	// 클라이언트에게 송신할 파일의 크기 출력
									
	if (access(fileName, R_OK | W_OK) != 0)	
	// 파일 읽기(R_OK), 쓰기(W_OK)가 가능한지 확인하기 위한 access 함수 호출  
	// 파일 접근 권한을 확인하기 위함
	{
		error_handling("파일 접근 권한이 없습니다.");
		// 만약 access 함수의 반환값이 0이 아니면 파일 접근 권한이 없다는 메시지 출력
	}

	FILE *fp = fopen(fileName, "r"); // 송신할 파일을 읽기모드로 오픈
	if (fp == NULL) error_handling("fopen() error"); // 실패시 에러 발생

	while (!feof(fp)) { // 파일의 끝까지 반복문 실행
		char fileBuf[BUFSIZE];	
		//파일에서 읽어온 데이터를 임시로 저장할 공간
		size_t numBytesRead = fread(fileBuf, sizeof(char), BUFSIZE, fp);	
		// 파일로부터 데이터를 읽어옴
		if (ferror(fp)) error_handling("fread() error"); 
		//읽기 실패시 에러 발생

		numBytesSent = send(clnt_sock, fileBuf, numBytesRead, 0);		
		//읽어온 데이터를 클라이언트에게 송신함
		usleep(100000);	//딜레이
																	
		/*비정상 송신시 에러 발생*/
		if (numBytesSent == -1) error_handling("send() error");
		else if (numBytesSent != numBytesRead) error_handling("sent unexpected number of bytes");
	}
	fclose(fp); //파일 닫기


	/*파일 전송 완료*/
	/*클라이언트로부터의 ack 메시지 수신후 화면에 성공여부 출력*/
	uint8_t msgType; // 메시지를 저장하기 위한 변수 msgType를 정의
	numBytesRcvd = recv(clnt_sock, &msgType, sizeof(msgType), MSG_WAITALL);
	// 클라이언트에게 메시지를 받음
	

	/*비정상 수신시 에러 발생*/
	if (numBytesRcvd == -1) error_handling("recv() error");
	// recv 함수의 반환값인 numBytesRcvd이 -1이면 recv 함수 에러 출력
	else if (numBytesRcvd == 0) error_handling("peer connection closed");
	// recv 함수의 반환값인 numBytesRcvd이 0이면 연결 종료 메시지 출력
	else if (numBytesRcvd != sizeof(msgType)) error_handling("recv unexpected number of bytes");
	// recv 함수의 반환값인 numBytesRcvd이 msgType의 크기와 다르면 제대로 된 메시지를 recv하지 않았다는 것이므로 에러 출력

	/*수신한 메세지 타입이 FileAck인지 확인하고, 결과 출력*/
	if (msgType == FileAck) printf("> %s 파일 전송 성공!!\n", fileName);
	// msgType와 FileAck이 같으면 파일 전송 성공 메시지 출력
	else printf("> %s 파일 전송 실패!!\n", fileName);
	// 파일 전송 실패 메시지 출력
}

// 에러 출력 및 프로그램 종료 함수
void error_handling(char *message) {
	printf("%c[1;31m", 27);	// 빨간색으로 글자색을 변경
	printf("> %s\n", message); // 에러 메세지 출력함
	printf("%c[0m", 27); // 기본색으로 복귀함
	exit(1); //프로그램 에러 종료
}
