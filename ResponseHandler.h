//
// Created by shuta on 24/09/27.
//

#ifndef PORTMANAGER_RESPONSEHANDLER_H
#define PORTMANAGER_RESPONSEHANDLER_H

#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <dirent.h>
#include <json-c/json.h>
#include <mariadb/mysql.h>
#include "Map.h"
#include "Sender.h"
#include "DatabaseUtil.h"

char* ROOT;
char* host;
char* user;
char* password;
char* db;
int port;

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

struct Response getResponse(char*, HashMap);
int rule(struct Response, int, int);
int handler(struct Response, int, int);
struct BodyObject rootFileReader(char*);
char* detectionContentType(char*);


struct Response{
    char method[8];
    HashMap header;
    char* body;
};

struct Response getResponse(char* response_row, HashMap hashMap){
    struct Response response;
    response.header = hashMap;

    char* index_start; //最初の文字が格納されたメモリアドレス
    char* index_end; //最後が格納されたメモリアドレス
    //パスとメソッドの格納
    int i = 0;
    while (1){
        char method[8];
        methods(i, method);
        index_start = strstr(response_row, method);
        if(index_start != NULL && method[0] != 0){
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
    insertToHashMap(&response.header, "path", path);

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

        insertToHashMap(&response.header, key, value);
        free(key);
        free(value);
    }
    char* content_length_str = getValueFromHashMap(&response.header, "Content-Length");
    if(content_length_str!=NULL) {
        int content_length = atoi(content_length_str);
        response.body = calloc(content_length + 1, 1);
        memcpy(response.body, index_end + 4, content_length);
    } else response.body=NULL;
    //メモリ開放
    free(path);

    return response;
}

/*
 * 注1:パスの最大長は絶対パス含め1024バイトに制限されます。
 * 注2:ファイルが開けない場合、NULLを返します。ディレクトリを開こうとした場合は、size=1のNULLを返します。
 */
struct BodyObject rootFileReader(char* path){
    //変数宣言
    FILE *fp;
    char* full_path = malloc(1024);
    char* file_data;
    unsigned int size;

    //絶対パス化
    sprintf(full_path, "%s%s", ROOT, path);

    //ディレクトリである場合
    DIR *dir = opendir(full_path);
    if(dir){
        return makeBodyObject(NULL, 1);
    }

