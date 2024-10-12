//
// Created by shuta on 24/09/28.
//

#ifndef PORTMANAGER_SENDER_H
#define PORTMANAGER_SENDER_H


#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <stdio.h>
#include <signal.h>

struct BodyObject;

void easySender(char*, char**, struct BodyObject, char*, int);
char** makeDataObject(int, int);
int byteSize(const char*);
void destroyBodyObject(struct BodyObject*);
struct BodyObject makeBodyObject(char*, unsigned int);
void errorHandler(int);

struct BodyObject{
    char* content;
    unsigned int size;
};

void destroyBodyObject(struct BodyObject *bodyObject){
    free(bodyObject->content);
}

/*
 * easySenderのbody用の構造体です。
 * content=bodyの内容（NULL許容）
 * size=bodyのバイト数
 */
struct BodyObject makeBodyObject(char* content, unsigned int size){
    struct BodyObject bodyObject;
    if(content!=NULL) {
        bodyObject.content = malloc(size);
        memcpy(bodyObject.content, content, size);
    } else bodyObject.content = NULL;
    bodyObject.size = size;
    return bodyObject;
}

/*
 * easySenderの追加ヘッダを記述するためのString配列を作成します。
 * また、配列の最後を明記するために0が挿入された配列が余分１つ作成されます。
 * n=いくつのヘッダを記入するか
 * size=ヘッダ一つに対するバイト数
 */
char** makeDataObject(int n, int size){
    char** var = calloc(n+1, size);
    for(int i = 0; i < n+1; i++){
        var[i] = calloc(1, size);
    }
    return var;
}

/*
 * レスポンスの送信を簡素化します。
 * method=HTTP/1.1 200 OKなどのメソッド
 * append=追加で送信するヘッダ（末尾に\r\nが挿入され、freeされます。）
 * body=送信する内容（NULL許容でです。）
 * type=Content-Type
 * wsock=送信先
 */
void easySender(char* method, char** append , struct BodyObject bodyObject, char* type, int wsock){
    char* send_method = (char*) malloc(32);
    char* length =      (char*) malloc(32);
    char* content_type =(char*) malloc(128);

    //エラーハンドラーの登録
    signal(SIGPIPE, errorHandler);

    //メソッドの作成
    sprintf(send_method, "%s\r\n", method);
    //Content-Typeの作成
    sprintf(content_type, "Content-Type: %s\r\n", type);

    //Content-Lengthの作成
    if(bodyObject.content!=NULL){
        sprintf(length, "Content-Length: %d\r\n", bodyObject.size);
    }
    //ヘッダ送信
    send(wsock, send_method, byteSize(send_method), 0);
    send(wsock, content_type, byteSize(content_type), 0);
    if(bodyObject.content!=NULL) send(wsock, length, byteSize(length), 0);
    //追加ヘッダを送信
    if(append!=NULL){
        int i = 0;
        do {
            char* concat = malloc(256);
            sprintf(concat, "%s\r\n", append[i]);
            send(wsock, concat, byteSize(concat), 0);
            free(concat);
            i++;
        } while (append[i][0]!=0);
        free(append);
    }
    send(wsock, "\r\n", 2, 0); // ヘッダとボディを区切る空行


    if (bodyObject.content!=NULL)send(wsock, bodyObject.content, bodyObject.size, 0);


    //メモリ開放
    free(send_method);
    free(content_type);
    free(length);
}

int byteSize(const char* content){
    int i = 0;
    if(content==NULL) return 0;
    while (content[i] != 0){
        i++;
    }
    return i;
}

void errorHandler(int signal){
    printf("exception signal: %d\n", signal);
}

#endif //PORTMANAGER_SENDER_H

