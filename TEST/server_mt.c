#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>

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
#define DIV 10

typedef struct REVIEW {
		int number;
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

typedef struct REQUEST {
		int op_code;
		char arg[80];
} REQUEST;

int sort_by_number_desc(const void*, const void*);
int sort_by_title_desc(const void*, const void*);
int sort_by_genre_desc(const void*, const void*);
int sort_by_story_score_desc(const void*, const void*);
int sort_by_music_score_desc(const void*, const void*);
int sort_by_casting_score_desc(const void*, const void*);
int sort_by_timer_desc(const void*, const void*);
int sort_by_number_asc(const void*, const void*);
int sort_by_title_asc(const void*, const void*);
int sort_by_genre_asc(const void*, const void*);
int sort_by_story_score_asc(const void*, const void*);
int sort_by_music_score_asc(const void*, const void*);
int sort_by_casting_score_asc(const void*, const void*);
int sort_by_timer_asc(const void*, const void*);
void select_review(const REVIEW_TABLE* a, REVIEW_TABLE* b, int (*fp)(const REVIEW*, const char*), const char* user_input);
int same_by_genre(const REVIEW* a, const char* user_genre);
int same_by_story_score(const REVIEW* a, const char* user_genre);
int same_by_music_score(const REVIEW* a, const char* user_genre);
int same_by_casting_score(const REVIEW* a, const char* user_genre);
int count_korean_char(char* s);
int format_width_count(char* s, int width);
REVIEW* fc(REVIEW_TABLE* rt);
REVIEW* fi(REVIEW_TABLE* rt, int index);
void write_to_file(char* file_location, REVIEW_TABLE* m, REVIEW_TABLE* a);
void memory_reallocation(REVIEW_TABLE* rt);
void init_reviewtable (REVIEW_TABLE* rt);
void send_reviewtable (int connfd, REVIEW_TABLE* rt);

int main(int argc, char* argv[]) {
		REVIEW_TABLE m, a;
		REVIEW temp;
		REVIEW_TABLE* sel;
		int listenfd, connfd;
		FILE *fp;
		int user_choice, user_choice2;
		int i;
		int count = 0;
		char file_location[80];
		char s_genre[GENRE_BYTE];
		int s_score;
		socklen_t clilen;
		struct sockaddr_in cliaddr, servaddr;

		int (*sort_method_array[2][7])(const void*, const void*) = {{sort_by_number_desc, sort_by_title_desc, sort_by_genre_desc, sort_by_story_score_desc, sort_by_music_score_desc, sort_by_casting_score_desc, sort_by_timer_desc}, {sort_by_number_asc, sort_by_title_asc, sort_by_genre_asc, sort_by_story_score_asc, sort_by_music_score_asc, sort_by_casting_score_asc, sort_by_timer_asc}};

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

				init_reviewtable (&m);
				init_reviewtable (&a);

				if (m.t == NULL || a.t == NULL) {
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
						fread(&count, sizeof(int), 1, fp);
						m.count = count;
						memory_reallocation(&m);
						fread(m.t, sizeof(REVIEW), count, fp);
						fread(&count, sizeof(int), 1, fp);
						a.count = count;
						memory_reallocation(&a);
						fread(a.t, sizeof(REVIEW), count, fp);
						fclose(fp);
				}
				printf("파일읽기완료\n");

				REQUEST req;

				while(1) { //첫화면 - 1
						read(connfd, &req, sizeof(req));
						printf("user request %d\n", req.op_code);

						REVIEW_TABLE selected;
						int op1 = req.op_code / DIV;
						int op2 = req.op_code % DIV;
						switch(op1) {
								case 0:
										send_reviewtable(connfd, &m);
								break;
								case 1: 
										init_reviewtable(&selected);
										select_review(&m, &selected, same_method_array[op2], req.arg);
										send_reviewtable(connfd, &selected);
										free(selected.t);
								break;


						}
				}
		}
		return 0;
}

