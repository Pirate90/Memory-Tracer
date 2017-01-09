#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <mysql.h>

#define KOREAN_WIDTH 2
#define KOREAN_BYTE 3
#define TITLE_LENGTH 10
#define GENRE_LENGTH 4
#define STORY_LENGTH 50
#define REVIEW_LENGTH 500
#define TITLE_BYTE TITLE_LENGTH * KOREAN_BYTE + 2
#define GENRE_BYTE GENRE_LENGTH * KOREAN_BYTE + 1
#define STORY_BYTE STORY_LENGTH * KOREAN_BYTE + 2
#define REVIEW_BYTE REVIEW_LENGTH * KOREAN_BYTE + 2

#define OP_SEND_REVIEW_TABLE 0
#define OP_SEND_REVIEW 1
#define OP_RECEIVE_INPUT_REVIEW 2
#define OP_RECEIVE_EDIT_REVIEW 3
#define OP_RECEIVE_DEL_REVIEW 4
#define OP_DESC_SORT 5
#define OP_ASC_SORT 6
#define OP_SAME 7
#define DIV 10
#define DIV2 100

typedef struct REVIEW {
	unsigned int id;
	char title[TITLE_BYTE];
	char genre[GENRE_BYTE];
	int story_score,music_score,casting_score;
	char story[STORY_BYTE];
	char review[REVIEW_BYTE];
	char pic[5][100];
	char timer[20];
} REVIEW;

typedef struct REVIEW_TABLE {
	int count;
	struct REVIEW* t;
} REVIEW_TABLE;

typedef struct REQUEST {
	int op_code;
	char arg[80];
} REQUEST;

int count_korean_char(char* s);
int format_width_count(char* s, int width);
void print_reviewtable(REVIEW_TABLE* rt);
void send_REVIEW_TABLE (int connfd, REVIEW_TABLE* rt);
void select_query_with_rt_alloc(MYSQL* connection, char* query, REVIEW_TABLE* rt);
void free_rt(REVIEW_TABLE* rt);



int main(int argc, char* argv[]) {
	REVIEW_TABLE rt_temp;
	REVIEW r_temp;
	int listenfd, connfd;
	int i;
	int count = 0;
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	MYSQL *connection=NULL, conn;
	mysql_init(&conn);

	MYSQL_RES* sql_result = NULL;
	MYSQL_ROW sql_row;
	int query_stat;
	const char ref_table[2][10] = {"movie", "ani"};
	const char order_by_value[7][20] = {"id", "title", "genre", "story_score", "music_score", "casting_score", "timer"};
	const char same_by_value[4][20] = {"genre", "story_score", "music_score", "casting_score"};

	connection = mysql_real_connect(&conn, "183.107.58.140", "pi1", "qwzxas34", "memory_tracer", 3306, (char*)NULL, 0);

	if (connection == NULL){
		fprintf(stderr, "Mysql connection error : %s", mysql_error(&conn));
		return 1;
	}

	if(argc < 1) {
		printf("사용법: %s port \n", argv[0]);
		exit(1);
	}

	listenfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
	servaddr.sin_port = htons(atoi(argv[1]));
	bind(listenfd, (struct sockaddr*)&servaddr, sizeof(servaddr));

	listen(listenfd, 5);

	while(connfd = accept(listenfd, (struct sockaddr*)&cliaddr, &clilen)) {

		printf("유저가 입장했습니다.\n");
		REQUEST req;

		while(1) { //첫화면 - 1
			read(connfd, &req, sizeof(req));
			printf("user request %d\n", req.op_code);
			int op1, op2, op_select;
			char buf[2000];

			op_select = req.op_code / DIV2;
			op1 = (req.op_code % DIV2) / DIV;
			op2 = req.op_code % DIV;
			int id;

			switch(op1) {
				case OP_SEND_REVIEW_TABLE:
				sprintf(buf, "SELECT * FROM %s", ref_table[op_select]);
				select_query_with_rt_alloc(connection, buf, &rt_temp);
				send_REVIEW_TABLE(connfd, &rt_temp);
				free_rt(&rt_temp);
				break;
				case OP_SEND_REVIEW:
				memcpy(&id, req.arg, sizeof(int));
				sprintf(buf, "SELECT * FROM %s WHERE id=%d", ref_table[op_select], id);
				select_query_with_rt_alloc(connection, buf, &rt_temp);
				write(connfd, rt_temp.t, sizeof(REVIEW));
				free_rt(&rt_temp);
				break;
				case OP_RECEIVE_INPUT_REVIEW:
				read(connfd, &r_temp, sizeof(REVIEW));
				sprintf(buf, "INSERT INTO %s (title, genre, story_score, music_score, casting_score, story, review) VALUES (\'%s\', \'%s\', %d, %d, %d, \'%s\', \'%s\')", ref_table[op_select], r_temp.title, r_temp.genre, r_temp.story_score, r_temp.music_score, r_temp.casting_score, r_temp.story, r_temp.review);
				printf("%s\n", buf);

				query_stat = mysql_query(connection, buf);
				if (query_stat != 0) {
					fprintf(stderr, "Mysql query error : %s", mysql_error(connection));
					exit(1);
				}
				break;
				case OP_RECEIVE_EDIT_REVIEW:
				read(connfd, &r_temp, sizeof(REVIEW));
				memcpy(&id, req.arg, sizeof(int));
				sprintf(buf, "UPDATE %s SET title=\'%s\', genre=\'%s\', story_score=%d, music_score=%d, casting_score=%d, story=\'%s\', review=\'%s\' WHERE id=%d", ref_table[op_select], r_temp.title, r_temp.genre, r_temp.story_score, r_temp.music_score, r_temp.casting_score, r_temp.story, r_temp.review, id);
				printf("%s\n", buf);

				query_stat = mysql_query(connection, buf);
				if (query_stat != 0) {
					fprintf(stderr, "Mysql query error : %s", mysql_error(connection));
					exit(1);
				}
				break;
				case OP_RECEIVE_DEL_REVIEW:
				read(connfd, &r_temp, sizeof(REVIEW));
				memcpy(&id, req.arg, sizeof(int));
				sprintf(buf, "DELETE FROM %s WHERE id=%d", ref_table[op_select], id);
				printf("%s\n", buf);
				
				query_stat = mysql_query(connection, buf);
				if (query_stat != 0) {
					fprintf(stderr, "Mysql query error : %s", mysql_error(connection));
					exit(1);
				}
				break;
				case OP_DESC_SORT:
				sprintf(buf, "SELECT * FROM %s ORDER BY %s DESC", ref_table[op_select], order_by_value[op2]);
				select_query_with_rt_alloc(connection, buf, &rt_temp);
				send_REVIEW_TABLE(connfd, &rt_temp);
				free_rt(&rt_temp);
				break;
				case OP_ASC_SORT:
				sprintf(buf, "SELECT * FROM %s ORDER BY %s ASC", ref_table[op_select], order_by_value[op2]);
				select_query_with_rt_alloc(connection, buf, &rt_temp);
				send_REVIEW_TABLE(connfd, &rt_temp);
				free_rt(&rt_temp);
				break;
				case OP_SAME:
				if (op2 == 0) {
					sprintf(buf, "SELECT * FROM %s WHERE %s=%s", ref_table[op_select], same_by_value[op2], req.arg);
				}
				else {
					memcpy(&id, req.arg, sizeof(int));
					sprintf(buf, "SELECT * FROM %s WHERE %s=%d", ref_table[op_select], same_by_value[op2], id);
				}
				select_query_with_rt_alloc(connection, buf, &rt_temp);
				send_REVIEW_TABLE(connfd, &rt_temp);
				free_rt(&rt_temp);
				break;
			}
		}
	}
	return 0;
}