    fp = fopen(full_path, "r");
    free(full_path);//メモリ開放
    if(fp==NULL){//ファイルが存在しなかった場合の処理
        return makeBodyObject(NULL, 0);
    }
    //ファイルのサイズ取得
    fseek(fp, 0, SEEK_END);
    fgetpos(fp, (fpos_t *) &size);
    fseek(fp, 0, SEEK_SET);//ファイルのサイズの取得が完了したら、ポインタをもとに戻す
    //ファイル読み込み
    file_data = calloc(1, size+1);
    fread(file_data, 1, size, fp);
    //ファイルクローズ
    fclose(fp);
    //構造体を作成
    struct BodyObject bodyObject = makeBodyObject(file_data, size);
    free(file_data);
    return bodyObject;
}

//Content-Typeを自動的に検出します。検出できない場合は、信頼できない値を返します。
char* detectionContentType(char* path){
    char* file_name = strdup(path);
    char* extension;

    //ファイル名を特定
    while (1){
        char* temp = strstr(file_name+1, "/");
        if(temp == NULL){
            break;
        }
        file_name = strdup(temp);
        //メモリ開放
    }
    //拡張子を特定
    extension = strstr(file_name+1, ".");

    if(extension==NULL){
        printf("\x1b[36m[WARN]detectionContentTypeの回答は保証できません（拡張子が見つかりませんでした）。\x1b[39m\n");
        return "text/plain";
    } else extension++;
    if(strcmp(extension, "html") == 0){
        return "text/html";
    }
    if(strcmp(extension, "css") == 0){
        return "text/css";
    }
    if(strcmp(extension, "js") == 0){
        return "text/javascript";
    }
    if(strcmp(extension, "png") == 0){
        return "image/png";
    }

    //該当しない場合
    printf("\x1b[36m[WARN]detectionContentTypeの回答は保証できません（Content-Typeを特定できませんでした-%s）。\x1b[39m\n", extension);

    free(file_name);
    char* concat = strcat("text/", extension);
    free(extension);
    return concat;
}

int rule(struct Response response, int wsock, int rsock){
    printf("%s\n", getValueFromHashMap(&response.header, "path"));
    //リダイレクトの設定
    if(strcmp(getValueFromHashMap(&response.header, "path"), "/") == 0){
        char** append_headers = makeDataObject(2, 32);
        append_headers[0] = "Location: /index";
        append_headers[1] = "Connection: close";
        struct BodyObject bodyObject = makeBodyObject(NULL, 0);//bodyはNULL
        easySender("HTTP/1.1 302 Found", append_headers, bodyObject, "text/plain", wsock);
        destroyBodyObject(&bodyObject);
        return 1;
    }

    if(strcmp(getValueFromHashMap(&response.header, "path"), "/index") == 0){
        // レスポンスの送信
        struct BodyObject html = rootFileReader("/index.html");
        easySender("HTTP/1.1 200 OK", NULL, html, "text/html", wsock);
        destroyBodyObject(&html);
        return 1;
    }

    if(strcmp(getValueFromHashMap(&response.header, "path"), "/ports") == 0){
        json_object *jsonObject = json_tokener_parse(response.body);
        json_object_object_foreach(jsonObject, key, val){
            if(strcmp(key, "serverID")==0){
                const char* table = json_object_get_string(val);
                //SQLとの接続を確立
                MYSQL *conn = getConnection(host, user, password, db, port);
                //コマンドを作成・送信する
                char* command = calloc(256, 1);
                sprintf(command, "SELECT * FROM %s;", table);
                struct Response_sql responseSql = sendCommandHasResponse(conn, command);
                free(command);
                //レスポンスの受信
                if(responseSql.error == 0){
                    json_object *main = json_object_new_array();
                    MYSQL_ROW row;
                    while ((row = mysql_fetch_row(responseSql.response)) != NULL){
                        json_object *data = json_object_new_object();

                        json_object_object_add(data, "name", json_object_new_string(row[0]));
                        json_object_object_add(data, "ipaddress", json_object_new_string(row[1]));
                        json_object_object_add(data, "port", json_object_new_int(atoi(row[2])));
                        json_object_object_add(data, "protocol", json_object_new_string(row[3]));

                        json_object_array_add(main, data);

                    }
                    const char* json_string = json_object_to_json_string(main);
                    struct BodyObject bodyObject = makeBodyObject((char*)json_string, byteSize(json_string));
                    easySender("HTTP/1.1 200 OK", NULL, bodyObject, "application/json;charset=UTF-8", wsock);
                    json_object_put(main);
                    destroyBodyObject(&bodyObject);
                    mysql_free_result(responseSql.response);
                }else{
                    struct BodyObject bodyObject = makeBodyObject(NULL, 0);
                    easySender("HTTP/1.1 500 Bad Gateway", NULL, bodyObject, "application/json;charset=UTF-8", wsock);
                    destroyBodyObject(&bodyObject);
                }
                mysql_close(conn);
            }
        }
        json_object_put(jsonObject);
        return 1;
    }
    if(strcmp(getValueFromHashMap(&response.header, "path"), "/applyPorts") == 0){
        printf("%s\n", response.body);
        /*
         * 0=エラーなし
         * 1=SQLコマンドの実行エラー
         * 2=名前またはポートの重複があった
         */
        char error = 0;

        json_object *json_server;
        json_object *json_name;
        json_object *json_ipaddress;
        json_object *json_select_port;
        json_object *json_protocol;
        json_object *json_change;

        const char *server;
        const char *name;
        const char *ipaddress;
        int         select_port;
        const char *protocol;
        json_bool   change;

        MYSQL *conn = getConnection(host, user, password, db, port);
        char *command1 = calloc(256, 1);
        char *command2 = calloc(256, 1);

        json_object *jsonObject = json_tokener_parse(response.body);

        json_object_object_get_ex(jsonObject, "server",     &json_server);
        json_object_object_get_ex(jsonObject, "name",       &json_name);
        json_object_object_get_ex(jsonObject, "ipaddress",  &json_ipaddress);
        json_object_object_get_ex(jsonObject, "port",       &json_select_port);
        json_object_object_get_ex(jsonObject, "protocol",   &json_protocol);
        json_object_object_get_ex(jsonObject, "change",     &json_change);

        server =        json_object_get_string(json_server);
        name =          json_object_get_string(json_name);
        ipaddress =     json_object_get_string(json_ipaddress);
        select_port =   json_object_get_int(json_select_port);
        protocol =      json_object_get_string(json_protocol);
        change =        json_object_get_boolean(json_change);

        //新規か編集かでコマンドを変える
        if(change) {
            sprintf(command1,
                    "SELECT * FROM %s WHERE NAME!=\"%s\" AND PORT=%d AND PROTOCOL=\"%s\";",
                    server, name, select_port, protocol);
            sprintf(command2, //ポートの編集だった場合、UPDATEコマンドを実行
                    "UPDATE %s SET IPADDRESS=\"%s\", PORT=%d, PROTOCOL=\"%s\" WHERE NAME = \"%s\";",
                    server, ipaddress, select_port, protocol, name);
        }else{
            sprintf(command1,
                    "SELECT * FROM %s WHERE NAME=\"%s\" OR (PORT=%d AND PROTOCOL=\"%s\");",
                    server, name, select_port, protocol);
            sprintf(command2, //ポートの新規作成だった場合、INSERTを実行
                    "INSERT INTO %s VALUES(\"%s\", \"%s\", %d, \"%s\");",
                    server, name, ipaddress, select_port, protocol);
        }

        //SELECTで重複チェック
        struct Response_sql responseSql = sendCommandHasResponse(conn, command1);
        if(responseSql.error != 1) {
            //SELECTコマンドの実行が正常に完了した場合
            if (mysql_fetch_row(responseSql.response) == NULL) {
                //重複がない場合
                if(sendCommand(conn, command2)==-1) error = 1;
            } else error = 2;
            mysql_close(conn);
        } else error = 1;

        //実行結果の返信
        char* msg = calloc(2, 1);
        msg[0] = (char)(error+48);//0はASCIIで48番であるため、エラー内容+48で数字の文字コードが割り出せる。
        struct BodyObject bodyObject = makeBodyObject(msg, 1);
        easySender("HTTP/1.1 200 OK", NULL, bodyObject, "text/plain", wsock);

        //メモリ開放
        destroyBodyObject(&bodyObject);
        free(command1);
        free(command2);
        free(msg);
        json_object_put(jsonObject);
        mysql_free_result(responseSql.response);

        return 1;
    }
    if(strcmp(getValueFromHashMap(&response.header, "path"), "/deletePorts") == 0){
        /*
         * 0=エラーなし
         * 1=SQL実行時にエラー発生
         */
        int error = 0;

        json_object *jsonObject = json_tokener_parse(response.body);

        json_object *json_server;
        json_object *json_name;

        const char* server;
        const char* name;

        json_object_object_get_ex(jsonObject, "server", &json_server);
        json_object_object_get_ex(jsonObject, "name",   &json_name);

        char* command = calloc(256, 1);
        MYSQL *conn = getConnection(host, user, password, db, port);

        server  = json_object_get_string(json_server);
        name    = json_object_get_string(json_name);

        //コマンドの生成
        sprintf(command,
                "DELETE FROM %s WHERE NAME = \"%s\";",
                server, name);

        //SQLの実行とエラー処理
        if(sendCommand(conn, command)==-1) error = 1;
        else mysql_close(conn);

        char* method = calloc(64, 1);
        if(!error) memcpy(method, "HTTP/1.1 200 GET", 16);
        else memcpy(method, "HTTP/1.1 500 Internal Server Error", 34);

        //レスポンス作成・送信
        struct BodyObject bodyObject = makeBodyObject(NULL, 0);
        easySender(method, NULL, bodyObject, method, wsock);

        //メモリ開放
        free(command);
        destroyBodyObject(&bodyObject);
        json_object_put(jsonObject);
    }

    //ルールがない場合、自動的にファイルから取得
    struct BodyObject html = rootFileReader(getValueFromHashMap(&response.header, "path"));
    if(html.content != NULL) {
        char* content_type = detectionContentType(getValueFromHashMap(&response.header, "path"));
        easySender("HTTP/1.1 200 OK", NULL, html, content_type, wsock);

        destroyBodyObject(&html);
        //free(content_type);

        return 200;
    }else {
        //ファイルが存在しない場合、エラー内容を選定
        if(html.size == 1){
            destroyBodyObject(&html);
            return 403;
        }
        destroyBodyObject(&html);
        return 404;
    }
}

int handler(struct Response response, int wsock, int rsock){

    char* text = malloc(64);
    char* method = malloc(32);
    switch (rule(response, wsock, rsock)) {
        case 404:
            memcpy(method, "HTTP/1.1 404 NOT FOUND\0", 23);
            memcpy(text, "404 not found\0", 14);
            break;
        case 403:
            memcpy(method, "HTTP/1.1 403 FORBIDDEN\0", 23);
            memcpy(text, "403 forbidden\0", 14);
            break;
    }
    if(text != NULL) {
        struct BodyObject bodyObject;
        bodyObject = makeBodyObject(text, byteSize(text));
        easySender(method, NULL, bodyObject, "text/plain", wsock);

        //メモリ開放
        destroyBodyObject(&bodyObject);
    }
    free(text);
    free(method);

    return 0;
}

#endif //PORTMANAGER_RESPONSEHANDLER_H
