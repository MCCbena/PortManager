#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include "ResponseHandler.h"

int main(void) {
    int rsock, wsock;
    struct sockaddr_in addr, client;
    int len;
    int ret;

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
        struct Response response = getResponse(response_get);
        handler(response, wsock, rsock);

        /* close TCP session */
        shutdown(wsock, SHUT_RDWR);
        //exit(0);
    }
    shutdown(rsock, SHUT_RDWR);
    return 0;
}
