#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ResponseHandler.h"
#include <signal.h>

int rsock;
void close(int signal){
    printf("例外発生 %d\n", signal);
    if(signal == 2){
        shutdown(rsock, SHUT_RDWR);
        exit(0);
    }
}

int main(int argc, char *argv[]) {

    int wsock;
    struct sockaddr_in addr, client;
    int len;
    int ret;

    for(int i = 0; i < argc; i++){
        printf("%s\n", argv[i]);
    }

    if(argc!=7){
        printf("引数の数が合いません。（必要:%d, 指定された数:%d）\n", 6, argc);
        exit(1);
    }

    host = argv[1];
    user = argv[2];
    password = argv[3];
    db = argv[4];
    port = atoi(argv[5]);
    ROOT = argv[6];

    //interruptのハンドラーを設定
    signal(SIGTERM, close);
    signal(SIGINT, close);

    //ソケットを展開
    rsock = socket(AF_INET, SOCK_STREAM, 0);

    if(rsock < 0){
        printf("Error, Cannot make socket.\n");
        return -1;
    }

    //ソケットの設定
    addr.sin_family      = AF_INET;
    addr.sin_port        = htons(8080);
    addr.sin_addr.s_addr = INADDR_ANY;

    ret = bind(rsock, (struct sockaddr *)&addr, sizeof(addr));

    if (ret < 0){
        printf("Cannot bind socket\n");
        return -1;
    }
    //起動
    listen(rsock, 5);

    //TCP接続を許可
    len = sizeof(client);

    while (1) {
        wsock = accept(rsock, (struct sockaddr *) &client, &len);

        //レスポンスの格納
        char *response_get = calloc(1, 1024);
        recv(wsock, response_get, 1024, 0);
        HashMap hashMap = *newHashMap(1024);
        struct Response response = getResponse(response_get, hashMap);
        handler(response, wsock, rsock);

        free(response_get);
        free(response.body);
        freeHashMap(&hashMap);

        /* close TCP session */
        shutdown(wsock, SHUT_RDWR);
        //exit(0);
    }
}