int sort_by_number_desc(const void* a, const void* b) {
		return ((REVIEW*)a)->number < ((REVIEW*)b)->number;
}

int sort_by_title_desc(const void* a, const void* b) {
		return strcmp(((REVIEW*)a)->title, ((REVIEW*)b)->title) < 0;
}

int sort_by_genre_desc(const void* a, const void* b) {
		return strcmp(((REVIEW*)a)->genre, ((REVIEW*)b)->genre) < 0;
}

int sort_by_story_score_desc(const void* a, const void* b) {
		return ((REVIEW*)a)->story_score < ((REVIEW*)b)->story_score;
}

int sort_by_music_score_desc(const void* a, const void* b) {
		return ((REVIEW*)a)->music_score < ((REVIEW*)b)->music_score;
}

int sort_by_casting_score_desc(const void* a, const void* b) {
		return ((REVIEW*)a)->casting_score < ((REVIEW*)b)->casting_score;
}

int sort_by_timer_desc(const void* a, const void* b) {
		return ((REVIEW*)a)->timer < ((REVIEW*)b)->timer;
}

int sort_by_number_asc(const void* a, const void* b) {
		return ((REVIEW*)a)->number > ((REVIEW*)b)->number;
}

int sort_by_title_asc(const void* a, const void* b) {
		return strcmp(((REVIEW*)a)->title, ((REVIEW*)b)->title) > 0;
}

int sort_by_genre_asc(const void* a, const void* b) {
		return strcmp(((REVIEW*)a)->genre, ((REVIEW*)b)->genre) > 0;
}

int sort_by_story_score_asc(const void* a, const void* b) {
		return ((REVIEW*)a)->story_score > ((REVIEW*)b)->story_score;
}

int sort_by_music_score_asc(const void* a, const void* b) {
		return ((REVIEW*)a)->music_score > ((REVIEW*)b)->music_score;
}

int sort_by_casting_score_asc(const void* a, const void* b) {
		return ((REVIEW*)a)->casting_score > ((REVIEW*)b)->casting_score;
}

int sort_by_timer_asc(const void* a, const void* b) {
		return ((REVIEW*)a)->timer > ((REVIEW*)b)->timer;
}

void select_review(const REVIEW_TABLE* a, REVIEW_TABLE* b, int (*fp)(const REVIEW*, const char*), const char* user_input) {
		int i;

		for (i=0; i<a->count; i++) {
				if(fp(&(a->t[i]), user_input)) {
						b->t[b->count] = a->t[i];
						b->count++;
						memory_reallocation(b);
				}
		}
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

REVIEW* fc(REVIEW_TABLE* rt) {
		return &(rt->t[rt->count]);
}

REVIEW* fi(REVIEW_TABLE* rt, int index) {
		return &(rt->t[index]);
}

void write_to_file(char* file_location, REVIEW_TABLE* m, REVIEW_TABLE* a) {
		FILE *fp;
		fp = fopen(file_location, "wb");

		fwrite(&(m->count), sizeof(int), 1, fp);
		fwrite(m->t, sizeof(REVIEW), m->count, fp);
		fwrite(&(a->count), sizeof(int), 1, fp);
		fwrite(a->t, sizeof(REVIEW), a->count, fp);
		fclose(fp);
}

void memory_reallocation(REVIEW_TABLE* rt) {
		rt->t = (REVIEW*)realloc(rt->t, (rt->count+1)*sizeof(REVIEW));
		if (rt->t == NULL) {
				printf("메모리가 부족해서 프로그램을 종료합니다.\n");
				getchar();
				exit(1);
		}
}

void init_reviewtable (REVIEW_TABLE* rt) {
		rt->t = (REVIEW*)malloc(sizeof(REVIEW));
		rt->count = 0;
}

void send_reviewtable (int connfd, REVIEW_TABLE* rt) {
		write(connfd, &(rt->count), sizeof(int));
		write(connfd, rt->t, sizeof(REVIEW)*rt->count);
}

