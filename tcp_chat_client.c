#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <errno.h>
#include <arpa/inet.h>

#define SERVER_ADDRESS "127.0.0.1"  // 서버 IP 주소
#define SERVER_PORT 8080            // 서버 포트
#define MAX_BUFFER_SIZE 1024        // 버퍼 크기

void set_nonblocking_mode(int socket_fd);
void handle_user_input(int server_socket);

int main() {
    int server_socket;
    struct sockaddr_in server_info;
    char receive_buffer[MAX_BUFFER_SIZE];
    int bytes_read;

    // 소켓 생성
    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        perror("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    // 서버 주소 설정
    memset(&server_info, 0, sizeof(server_info));
    server_info.sin_family = AF_INET;
    server_info.sin_port = htons(SERVER_PORT);

    // 서버 IP 주소 변환
    if (inet_pton(AF_INET, SERVER_ADDRESS, &server_info.sin_addr) <= 0) {
        perror("Failed to convert IP address");
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // 서버에 연결 시도
    while (connect(server_socket, (struct sockaddr*)&server_info, sizeof(server_info)) < 0) {
        if (errno == EINTR) {
            continue;  // 시그널로 인한 중단 시 재시도
        } else if (errno == EINPROGRESS || errno == EALREADY) {
            break;  // 연결 진행 중
        } else {
            perror("Failed to connect to server");
            close(server_socket);
            exit(EXIT_FAILURE);
        }
    }

    // 소켓과 표준 입력을 논블로킹 모드로 설정
    set_nonblocking_mode(server_socket);
    set_nonblocking_mode(STDIN_FILENO);

    // 프로그램 헤더 출력
    printf("╔═════════════════════════════════════════╗\n");
    printf("║               VEDA TALK                 ║\n");
    printf("╠═════════════════════════════════════════╣\n");
    printf("║                                         ║\n");
    printf("║         Welcome to VEDA TALK!           ║\n");
    printf("║                                         ║\n");
    printf("║   ┌─────────────────────────────────┐   ║\n");
    printf("║   │ REGISTER <username> <password>  │   ║\n");
    printf("║   └─────────────────────────────────┘   ║\n");
    printf("║                                         ║\n");
    printf("║   ┌─────────────────────────────────┐   ║\n");
    printf("║   │ LOGIN    <username> <password>  │   ║\n");
    printf("║   └─────────────────────────────────┘   ║\n");
    printf("║                                         ║\n");
    printf("║         To logout, type 'LOGOUT'        ║\n");
    printf("║                                         ║\n");
    printf("╚═════════════════════════════════════════╝\n");

    while (1) {
        // 사용자 입력 처리
        handle_user_input(server_socket);

        // 서버로부터 메시지 수신
        bytes_read = read(server_socket, receive_buffer, MAX_BUFFER_SIZE - 1);
        if (bytes_read > 0) {
            receive_buffer[bytes_read] = '\0';
            printf("%s", receive_buffer);  // 서버에서 받은 메시지 출력
            fflush(stdout);
        } else if (bytes_read == 0) {
            printf("Connection to the server has been closed.\n");
            break;
        } else if (bytes_read < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
            perror("Failed to read from server");
            break;
        }

        usleep(10000); // 10ms 지연
    }

    close(server_socket);
    return 0;
}

void handle_user_input(int server_socket) {
    char input_buffer[MAX_BUFFER_SIZE];
    int bytes_read = read(STDIN_FILENO, input_buffer, MAX_BUFFER_SIZE - 1);
    if (bytes_read > 0) {
        input_buffer[bytes_read] = '\0';

        // 로그아웃 명령어 처리
        if (strncmp(input_buffer, "LOGOUT", 6) == 0) {
            printf("Log out!\n");
            printf("Thanks for using VEDA TALK :)\n");
            // 서버에 로그아웃 메시지 전송
            write(server_socket, input_buffer, strlen(input_buffer));
            close(server_socket);
            exit(0); // 클라이언트 프로그램 종료
        }

        // 서버로 메시지 전송
        if (write(server_socket, input_buffer, strlen(input_buffer)) < 0) {
            if (errno != EWOULDBLOCK && errno != EAGAIN) {
                perror("Failed to write to server");
            }
        }
    } else if (bytes_read < 0 && errno != EWOULDBLOCK && errno != EAGAIN) {
        perror("Failed to read from standard input");
    }
}

void set_nonblocking_mode(int socket_fd) {
    int flags = fcntl(socket_fd, F_GETFL, 0);
    if (flags == -1) {
        perror("fcntl(F_GETFL) failed");
        exit(EXIT_FAILURE);
    }
    if (fcntl(socket_fd, F_SETFL, flags | O_NONBLOCK) == -1) {
        perror("fcntl(F_SETFL) failed");
        exit(EXIT_FAILURE);
    }
}

