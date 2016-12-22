#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

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
#define OP_SEND_LAST_ID 1
#define OP_SEND_REVIEW 2
#define OP_RECEIVE_INPUT_REVIEW 3
#define OP_RECEIVE_EDIT_REVIEW 4
#define OP_RECEIVE_DEL_REVIEW 5
#define OP_DESC_SORT 6
#define OP_ASC_SORT 7
#define OP_SAME 8
#define DIV 10
#define DIV2 100

typedef struct REVIEW {
	int id_number;
	char title[TITLE_BYTE];
	char genre[GENRE_BYTE];
	int story_score,music_score,casting_score;
	char story[STORY_BYTE];
	char review[REVIEW_BYTE];
	char pic[5][100];
	time_t timer;
} REVIEW;

typedef struct REVIEW_TABLE {
	int count;
	struct REVIEW* t;
} REVIEW_TABLE;

typedef struct DB_REVIEW_TABLE {
	int last_id;
	struct REVIEW_TABLE rt;
} DB_REVIEW_TABLE;

typedef struct REVIEW_REF_TABLE {
	int count;
	struct REVIEW** pt;
} REVIEW_REF_TABLE;

typedef struct REQUEST {
	int op_code;
	char arg[80];
} REQUEST;

int sort_by_id_number_desc(const void*, const void*);
int sort_by_title_desc(const void*, const void*);
int sort_by_genre_desc(const void*, const void*);
int sort_by_story_score_desc(const void*, const void*);
int sort_by_music_score_desc(const void*, const void*);
int sort_by_casting_score_desc(const void*, const void*);
int sort_by_timer_desc(const void*, const void*);
int sort_by_id_number_asc(const void*, const void*);
int sort_by_title_asc(const void*, const void*);
int sort_by_genre_asc(const void*, const void*);
int sort_by_story_score_asc(const void*, const void*);
int sort_by_music_score_asc(const void*, const void*);
int sort_by_casting_score_asc(const void*, const void*);
int sort_by_timer_asc(const void*, const void*);
void select_review(REVIEW_REF_TABLE* rrt, DB_REVIEW_TABLE* db_rt, int (*fp)(const REVIEW*, const char*), const char* user_input);
int find_review_by_id(DB_REVIEW_TABLE* db_rt, char* user_input);
int same_by_id(REVIEW* a, char* user_input);
int same_by_genre(const REVIEW* a, const char* user_genre);
int same_by_story_score(const REVIEW* a, const char* user_genre);
int same_by_music_score(const REVIEW* a, const char* user_genre);
int same_by_casting_score(const REVIEW* a, const char* user_genre);
int count_korean_char(char* s);
int format_width_count(char* s, int width);
void write_to_file(char* file_location, DB_REVIEW_TABLE* m, DB_REVIEW_TABLE* a);
void memory_reallocation(DB_REVIEW_TABLE* db_rt);
void init_db_reviewtable (DB_REVIEW_TABLE* db_rt);
void send_REVIEW_TABLE (int connfd, REVIEW_REF_TABLE* rrt);
void ref_table_sync (REVIEW_REF_TABLE* rrt, DB_REVIEW_TABLE* db_rt);

