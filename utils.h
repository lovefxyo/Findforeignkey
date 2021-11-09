//
// Created by Pygmalion on 2021/11/8.
//

#ifndef DBUTILS_UTILS_H
#define DBUTILS_UTILS_H


#include<iostream>
#include"mysql.h"
#include<string>
#include<list>
#include<vector>
#include <iomanip>
#include <map>
using namespace std;
class utils {
private:
    const char *host;
    const char *user;
    const char *passwd;
    const char *dbname;
    int port;
public:
    MYSQL mysql_conn;
    MYSQL_RES *result;
    //查询的数据库名，目前是类中指定，可以后期更改
    string QUERY_SCHEMA;
    float threshold=0.6;
    //用到的结构体
    //外键结构体
    struct foreign_key{
        string btable_name;
        string bcolumn_name;
        string constraint_name;
        string acolumn_name;
        string atable_name;
    };
    //表中字段结构体
    struct table_info{
        string column_name;
        string data_type;
    };

    utils(const char *host, const char *user, const char *passwd, const char *dbname, char port){
        this->host=host;
        this->user=user;
        this->passwd=passwd;
        this->dbname=dbname;
        this->port=port;
        QUERY_SCHEMA=dbname;
    }
    void init_mysql();
    void close_mysql();
    list<foreign_key> query_foreignkey(string TABLE_NAME);
    list<string> query_tablename(string database_name);
    list<table_info> query_tableinfo(string table_name);
    vector<vector<string>> query_rowtabledata(string table_name);
    map<string,vector<string>> query_coltabledata(string table_name);
    map<string ,vector<string>> find_relation(string table_name);
};

void utils::init_mysql() {
    mysql_init(&mysql_conn);   //初始化MYSQL变量
    if (mysql_real_connect(&mysql_conn, host, user, passwd, dbname, 3306, 0, 0))  //连接到mysql 账号 密码 数据库名 端口
    {
        cout << "----"<<host<<"地址的MySQL已连接,数据库是" <<dbname<<"----"<< endl;
    }
    else
    {
        cout << "----MySQL连接失败----" << endl;
    }
}
void utils::close_mysql() {
    mysql_close(&mysql_conn);
    mysql_free_result(result);
    cout<<"dsad";
    cout << "----MySQL断开连接----" << endl;
    cout<<"22";
}

/**
 * 查询表中外键的名字
 * 类给定了数据库名字(若不符合要求可以修改）
 * Input:表名
 * Output:list<foreign_key> 外键结构体的一个list，包含外键名，引用的表，和字段名
 * eg:
    ----外键共有：2个----
    -------------------------------------
    外键名为：ccid
    查询表的表名:cfoucuss
    查询表中外键列名：cid
    外键对应表名:customerinfo
    对应表中外键列名：cid
    -------------------------------------
    外键名为：csid
    查询表的表名:cfoucuss
    查询表中外键列名：sid
    外键对应表名:storeinfo
    对应表中外键列名：sid
    -------------------------------------
 */
list<utils::foreign_key> utils::query_foreignkey(string table_name) {
    //建立连接
    init_mysql();
    MYSQL_FIELD *field = NULL;
    MYSQL_ROW row;
    string sql =
            "select REFERENCED_TABLE_NAME,CONSTRAINT_NAME,REFERENCED_COLUMN_NAME,COLUMN_NAME from information_schema.key_column_usage t where t.constraint_schema='" +
            QUERY_SCHEMA +
            "' and t.constraint_name in (SELECT CONSTRAINT_NAME FROM information_schema.TABLE_CONSTRAINTS where CONSTRAINT_SCHEMA='" +
            QUERY_SCHEMA + "' and CONSTRAINT_TYPE='FOREIGN KEY' and TABLE_NAME='" + table_name + "')";
    if (!mysql_query(&mysql_conn, sql.c_str()))  //若查询成功返回0，失败返回随机数
    {
        cout << "----查询外键成功----" << endl;
    }
    result = mysql_store_result(&mysql_conn);    //将查询到的结果集储存到result中
    list<foreign_key> foreign_list;
    cout<<QUERY_SCHEMA;
    while ((row = mysql_fetch_row(result))) {
        foreign_key foreignKey1;
        foreignKey1.bcolumn_name = row[2];
        foreignKey1.constraint_name = row[1];
        foreignKey1.btable_name = row[0];
        foreignKey1.acolumn_name=row[3];
        foreignKey1.atable_name=table_name;
        foreign_list.push_back(foreignKey1);
        cout<<"11";
    }
    close_mysql();
    return foreign_list;
}
/**
 * 查询数据库中每个表的名字
 * Input:数据库名
 * Output：list<string> 表名的一个list
 * eg:
    ----usedbookstore共有11个表----
    1:purchase
    2:storeinfo
    3:cfoucuss
    4:accounts
    5:store
    6:book_amount
    7:shopping_cart
    8:customerinfo
    9:books
    10:customeradr
    11:admininfo
 */
