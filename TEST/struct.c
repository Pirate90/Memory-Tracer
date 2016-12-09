#include <stdio.h>
#include <string.h>
#include <time.h>
#define AN 10//Array Number
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
		struct REVIEW t[10];
} REVIEW_TABLE;

char* timeToString(struct tm* t);
void get_input_range(int* input, int max_r, int min_r);
void print_reviewtable(REVIEW_TABLE* rt);
void sort(REVIEW_TABLE* rt, int da, int n);
void swap(REVIEW *first, REVIEW *second);
void same(REVIEW_TABLE* rt, int n, char* g, int s);
void print_sametable(REVIEW* a, int index);
void print_selectreview(REVIEW* a);
void print_star(int score);
int count_korean_char(char* s);
int format_width_count(char* s, int width);
REVIEW* fc(REVIEW_TABLE* rt);
REVIEW* fi(REVIEW_TABLE* rt, int index);
void write_to_file(char* file_location, REVIEW_TABLE* m, REVIEW_TABLE* a);

int main(int argc, char* argv[]) {
		REVIEW_TABLE m, a;
		REVIEW temp;
		REVIEW_TABLE* sel;
		int sorted_m[AN];
		FILE *fp;
		int user_choice;
		int i;
		int count = 0;
		char file_location[80];
		char s_genre[GENRE_BYTE];
		int s_score;

		if (argc == 1)		
				strcpy(file_location, "M_data.dat");
		else 
				strcpy(file_location, argv[1]);

		fp = fopen(file_location, "rb");

		if (fp != NULL) {
				fread(&count, sizeof(int), 1, fp);
				m.count = count;
				fread(m.t, sizeof(REVIEW), count, fp);
				fread(&count, sizeof(int), 1, fp);
				a.count = count;
				fread(a.t, sizeof(REVIEW), count, fp);
				fclose(fp);
		}

		while(1) { //첫화면 - 1
				printf("Memory Tracer에 오신것을 환영합니다.\n");
				printf("메뉴를 선택하세요.\n");
				printf("1. 새로운 감상문 입력하기\n");
				printf("2. 감상문목록 관리하기\n");
				printf("3. 추천 목록보기\n");
				printf("4. 종료하기\n");

				printf("당신의 선택은?\n");
				get_input_range(&user_choice, 4, 1);

				if(user_choice == 1) { //감상문 입력페이지 - 1.1
						printf("1. 영화감상문 입력\n2. 애니메이션 감상문 입력\n");
						get_input_range(&user_choice, 2, 1);

						if(user_choice == 1)
								sel = &m;
						else
								sel = &a;
						REVIEW* mar = fc(sel);

						mar->number = sel->count + 1;
						printf("제목을 입력하세요 (단, 한글 10자 이내) : ");
						fgets(mar->title, TITLE_BYTE, stdin);
						mar->title[strlen(mar->title) - 1] = '\0';
						printf("장르를 입력하세요 (액션, SF, 멜로, 공포 중에서) : ");
						scanf("%s", mar->genre);
						printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						get_input_range(&(mar->story_score), 10, 0);
						printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						get_input_range(&(mar->music_score), 10, 0);
						printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						get_input_range(&(mar->casting_score), 10, 0);
						printf("스토리를 입력하세요 (단, 한글 50자 이내) : ");
						fgets(mar->story, STORY_BYTE, stdin);
						printf("느낀점을 입력하세요 (단, 한글 500자 이내) : ");
						fgets(mar->review, REVIEW_BYTE, stdin);
						mar->timer = time(NULL);

						printf("입력이 완료되었습니다.\n 1. 저장하겠습니다.\n 2. 취소하겠습니다.");
						get_input_range(&user_choice, 2, 1);

						if(user_choice == 1) {
								sel->count++;
								write_to_file(file_location, &m, &a);
						}
						else if(user_choice == 2) {	
						}	
				}
				else if(user_choice == 2) { //감상문 관리페이지 - 1.2
						printf("1. 영화감상문 목록으로\n2. 애니메이션 감상문 목록으로\n");
						get_input_range(&user_choice, 2, 1);
						if(user_choice == 1)
								sel = &m;
						else
								sel = &a;

						sort(sel, 1, 0);
						print_reviewtable(sel);
						while(1) {
								printf("1. 감상문 목록 정렬\n");
								printf("2. 감상문 자세히보기\n");
								printf("3. 감상문 편집하기\n");
								printf("4. 감상문 삭제하기\n");
								printf("5. 메인화면으로\n");
								printf("당신의 선택은?\n");
								get_input_range(&user_choice, 5, 1);

								if (user_choice == 1) { //감상문 목록 정렬페이지 - 1.2.1
										printf("감상문 목록을 정렬하는 페이지입니다.\n");
										printf("1. 정렬하기(번호, 제목, 장르, 스토리점수, 음악점수, 캐스팅점수, 저장날짜)\n");
										printf("2. 모아보기(장르, 스토리점수, 음악점수, 캐스팅점수)\n");
										get_input_range(&user_choice, 2, 1);
										if (user_choice == 1) {
												printf("기준으로 정렬할 항목을 적어주세요.\n");
												printf("1. 번호\n2. 제목\n3. 장르\n4. 스토리점수\n5. 음악점수\n6. 캐스팅점수\n7. 저장날짜\n");
												get_input_range(&user_choice, 7, 1);
												if (user_choice == 1) {
														printf("번호를 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n");
														get_input_range(&user_choice, 2, 1);
														if (user_choice == 1) {
																sort(sel, 0, 0);
																print_reviewtable(sel);
														}
														else if (user_choice == 2) {
																sort(sel, 1, 0); 
																print_reviewtable(sel);
														}
												}
												else if (user_choice == 2) {
														printf("제목을 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n");
														get_input_range(&user_choice, 2, 1);
														if (user_choice == 1) {
																sort(sel, 0, 1);
																print_reviewtable(sel);
														}
														else if (user_choice == 2) {
																sort(sel, 1, 1);
																print_reviewtable(sel);
														}
												}
												else if (user_choice == 3) {
														printf("장르를 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n");
														get_input_range(&user_choice, 2, 1);
														if (user_choice == 1) {
																sort(sel, 0, 2);
																print_reviewtable(sel);
														}
														else if (user_choice == 2) {
																sort(sel, 1, 2);
																print_reviewtable(sel);
														}
												}
												else if (user_choice == 4) {
														printf("스토리점수를 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n");
														get_input_range(&user_choice, 2, 1);
														if (user_choice == 1) {
																sort(sel, 0, 3);
																print_reviewtable(sel);
														}
														else if (user_choice == 2) {
																sort(sel, 1, 3);
																print_reviewtable(sel);
														}
												}
												else if (user_choice == 5) {
														printf("음악점수를 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n");
														get_input_range(&user_choice, 2, 1);
														if (user_choice == 1) {
																sort(sel, 0, 4);
																print_reviewtable(sel);
														}
														else if (user_choice == 2) {
																sort(sel, 1, 4);
																print_reviewtable(sel);
														}
												}
												else if (user_choice == 6) {
														printf("캐스팅점수를 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n");
														get_input_range(&user_choice, 2, 1);
														if (user_choice == 1) {
																sort(sel, 0, 5);
																print_reviewtable(sel);
														}
														else if (user_choice == 2) {
																sort(sel, 1, 5);
																print_reviewtable(sel);
														}
												}
												else if (user_choice == 7) {
														printf("저장날짜를 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n");
														get_input_range(&user_choice, 2, 1);
														if (user_choice == 1) {
																sort(sel, 0, 6);
																print_reviewtable(sel);
														}
														else if (user_choice == 2) {
																sort(sel, 1, 6);
																print_reviewtable(sel);
														}
												}
										}
										else if (user_choice == 2) {
												printf("같은 값으로 모아볼 항목을 적어주세요.\n");
												printf("1. 장르\n2. 스토리점수\n3. 음악점수\n4. 캐스팅점수\n");
												get_input_range(&user_choice, 4, 1);
												if (user_choice == 1) {
														printf("모아 볼 장르를 적어주세요.\n");
														scanf("%s", s_genre);
														same(sel, 0, s_genre, s_score);
												}
												else if (user_choice == 2) {
														printf("모아 볼 스토리점수를 적어주세요.\n");
														scanf("%d", &s_score);
														same(sel, 1, s_genre, s_score);
												}
												else if (user_choice == 3) {
														printf("모아 볼 음악점수를 적어주세요.\n");
														scanf("%d", &s_score);
														same(sel, 2, s_genre, s_score);
												}
												else if (user_choice == 4) {
														printf("모아 볼 캐스팅점수를 적어주세요.\n");
														scanf("%d", &s_score);
														same(sel, 3, s_genre, s_score);
												}
										}
								}
								else if (user_choice == 2) { //선택감상문 보는페이지 - 1.2.2 
										sort(sel, 1, 0);
										printf("자세히 보고 싶은 감상문의 번호를 적어주세요.\n");
										get_input_range(&user_choice, sel->count, 1);
										user_choice = user_choice - 1;
										print_selectreview(fi(sel, user_choice));
										printf("1. 목록화면으로 이동\n2. 첫화면으로 이동\n");
										get_input_range(&user_choice, 2, 1);
										if (user_choice == 1) {
												print_reviewtable(sel);
										}
										else if (user_choice == 2) break;
								}
								else if (user_choice == 3) { //선택감상문 편집페이지 - 1.2.3
										sort(sel, 1, 0);
										printf("편집하고 싶은 감상문의 번호를 적어주세요.\n");
										get_input_range(&user_choice, sel->count, 1);
										temp.number = user_choice;
										user_choice = user_choice -1;
										REVIEW* ucr = fi(sel, user_choice);
										printf("제목을 입력하세요 (단, 한글 10자 이내) : (이전에 입력한 내용: %s) ", ucr->title);
										fgets(temp.title, TITLE_BYTE, stdin);
										temp.title[strlen(temp.title) - 1] = '\0';
										printf("장르를 입력하세요 (액션, SF, 멜로, 공포 중에서) : (이전에 입력한 내용: %s) ", ucr->genre);
										scanf("%s", temp.genre);
										printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : (이전에 입력한 내용: %d) ", ucr->story_score);
										get_input_range(&temp.story_score, 10, 0);
										printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : (이전에 입력한 내용: %d) ", ucr->music_score);
										get_input_range(&temp.music_score, 10, 0);
										printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : (이전에 입력한 내용: %d) ", ucr->casting_score);
										get_input_range(&temp.casting_score, 10, 0);
										printf("스토리를 입력하세요 (단, 한글 50자 이내) : (이전에 입력한 내용: %s) ", ucr->story);
										fgets(temp.story, STORY_BYTE, stdin);
										printf("느낀점을 입력하세요 (단, 한글 500자 이내) : (이전에 입력한 내용: %s) ", ucr->review);
										fgets(temp.review, REVIEW_BYTE, stdin);
										temp.timer = time(NULL);

										printf("입력이 완료되었습니다.\n1. 저장하겠습니다.\n2. 취소하겠습니다.");
										get_input_range(&i, 2, 1);

										if(i == 1) {
												*ucr = temp;
												write_to_file(file_location, &m, &a);	
										}
										else if(i == 2) {	
										}
										print_reviewtable(sel);
								}
								else if (user_choice == 4) { //선택감상문 삭제페이지 - 1.2.4
										sort(sel, 1, 0);
										printf("삭제하고 싶은 감상문의 번호를 적어주세요.\n");
										get_input_range(&user_choice, sel->count, 1);
										REVIEW* ucr = fi(sel, user_choice);
										printf("%s을(를) 선택하셨습니다.\n1. 삭제하겠습니다.\n2. 취소하겠습니다.", (ucr-1)->title);
										get_input_range(&i, 2, 1);

										if(i == 1) {
												for (i=0; i<sel->count-user_choice; i++) {
														*(ucr-1+i) = *(ucr+i);
														(ucr-1+i)->number = (ucr-1+i)->number - 1;
												}
												sel->count --;
												write_to_file(file_location, &m, &a);
										}
										else if(i == 2) {	
										}
										print_reviewtable(sel);
								}
								else if (user_choice == 5) { //메인화면으로 이동페이지 - 1.2.5
										break;
								}
						}
				}
				else if(user_choice == 3) { //감상문 추천페이지 - 1.3
						printf("추천하는 목록은 --- 입니다.");
				}
				else if(user_choice == 4) { // 프로그램 종료페이지 - 1.4
						break;
				}
		}
		return 0;
}