int main(int argc, char* argv[]) {
	DB_REVIEW_TABLE m, a;
	DB_REVIEW_TABLE* sel;
	REVIEW_REF_TABLE ref;
	REVIEW temp;
	int listenfd, connfd;
	FILE *fp;
	int i;
	int count = 0;
	char file_location[80];
	socklen_t clilen;
	struct sockaddr_in cliaddr, servaddr;

	int (*desc_sort_array[7])(const void*, const void*) = {sort_by_id_number_desc, sort_by_title_desc, sort_by_genre_desc, sort_by_story_score_desc, sort_by_music_score_desc, sort_by_casting_score_desc, sort_by_timer_desc};
	int (*asc_sort_array[7])(const void*, const void*) = {sort_by_id_number_asc, sort_by_title_asc, sort_by_genre_asc, sort_by_story_score_asc, sort_by_music_score_asc, sort_by_casting_score_asc, sort_by_timer_asc};
	int (*same_method_array[4])(const REVIEW*, const char*) = {same_by_genre, same_by_story_score, same_by_music_score, same_by_casting_score};

	if(argc < 2) {
		printf("사용법: %s port [file_name]\n", argv[0]);
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

		init_db_reviewtable (&m);
		init_db_reviewtable (&a);

		if (m.rt.t == NULL || a.rt.t == NULL) {
			printf("메모리가 부족해서 프로그램을 종료합니다.\n");
			getchar();
			exit(1);
		}
		printf("메모리 초기화 완료\n");
		if (argc < 3)		
			strcpy(file_location, "M_data.mt");
		else 
			strcpy(file_location, argv[2]);
		printf("file location %s\n", file_location);
		fp = fopen(file_location, "rb");

		if (fp != NULL) {
			if (fread(&count, sizeof(int), 1, fp) != 0) {
				m.rt.count = count;
				printf("%d\n", m.rt.count);
				fread(&(m.last_id), sizeof(int), 1, fp);
				memory_reallocation(&m);
				fread(m.rt.t, sizeof(REVIEW), count, fp);
				fread(&count, sizeof(int), 1, fp);
				a.rt.count = count;
				printf("%d\n", a.rt.count);
				fread(&(a.last_id), sizeof(int), 1, fp);
				memory_reallocation(&a);
				fread(a.rt.t, sizeof(REVIEW), count, fp);
			}
			fclose(fp);
		}
		printf("파일읽기완료\n");

		ref.pt = NULL;
		REQUEST req;

		while(1) { //첫화면 - 1
			DB_REVIEW_TABLE* select_array[2] = {&m, &a};
			read(connfd, &req, sizeof(req));
			printf("user request %d\n", req.op_code);
			int op1, op2, op_select;

			op_select = req.op_code / DIV2;
			op1 = (req.op_code % DIV2) / DIV;
			op2 = req.op_code % DIV;
			sel = select_array[op_select];

			switch(op1) {
				case OP_SEND_REVIEW_TABLE:
					ref_table_sync(&ref, sel);
					send_REVIEW_TABLE(connfd, &ref);
					break;
				case OP_SEND_LAST_ID:
					write(connfd, &(sel->last_id), sizeof(int));
					break;
				case OP_SEND_REVIEW:
					if (find_review_by_id(sel, req.arg) == -1) {
						temp.id_number = -1;
						write(connfd, &temp, sizeof(REVIEW));
						printf("%d\n", temp.id_number);
					}
					else {
						write(connfd, &(sel->rt.t[find_review_by_id(sel, req.arg)]), sizeof(REVIEW));
						printf("%d\n", sel->rt.t[find_review_by_id(sel, req.arg)].id_number);
					}
					break;
				case OP_RECEIVE_INPUT_REVIEW:
					sel->rt.count++;
					memory_reallocation(sel);
					read(connfd, &(sel->rt.t[sel->rt.count-1]), sizeof(REVIEW));
					sel->rt.t[sel->rt.count-1].id_number = sel->last_id;
					sel->last_id++;
					write_to_file(file_location, &m, &a);
					break;
				case OP_RECEIVE_EDIT_REVIEW:
					read(connfd, &temp, sizeof(REVIEW));
					memcpy(req.arg, &temp.id_number, sizeof(int));
					sel->rt.t[find_review_by_id(sel, req.arg)] = temp;
					write_to_file(file_location, &m, &a);
					break;
				case OP_RECEIVE_DEL_REVIEW:
					read(connfd, &temp, sizeof(REVIEW));
					memcpy(req.arg, &temp.id_number, sizeof(int));
					int j = find_review_by_id(sel, req.arg);
					for (i=0; i<sel->rt.count-j; i++) {
						sel->rt.t[j+i] = sel->rt.t[j+1+i];
					}
					sel->rt.count --;
					write_to_file(file_location, &m, &a);
					break;
				case OP_DESC_SORT:
					ref_table_sync(&ref, sel);
					qsort((void*)ref.pt, ref.count, sizeof(REVIEW*), desc_sort_array[op2]);
					send_REVIEW_TABLE(connfd, &ref);
					break;
				case OP_ASC_SORT:
					ref_table_sync(&ref, sel);
					qsort((void*)ref.pt, ref.count, sizeof(REVIEW*), asc_sort_array[op2]);
					send_REVIEW_TABLE(connfd, &ref);
					break;
				case OP_SAME:
					select_review(&ref, sel, same_method_array[op2], req.arg);
					send_REVIEW_TABLE(connfd, &ref);
					break;
			}
		}
	}
	return 0;
}

int sort_by_id_number_desc(const void* a, const void* b) {
	return (*((REVIEW**)a))->id_number < (*((REVIEW**)b))->id_number;
}

int sort_by_title_desc(const void* a, const void* b) {
	return strcmp((*((REVIEW**)a))->title, (*((REVIEW**)b))->title) < 0;
}

int sort_by_genre_desc(const void* a, const void* b) {
	return strcmp((*((REVIEW**)a))->genre, (*((REVIEW**)b))->genre) < 0;
}

int sort_by_story_score_desc(const void* a, const void* b) {
	return (*((REVIEW**)a))->story_score < (*((REVIEW**)b))->story_score;
}

int sort_by_music_score_desc(const void* a, const void* b) {
	return (*((REVIEW**)a))->music_score < (*((REVIEW**)b))->music_score;
}

int sort_by_casting_score_desc(const void* a, const void* b) {
	return (*((REVIEW**)a))->casting_score < (*((REVIEW**)b))->casting_score;
}

int sort_by_timer_desc(const void* a, const void* b) {
	return (*((REVIEW**)a))->timer < (*((REVIEW**)b))->timer;
}

