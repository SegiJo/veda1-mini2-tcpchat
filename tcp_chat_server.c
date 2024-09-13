#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define PORT 8080
#define MAX_CLIENTS 100
#define MAX_BUFFER 1024
#define USER_DATA_FILE "./users_jsg.txt"  // 절대 경로로 수정

typedef struct {
    int client_socket;
    int logged_in;
    char username[MAX_BUFFER];
    int parent_read_pipe[2]; // 부모가 읽고, 자식이 쓰는 파이프
    int child_read_pipe[2];  // 자식이 읽고, 부모가 쓰는 파이프
} ClientInfo;

// 함수 선언
void set_nonblocking_mode(int socket);
int register_new_user(const char *username, const char *password);
int authenticate_user(const char *username, const char *password);
void process_client_message(ClientInfo *client, const char *message, ClientInfo *clients);
void send_message_to_all(const char *message, int sender_socket, ClientInfo *clients, int max_clients);
void announce_client_status(const char *username, const char *status, ClientInfo *clients, int max_clients);
void handle_client_logout(ClientInfo *client);
void disconnect_client(ClientInfo *client);
void handle_sigchld(int sig);
void daemonize();

ClientInfo clients[MAX_CLIENTS];

int main() {
    int server_socket, client_socket, i;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    pid_t pid;

    // 데몬화
    daemonize();

    // SIGCHLD 핸들러 설정
    struct sigaction sa;
    sa.sa_handler = handle_sigchld;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART | SA_NOCLDSTOP;
    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        exit(EXIT_FAILURE);
    }

    if ((server_socket = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        exit(EXIT_FAILURE);
    }

    set_nonblocking_mode(server_socket);

    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    server_addr.sin_port = htons(PORT);

    if (bind(server_socket, (struct sockaddr*)&server_addr, sizeof(server_addr)) < 0) {
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    if (listen(server_socket, 8) < 0) {
        close(server_socket);
        exit(EXIT_FAILURE);
    }

    // 클라이언트 배열 초기화
    for (i = 0; i < MAX_CLIENTS; i++) {
        clients[i].client_socket = -1;
        clients[i].logged_in = 0;
        memset(clients[i].username, 0, MAX_BUFFER);
        clients[i].parent_read_pipe[0] = -1;
        clients[i].parent_read_pipe[1] = -1;
        clients[i].child_read_pipe[0] = -1;
        clients[i].child_read_pipe[1] = -1;
    }

    while (1) {
        client_socket = accept(server_socket, (struct sockaddr*)&client_addr, &client_addr_len);
        if (client_socket >= 0) {
            set_nonblocking_mode(client_socket);

            for (i = 0; i < MAX_CLIENTS; i++) {
                if (clients[i].client_socket == -1) {
                    clients[i].client_socket = client_socket;

                    // 두 개의 파이프 생성
                    if (pipe(clients[i].parent_read_pipe) < 0 || pipe(clients[i].child_read_pipe) < 0) {
                        close(client_socket);
                        clients[i].client_socket = -1;
                        break;
                    }

                    set_nonblocking_mode(clients[i].parent_read_pipe[0]);
                    set_nonblocking_mode(clients[i].parent_read_pipe[1]);
                    set_nonblocking_mode(clients[i].child_read_pipe[0]);
                    set_nonblocking_mode(clients[i].child_read_pipe[1]);

                    pid = fork();
                    if (pid < 0) {
                        close(client_socket);
                        close(clients[i].parent_read_pipe[0]);
                        close(clients[i].parent_read_pipe[1]);
                        close(clients[i].child_read_pipe[0]);
                        close(clients[i].child_read_pipe[1]);
                        clients[i].client_socket = -1;
                        break;
                    } else if (pid == 0) {
                        // 자식 프로세스
                        close(server_socket); // 부모 소켓 닫기
                        close(clients[i].parent_read_pipe[1]); // 부모 쓰기 끝 닫기
                        close(clients[i].child_read_pipe[0]);  // 자식 읽기 끝 닫기
                        while (1) {
                            char buffer[MAX_BUFFER];
                            int n = read(clients[i].parent_read_pipe[0], buffer, MAX_BUFFER - 1);
                            if (n > 0) {
                                buffer[n] = '\0';

                                if (strncmp(buffer, "LOGOUT", 6) == 0) {
                                    announce_client_status(clients[i].username, "has left the chat", clients, MAX_CLIENTS);
                                    break;
                                }

                                process_client_message(&clients[i], buffer, clients);
                            }
                            usleep(10000); // 10ms 지연
                        }
                        close(clients[i].parent_read_pipe[0]);
                        close(clients[i].child_read_pipe[1]); // 자식 쓰기 끝 닫기
                        exit(0);
                    } else {
                        // 부모 프로세스
                        close(clients[i].parent_read_pipe[0]); // 부모 읽기 끝 닫기
                        close(clients[i].child_read_pipe[1]);  // 자식 쓰기 끝 닫기
                    }

                    break;
                }
            }

            if (i == MAX_CLIENTS) {
                close(client_socket);
            }
        }

        // 부모가 클라이언트로부터 메시지 수신
        for (i = 0; i < MAX_CLIENTS; i++) {
            client_socket = clients[i].client_socket;
            if (client_socket != -1) {
                char buffer[MAX_BUFFER];
                int n = read(client_socket, buffer, MAX_BUFFER - 1);
                if (n > 0) {
                    buffer[n] = '\0';
                    // 클라이언트의 메시지를 자식 프로세스로 전달
                    write(clients[i].parent_read_pipe[1], buffer, n);
                } else if (n == 0 || (n < 0 && errno != EWOULDBLOCK && errno != EAGAIN)) {
                    if (clients[i].logged_in) {
                        announce_client_status(clients[i].username, "has left the chat", clients, MAX_CLIENTS);
                    }
                    disconnect_client(&clients[i]);
                }
            }
        }

        // 자식 프로세스로부터 메시지 수신
        for (i = 0; i < MAX_CLIENTS; i++) {
            if (clients[i].child_read_pipe[0] != -1) {
                char pipe_buffer[MAX_BUFFER];
                int m = read(clients[i].child_read_pipe[0], pipe_buffer, MAX_BUFFER - 1);
                if (m > 0) {
                    pipe_buffer[m] = '\0';
                    send_message_to_all(pipe_buffer, clients[i].client_socket, clients, MAX_CLIENTS);
                }
            }
        }

        usleep(10000); // 10ms 지연
    }

    close(server_socket);
    return 0;
}

// 데몬화 함수
void daemonize() {
    pid_t pid;

    // 부모 프로세스 종료
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // 새로운 세션 리더가 됨
    if (setsid() < 0) {
        exit(EXIT_FAILURE);
    }

    // 시그널 무시
    signal(SIGCHLD, SIG_IGN);
    signal(SIGHUP, SIG_IGN);

    // 두 번째 포크
    pid = fork();
    if (pid < 0) {
        exit(EXIT_FAILURE);
    }
    if (pid > 0) {
        exit(EXIT_SUCCESS);
    }

    // 파일 모드 생성 마스크 설정
    umask(0);

    // 현재 작업 디렉토리 변경하지 않음

    // 표준 파일 디스크립터 닫기
    close(STDIN_FILENO);
    close(STDOUT_FILENO);
    close(STDERR_FILENO);
}

void handle_sigchld(int sig) {
    while (waitpid(-1, NULL, WNOHANG) > 0);
}

void process_client_message(ClientInfo *client, const char *message, ClientInfo *clients) {
    char username[MAX_BUFFER], password[MAX_BUFFER];

    // 명령어 파싱
    if (sscanf(message, "REGISTER %1023s %1023s", username, password) == 2) {
        if (register_new_user(username, password)) {
            write(client->child_read_pipe[1], "Registration successful\n", strlen("Registration successful\n"));
        } else {
            write(client->child_read_pipe[1], "Registration failed\n", strlen("Registration failed\n"));
        }
    } else if (sscanf(message, "LOGIN %1023s %1023s", username, password) == 2) {
        if (authenticate_user(username, password)) {
            client->logged_in = 1;
            strncpy(client->username, username, MAX_BUFFER - 1);
            announce_client_status(client->username, "has joined the chat", clients, MAX_CLIENTS);
        } else {
            write(client->child_read_pipe[1], "Login failed\n", strlen("Login failed\n"));
        }
    } else if (strncmp(message, "LOGOUT", 6) == 0) {
        handle_client_logout(client);
        write(client->child_read_pipe[1], "Logged out successfully\n", strlen("Logged out successfully\n"));
    } else if (client->logged_in) {
        // 로그인 상태에서 일반 메시지 처리
        char full_message[MAX_BUFFER];
        snprintf(full_message, MAX_BUFFER, "%s: %s", client->username, message);
        write(client->child_read_pipe[1], full_message, strlen(full_message));
    } else {
        write(client->child_read_pipe[1], "Please login first\n", strlen("Please login first\n"));
    }
}

void announce_client_status(const char *username, const char *status, ClientInfo *clients, int max_clients) {
    char status_message[MAX_BUFFER];
    snprintf(status_message, MAX_BUFFER, "\033[0;36m%s %s.\033[0m\n", username, status);
    send_message_to_all(status_message, -1, clients, max_clients);
}

void send_message_to_all(const char *message, int sender_socket, ClientInfo *clients, int max_clients) {
    for (int i = 0; i < max_clients; i++) {
        int client_socket = clients[i].client_socket;
        if (client_socket != -1 && client_socket != sender_socket) {
            if (write(client_socket, message, strlen(message)) < 0) {
                // 에러 처리 가능
            }
        }
    }
}

void handle_client_logout(ClientInfo *client) {
    disconnect_client(client);
}

void disconnect_client(ClientInfo *client) {
    close(client->client_socket);
    close(client->parent_read_pipe[1]);
    close(client->child_read_pipe[0]);
    client->client_socket = -1;
    client->logged_in = 0;
    memset(client->username, 0, MAX_BUFFER);
}

int register_new_user(const char *username, const char *password) {
    FILE *file = fopen(USER_DATA_FILE, "a");
    if (!file) {
        return 0;
    }

    fprintf(file, "%s %s\n", username, password);
    fclose(file);
    return 1;
}

int authenticate_user(const char *username, const char *password) {
    FILE *file = fopen(USER_DATA_FILE, "r");
    if (!file) {
        return 0;
    }

    char file_username[MAX_BUFFER], file_password[MAX_BUFFER];
    while (fscanf(file, "%1023s %1023s", file_username, file_password) == 2) {
        if (strcmp(username, file_username) == 0 && strcmp(password, file_password) == 0) {
            fclose(file);
            return 1;
        }
    }

    fclose(file);
    return 0;
}

void set_nonblocking_mode(int socket) {
    int opts = fcntl(socket, F_GETFL);
    if (opts < 0) {
        exit(EXIT_FAILURE);
    }
    opts = (opts | O_NONBLOCK);
    if (fcntl(socket, F_SETFL, opts) < 0) {
        exit(EXIT_FAILURE);
    }
}