char* timeToString(struct tm* t) {
		static char s[20];

		sprintf(s, "%04d-%02d-%02d %02d:%02d:%02d", t->tm_year + 1900, t->tm_mon + 1, t->tm_mday + (int)((t->tm_hour + 9) / 24), (t->tm_hour + 9) % 24, t->tm_min, t->tm_sec);

		return s;
}

void get_input_range(int* input, int max_r, int min_r) { //사용자로부터 점수를 입력받는 함수
		scanf("%d", input);
		getchar();
		while((*input) > max_r || (*input) < min_r) {
				printf("%d~%d사이의 정수의 값을 다시 입력해주세요!", min_r, max_r);
				scanf("%d", input);
				getchar();
		}
}

void print_reviewtable(REVIEW_TABLE* rt) { //감상문테이블을 화면으로 출력하는 함수
		int i;
		struct tm* t;
		char buf[100];
		int count = rt->count;
		REVIEW* a = rt->t;

		printf("\n|%-8s|%-22s|%-10s|%-15s|%-14s|%-15s|%s\n","번호","제목","장르","스토리점수","음악점수","캐스팅점수","저장날짜");

		for(i=0; i<count; i++) {
				sprintf(buf, "|%%-6d|%%-%ds|%%-%ds|", format_width_count(a[i].title, KOREAN_WIDTH * TITLE_LENGTH), format_width_count(a[i].genre, KOREAN_WIDTH * GENRE_LENGTH));
				printf(buf,a[i].number,a[i].title,a[i].genre);
				print_star(a[i].story_score);
				printf("%s", "|");
				print_star(a[i].music_score);
				printf("%s", "|");
				print_star(a[i].casting_score);
				printf("%s", "|");
				t = localtime(&(a[i].timer));
				printf("%s\n", timeToString(t));
		}
		printf("\n");
}