list<string> utils::query_tablename(string database_name) {
    init_mysql();
    MYSQL_FIELD *field = NULL;
    MYSQL_ROW row;
    string sql="SELECT TABLE_NAME FROM information_schema.`TABLES` WHERE TABLE_SCHEMA='"+database_name+"'";
    if (!mysql_query(&mysql_conn, sql.c_str()))  //若查询成功返回0，失败返回随机数
    {
        cout << "----查询表名成功----" << endl;
    }
    result = mysql_store_result(&mysql_conn);    //将查询到的结果集储存到result中
    list<string> schema_list;
    while ((row = mysql_fetch_row(result))) {
        schema_list.push_back(row[0]);
    }
    close_mysql();
    return schema_list;
}
/**
 * 查询表中信息
 * Input:查询表的名字
 * Output:输出结构体table_info,包含字段名和数据类型
 * eg:
 * ----usedbookstore共有5个字段----
    -------------------------------------
    表中字段为为：tel--数据类型为:varchar
    -------------------------------------
    表中字段为为：username--数据类型为:varchar
    -------------------------------------
    表中字段为为：userpwd--数据类型为:varchar
    -------------------------------------
    表中字段为为：email--数据类型为:varchar
    -------------------------------------
    表中字段为为：role--数据类型为:enum
    -------------------------------------
 */
list<utils::table_info> utils::query_tableinfo(string table_name) {
    init_mysql();
    MYSQL_FIELD *field = NULL;
    MYSQL_ROW row;
    string sql="SELECT COLUMN_NAME,DATA_TYPE FROM information_schema.`COLUMNS` WHERE TABLE_SCHEMA='"+QUERY_SCHEMA+"' and TABLE_NAME='"+table_name+"'";
    if (!mysql_query(&mysql_conn, sql.c_str()))  //若查询成功返回0，失败返回随机数
    {
        cout << "----查询表信息成功----" << endl;
    }
    result = mysql_store_result(&mysql_conn);    //将查询到的结果集储存到result中
    list<table_info> tableinfo_list;
    while ((row = mysql_fetch_row(result))) {
        table_info tableInfo;
        tableInfo.column_name=row[0];
        tableInfo.data_type=row[1];
        tableinfo_list.push_back(tableInfo);
    }
    close_mysql();
    return tableinfo_list;
}
/***
 * 查询表中字段，并按照查询table_info中顺序输出
 * Input:查询表的名字
 * Output:vector<vector<string>>，输出table_info顺序的字段信息
 *eg:
 *  ----查询的是cid,cname,csex,birth,motto,cmoney,tel,结果如下:
    20004 王建新 男 1997-02-15 好好学习. 45.000 18239165907
    20008 "" "" null "" 4000.000 99998888777
    20009 李澳 男 1999-12-14 学习使我快乐 450.000 15824827198
    20010 李淑婉 女 1999-01-01 不吃饭则饥，不读书则愚。 100.000 13509230000
    20012 李咏德 男 1999-01-01 不能则学，不知则问，耻于问人，决无长进。 100.000 13509230001
    20013 李红英 女 1999-01-01 不听指点，多绕弯弯。不懂装懂，永世饭桶。 100.000 13509230002
    20014 李新荣 男 1999-01-01 不问的人永远和愚昧在一起。 100.000 13509230003
 */
vector<vector<string>> utils::query_rowtabledata(string table_name) {
    list<table_info> tableinfo_list=query_tableinfo(table_name);
    string columns;
    int num=tableinfo_list.size();
    list<table_info>::iterator it;
    for(it=tableinfo_list.begin();it!=tableinfo_list.end();it++){
        columns.append(it->column_name);
        columns.append(",");
    }
    string sql="SELECT "+columns.substr(0,columns.size()-1)+" FROM "+table_name;
    cout<<sql;
    init_mysql();
    MYSQL_FIELD *field = NULL;
    MYSQL_ROW row;
    if (!mysql_query(&mysql_conn, sql.c_str()))  //若查询成功返回0，失败返回随机数
    {
        cout << "----查询行表字段成功----" << endl;
        cout<<"----查询的是"+columns+"结果如下:"<<endl;
    }else
        cout<<"查询失败";
    result = mysql_store_result(&mysql_conn);//将查询到的结果集储存到result中
    int num_fields = mysql_num_fields(result);
    vector<vector<string>> tabledata_list;
    while ((row = mysql_fetch_row(result))) {
        vector<string> table_data;
        for(int i=0;i<num_fields;i++){
            if(row[i]==NULL)
                table_data.push_back("null");
            else if(!strcmp(row[i]," "))
                table_data.push_back("\"\"");
            else
                table_data.push_back(row[i]);
        }
        tabledata_list.push_back(table_data);
    }
    close_mysql();
    return tabledata_list;
}
/**
 *返回一个map<string,vector<string>>,key是字段属性名，vector是字段值
 */
