CC = gcc

all : tcp_chat_server tcp_chat_client

tcp_chat_server: tcp_chat_server.c
	$(CC) -o tcp_chat_server $^

tcp_chat_client: tcp_chat_client.c
	$(CC) -o tcp_chat_client $^

clean:
	rm tcp_chat_server tcp_chat_client