int count_korean_char(char* s) {
	int count = 0;
	int i;
	for(i = 0; i < strlen(s); ++i) {
		if(s[i] > 0xBF)
			++count;
	}
	return count;
}

int format_width_count(char* s, int width) {
	int byte3count = count_korean_char(s);
	return byte3count + width;
}


void print_reviewtable(REVIEW_TABLE* rt) { //감상문테이블을 화면으로 출력하는 함수
	int i;
	char buf[100];
	int count = rt->count;
	REVIEW* a = rt->t;

	printf("\n|%-8s|%-8s|%-22s|%-10s|%-15s|%-14s|%-15s|%s\n","순서","번호","제목","장르","스토리점수","음악점수","캐스팅점수","저장날짜");

	for(i=0; i<count; i++) {
		sprintf(buf, "|%%-6d|%%-6d|%%-%ds|%%-%ds|", format_width_count(a[i].title, KOREAN_WIDTH * TITLE_LENGTH), format_width_count(a[i].genre, KOREAN_WIDTH * GENRE_LENGTH));
		printf(buf, i+1, a[i].id,a[i].title,a[i].genre);
		printf("%d|", a[i].story_score);
		printf("%d|", a[i].music_score);
		printf("%d|", a[i].casting_score);
		printf("%s|", a[i].timer);
	}
	printf("\n");
}

void select_query_with_rt_alloc(MYSQL* connection, char* query, REVIEW_TABLE* rt) {
	MYSQL_ROW sql_row;
	int query_stat = mysql_query(connection, query);
	if (query_stat != 0) {
		fprintf(stderr, "Mysql query error : %s", mysql_error(connection));
		exit(1);
	}

	MYSQL_RES* sql_result = mysql_store_result(connection);

	rt->count = mysql_num_rows(sql_result);
	rt->t = (REVIEW*)malloc(sizeof(REVIEW)*rt->count);

	REVIEW* row = rt->t;
	while ( (sql_row = mysql_fetch_row(sql_result)) != NULL )
	{
		row->id = (unsigned int)atoi(sql_row[0]);
		strcpy(row->title, sql_row[1]);
		strcpy(row->genre, sql_row[2]);
		row->story_score = atoi(sql_row[3]);
		row->music_score = atoi(sql_row[4]);
		row->casting_score = atoi(sql_row[5]);
		strcpy(row->story, sql_row[6]);
		strcpy(row->review, sql_row[7]);
		strcpy(row->timer, sql_row[8]);
		row++;
	}
	print_reviewtable(rt);

	mysql_free_result(sql_result);
}

void free_rt(REVIEW_TABLE* rt) {
	free(rt->t);
}

void send_REVIEW_TABLE (int connfd, REVIEW_TABLE* rt) {
	write(connfd, &(rt->count), sizeof(int));
	write(connfd, rt->t, sizeof(REVIEW)*rt->count);
}