map<string, vector<string>> utils::query_coltabledata(string table_name) {
    list<table_info> tableinfo_list=query_tableinfo(table_name);
    init_mysql();
    list<table_info>::iterator it;
    map<string,vector<string>> table_map;
    MYSQL_ROW row;
    for(it=tableinfo_list.begin();it!=tableinfo_list.end();it++){
        string sql="SELECT "+it->column_name+" FROM "+table_name;
        if (!mysql_query(&mysql_conn, sql.c_str()))  //若查询成功返回0，失败返回随机数
        {
            cout << "----查询行表字段成功----" << endl;
        }else
            cout<<"查询失败";
        result = mysql_store_result(&mysql_conn);//将查询到的结果集储存到result中
        vector<string> coltable_data;
        while(row = mysql_fetch_row(result)){
            if(row[0]==NULL)
                coltable_data.push_back("null");
            else if(!strcmp(row[0]," "))
                coltable_data.push_back("\"\"");
            else
                coltable_data.push_back(row[0]);
        }
        cout << "插入:"<<it->column_name<<"数据共："<<coltable_data.size() << endl;
        table_map.insert(pair<string,vector<string>>(it->column_name,coltable_data));
    }
    close_mysql();
    return table_map;
}
/**
 *找属性间的相似度，并且分组
 * 返回map<string,vector<string>>,key是group1、2，vector是分组的属性名
 */
map<string, vector<string>> utils::find_relation(string table_name) {
    cout<<"1";
    map<string ,vector<string>> table_coldata=query_coltabledata(table_name);
    cout<<"2giao"<<endl;
    map<string,vector<string>>::iterator  it1;
    map<string,vector<string>>::iterator  it2;
    int row_num=table_coldata.begin()->second.size();
    vector<string> colname;
    int flag=0;
    int i=0,j=1;
    for(it1=table_coldata.begin();it1!=table_coldata.end();it1++){
        colname.push_back(it1->first);
    }
    int col_num=colname.size();
    int vis[col_num];
    memset(vis,0,sizeof(vis));
    //横向移动加纵向比较
    for(it1=table_coldata.begin();it1!=table_coldata.end();it1++){
        //跳过i=end的情况
        if(it1==table_coldata.end()){
            break;
        }
        for(it2=it1;it2!=table_coldata.end();it2++){
            if(it2==it1){
                //跳过i=j的情况
                continue;
            }
            //计算相似度
            int equal=0;
            int tol=row_num;
            for(int t=0;t<row_num;t++){
    //记录分组信息
                  if(it1->second[t].compare(it2->second[t])==0) {
                      equal++;
                  }
            }
            /***
             * 分组算法：
             * vis[i]初始全部为0
             * 如果i和j属性相似，且vis[i],vis[j]都为0，则让flag++，并且让vis[i]=vis[j]=flag。
             * 如果i和j属性相似，且vis[i]!=vis[j]，则让为0的等于不为0的。
             * 每一个flag是一个分组，最后vis[]中，相等的是一组，为0的单独分组
             */
            if((double)equal/tol>=threshold){
                if(vis[i]==0&&vis[j]==0){
                    flag++;
                    vis[i]=flag;
                    vis[j]=flag;
                }else if(vis[i]!=0&&vis[j]==0){
                    vis[j]=vis[i];
                }else if(vis[i]==0&&vis[j]!=0){
                    vis[i]=vis[j];
                }
            }
            j++;
        }
        i++;
        j=i+1;
    }
    map<string,vector<string>> group_map;
    int t=1;
    //通过vis值实现分组
    for(int i=1;i<=flag;i++){
        vector<string> group;
        for(int j=0;j<col_num;j++){
            if(vis[j]==i) group.push_back(colname[j]);
        }
        string key_string="group";
        key_string.append(to_string(t));
        cout<<key_string;
        group_map.insert(pair<string,vector<string>>(key_string, group));
        t++;
    }
    for(int i=0;i<col_num;i++){
        if(vis[i]==0){
            vector<string> group;
            string key_string="group";
            key_string.append(to_string(t));
            cout<<key_string;
            cout<<i<<colname[i]<<endl;
            group.push_back(colname[i]);
            group_map.insert(pair<string,vector<string>>(key_string, group));
            t++;
        }
    }
    //map<int,vector<string>> group_map;
    return group_map;


}

#endif //DBUTILS_UTILS_H