int sort_by_id_number_asc(const void* a, const void* b) {
	return (*((REVIEW**)a))->id_number > (*((REVIEW**)b))->id_number;
}

int sort_by_title_asc(const void* a, const void* b) {
	return strcmp((*((REVIEW**)a))->title, (*((REVIEW**)b))->title) > 0;
}

int sort_by_genre_asc(const void* a, const void* b) {
	return strcmp((*((REVIEW**)a))->genre, (*((REVIEW**)b))->genre) > 0;
}

int sort_by_story_score_asc(const void* a, const void* b) {
	return (*((REVIEW**)a))->story_score > (*((REVIEW**)b))->story_score;
}

int sort_by_music_score_asc(const void* a, const void* b) {
	return (*((REVIEW**)a))->music_score > (*((REVIEW**)b))->music_score;
}

int sort_by_casting_score_asc(const void* a, const void* b) {
	return (*((REVIEW**)a))->casting_score > (*((REVIEW**)b))->casting_score;
}

int sort_by_timer_asc(const void* a, const void* b) {
	return (*((REVIEW**)a))->timer > (*((REVIEW**)b))->timer;
}

void select_review(REVIEW_REF_TABLE* rrt, DB_REVIEW_TABLE* db_rt, int (*fp)(const REVIEW*, const char*), const char* user_input) {	
	int i;
	rrt->pt = (REVIEW**)realloc(rrt->pt, db_rt->rt.count*sizeof(REVIEW*));
	rrt->count = 0;

	for (i=0; i<db_rt->rt.count; i++) {
		if(fp(&(db_rt->rt.t[i]), user_input)) {
			rrt->pt[rrt->count] = &(db_rt->rt.t[i]);
			rrt->count++;
		}
	}
}

int find_review_by_id(DB_REVIEW_TABLE* db_rt, char* user_input) {
	int i;
	for (i=0; i<db_rt->last_id-1; i++) {
		if(same_by_id(&(db_rt->rt.t[i]), user_input)) {
			return i;
		}
	}
	return -1;
}

int same_by_id(REVIEW* a, char* user_input) {
	return *((int*)user_input) == a->id_number;
}

int same_by_genre(const REVIEW* a, const char* user_genre) {
	return strcmp((char*)user_genre, a->genre) == 0;
}

int same_by_story_score(const REVIEW* a, const char* user_story_score) {
	return atoi(user_story_score) == a->story_score;
}

int same_by_music_score(const REVIEW* a, const char* user_music_score) {
	return atoi(user_music_score) == a->music_score;
}

int same_by_casting_score(const REVIEW* a, const char* user_casting_score) {
	return atoi(user_casting_score) == a->casting_score;
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

void write_to_file(char* file_location, DB_REVIEW_TABLE* m, DB_REVIEW_TABLE* a) {
	FILE *fp;
	fp = fopen(file_location, "wb");

	fwrite(&(m->rt.count), sizeof(int), 1, fp);
	fwrite(&(m->last_id), sizeof(int), 1, fp);
	fwrite(m->rt.t, sizeof(REVIEW), m->rt.count, fp);
	fwrite(&(a->rt.count), sizeof(int), 1, fp);
	fwrite(&(a->last_id), sizeof(int), 1, fp);
	fwrite(a->rt.t, sizeof(REVIEW), a->rt.count, fp);
	fclose(fp);
}

void memory_reallocation(DB_REVIEW_TABLE* db_rt) {
	db_rt->rt.t = (REVIEW*)realloc(db_rt->rt.t, (db_rt->rt.count+1)*sizeof(REVIEW));
	if (db_rt->rt.t == NULL) {
		printf("메모리가 부족해서 프로그램을 종료합니다.\n");
		getchar();
		exit(1);
	}
}

void init_db_reviewtable (DB_REVIEW_TABLE* db_rt) {
	db_rt->rt.t = (REVIEW*)malloc(sizeof(REVIEW));
	db_rt->rt.count = 0;
	db_rt->last_id = 1;
}

void send_REVIEW_TABLE (int connfd, REVIEW_REF_TABLE* rrt) {
	int i;
	REVIEW* buf = (REVIEW*)malloc(rrt->count*sizeof(REVIEW));
	for (i=0; i<rrt->count; i++) {
		buf[i] = *(rrt->pt[i]);
	}
	write(connfd, &(rrt->count), sizeof(int));
	write(connfd, buf, sizeof(REVIEW)*rrt->count);
	free(buf);
}

void ref_table_sync (REVIEW_REF_TABLE* rrt, DB_REVIEW_TABLE* db_rt) {
	rrt->count = db_rt->rt.count;
	rrt->pt = (REVIEW**)realloc(rrt->pt, rrt->count*sizeof(REVIEW*));
	int i;

	for (i=0; i<db_rt->rt.count; i++) {
		rrt->pt[i] = &(db_rt->rt.t[i]);
	}
}