void sort(REVIEW_TABLE* rt, int da, int n) {
		int count = rt->count;
		REVIEW* a = rt->t;
		int i, j;

		switch (da) {
				case 0://내림차순
						switch (n) {
								case 0:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].number < a[j+1].number) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 1:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (strcmp(a[j].title, a[j+1].title) < 0) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 2:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (strcmp(a[j].genre, a[j+1].genre) < 0) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 3:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].story_score < a[j+1].story_score) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 4:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].music_score < a[j+1].music_score) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 5:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].casting_score < a[j+1].casting_score) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 6:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].timer < a[j+1].timer) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
						}
						break;
				case 1://오름차순
						switch (n) {
								case 0:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].number > a[j+1].number) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 1:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (strcmp(a[j].title, a[j+1].title) > 0) {
																swap(&a[j],&a[j+1]);
														}
												}
										}break;
								case 2:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (strcmp(a[j].genre, a[j+1].genre) > 0) {
																swap(&a[j],&a[j+1]);
														}
												}
										}break;
								case 3:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].story_score > a[j+1].story_score) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 4:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].music_score > a[j+1].music_score) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 5:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].casting_score > a[j+1].casting_score) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
								case 6:
										for (i=0; i<count; i++) {
												for (j=0; j<count-1; j++) {
														if (a[j].timer > a[j+1].timer) {
																swap(&a[j],&a[j+1]);
														}
												}
										}
										break;
						}
						break;
		}

}								

