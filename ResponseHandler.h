//
// Created by shuta on 24/09/27.
//

#ifndef PORTMANAGER_RESPONSEHANDLER_H
#define PORTMANAGER_RESPONSEHANDLER_H

#include <string.h>

//メソッドの定義
void methods(int id, char* dest){
    char* method[] = {
            "GET\0\0\0\0\0",
            "HEAD\0\0\0\0",
            "POST\0\0\0\0",
            "OPTIONS\0",
            "PUT\0\0\0\0\0",
            "DELETE\0\0",
            "TRACE\0\0\0",
            "\0\0\0\0\0\0\0\0"
    };

    memcpy(dest, method[id], 8);
}

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include "Map.h"

struct Response getResponse(char*);
int handler(struct Response, int, int);

struct Response{
    char method[8];
    HashMap hashMap;
};

struct Response getResponse(char* response_row){
    struct Response response;
    response.hashMap = *newHashMap(1024);

    char* index_start; //最初の文字が格納されたメモリアドレス
    char* index_end; //最後が格納されたメモリアドレス
    //パスとメソッドの格納
    int i = 0;
    while (1){
        char method[8];
        methods(i, method);
        index_start = strstr(response_row, method);
        if(index_start != NULL || method[0] == 0){
            memcpy(response.method, method, 8);
            index_start += strlen(method)+1; //絶対にNULLにはならない
            break;
        }
        i++;
    }
    index_end = strstr(response_row, " HTTP");
    char* path = calloc(1, 1024);
    memcpy(
            path,
            &response_row[strlen(response.method)+1], //リクエストメソッドを除いた、パスから最初に始まる文字列を取得
            index_end-index_start //メモリアドレスを計算してパスのみを取得
            );
    insertToHashMap(&response.hashMap, "path", path);

    index_end = strstr(index_end, "\r\n");
    while (strncmp(index_end, "\r\n\r\n", 4) != 0){
        char *key = calloc(1024, 1);
        char* value = calloc(1024, 1);

        //keyを保存
        index_start = index_end+2;
        index_end = strstr(index_end, ": ");
        memcpy(key, index_start, index_end-index_start);

        //valueを保存
        index_start = index_end+2;
        index_end = strstr(index_end, "\r\n");
        memcpy(value, index_start, index_end-index_start);

        insertToHashMap(&response.hashMap, key, value);

        free(key);
        free(value);
    }
    free(path);
    return response;
}

int handler(struct Response response, int wsock, int rsock){
    //リダイレクトの設定

    if(strcmp(getValueFromHashMap(&response.hashMap, "path"), "/") == 0){
        send(wsock, "HTTP/1.1 302 Found\r\n", 20, 0);
        send(wsock, "Location: /index\r\n", 18, 0);
        send(wsock, "Content-Type: text/html\r\n", 25, 0);
        send(wsock, "Connection: close\r\n", 19, 0);
        send(wsock, "\r\n", 2, 0); // ヘッダとボディの区切り
    }

    if(strcmp(getValueFromHashMap(&response.hashMap, "path"), "/index") == 0){
        // レスポンスヘッダの送信
        send(wsock, "HTTP/1.1 200 OK\r\n", 17, 0);
        send(wsock, "Content-Type: text/html\r\n", 25, 0);
        send(wsock, "Content-Length: 13\r\n", 20, 0);
        send(wsock, "\r\n", 2, 0); // ヘッダとボディを区切る空行
        // レスポンスボディの送信
        send(wsock, "Hello, World!\r\n", 15, 0);
    }
    printf("%s\n", getValueFromHashMap(&response.hashMap, "path"));

    destroyHashMap(&response.hashMap);
    return 0;
}

#endif //PORTMANAGER_RESPONSEHANDLER_H
