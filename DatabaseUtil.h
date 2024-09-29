//
// Created by shuta on 24/09/29.
//

#ifndef PORTMANAGER_DATABASEUTIL_H
#define PORTMANAGER_DATABASEUTIL_H

#include <stdlib.h>
#include <mariadb/mysql.h>
#include <stdio.h>

#define host        "127.0.0.1"
#define user        "testar"
#define password    "pass"
#define database    "PORTS"
#define port        3306

struct Response_sql;
MYSQL* getConnection();
struct Response_sql sendCommandHasResponse(MYSQL*, char*);
int sendCommand(MYSQL*, char*);

struct Response_sql{
    char error;
    MYSQL_RES *response;
};

MYSQL* getConnection(){
    MYSQL* conn = NULL;
    if(!(conn = mysql_init(0))) return NULL;
    if(!mysql_real_connect(conn,
                          "127.0.0.1",
                          "tester",
                          "pass",
                          "PORTS",
                          3306,
                          NULL,0))
        return NULL;
    return conn;
}

/*
 * SELECTなどのレスポンスがあるSQLコマンドを送信する場合に使用します。
 * エラーが発生した場合は、構造体のerrorに1が代入されます。
 */
struct Response_sql sendCommandHasResponse(MYSQL* conn, char* command){
    struct Response_sql response_sql;
    if(mysql_query(conn, &command[0])){
        response_sql.error = 1;
        printf("[sendCommandHasResponse] コマンド実行時にエラーが発生しました。%s\n", command);
        return response_sql;
    }
    response_sql.error = 0;
    response_sql.response = mysql_use_result(conn);
    return response_sql;
}

/*
 * INSERTなどのレスポンスがないSQLコマンドを送信する場合に使用します。
 * エラーが発生した場合は、-1を返します。
 */
int sendCommand(MYSQL* conn, char* command){
    if(mysql_query(conn, command)){
        return -1;
    }
    return 0;
}

#endif //PORTMANAGER_DATABASEUTIL_H