void swap(REVIEW *first, REVIEW *second) { 
		REVIEW tmp;
		tmp = *first; 
		*first = *second; 
		*second = tmp; 
}

void same(REVIEW_TABLE* rt, int n, char* g, int s) {
		int count = rt->count;
		REVIEW* a = rt->t;
		int i;
		printf("\n|%-8s|%-22s|%-10s|%-15s|%-14s|%-15s|%s\n","번호","제목","장르","스토리점수","음악점수","캐스팅점수","저장날짜");
		switch (n) {
				case 0:
						for (i=0; i<count; i++) {
								if (strcmp(g, a[i].genre) == 0) {
										print_sametable(a, i);
								}
						}
						break;
				case 1:
						for (i=0; i<count; i++) {
								if (s == a[i].story_score) {
										print_sametable(a, i);
								}
						}
						break;
				case 2:
						for (i=0; i<count; i++) {
								if (s == a[i].music_score) {
										print_sametable(a, i);
								}
						}
						break;
				case 3:
						for (i=0; i<count; i++) {
								if (s == a[i].casting_score) {
										print_sametable(a, i);
								}
						}
						break;
		}
		printf("\n");
}

void print_sametable(REVIEW* a, int index) { //감상문테이블을 화면으로 출력하는 함수
		struct tm* t;
		char buf[100];
		
		sprintf(buf, "|%%-6d|%%-%ds|%%-%ds|", format_width_count(a[index].title, KOREAN_WIDTH * TITLE_LENGTH), format_width_count(a[index].genre, KOREAN_WIDTH * GENRE_LENGTH));
		printf(buf,a[index].number,a[index].title,a[index].genre);
		print_star(a[index].story_score);
		printf("%s", "|");
		print_star(a[index].music_score);
		printf("%s", "|");
		print_star(a[index].casting_score);
		printf("%s", "|");
		t = localtime(&(a[index].timer));
		printf("%s\n", timeToString(t));
}

void print_selectreview(REVIEW* a) { //특정감상문의 자세한 내용을 화면으로 출력하는 함수
		struct tm* t;
		t = localtime(&(a->timer));
		printf("\n|제목 : %s\n|장르 : %s\n",a->title,a->genre);
		printf("|스토리점수 : ");
		print_star(a->story_score);
		printf("\n|음악점수   : ");
		print_star(a->music_score);
		printf("\n|캐스팅점수 : ");
		print_star(a->casting_score);
		printf("\n|스토리 : %s|느낀점 : %s",a->story,a->review);
		printf("|저장날짜 : %s\n\n", timeToString(t));
}

void print_star(int score) { //점수를 별표로 변환하여 화면으로 출력하는 함수
		int i;
		for(i=0; i<score; i++)
				printf ("★");
		for(i=0; i<10-score; i++)
				printf ("☆");
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
