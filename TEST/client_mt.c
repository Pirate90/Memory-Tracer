#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>

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

typedef struct REQUEST {
	int op_code;
	char arg[80];
} REQUEST;

char* timeToString(struct tm* t);
void get_input_range(int* input, int max_r, int min_r);
void print_reviewtable(REVIEW_TABLE* rt);
void print_selectreview(REVIEW* a);
void print_star(int score);
int count_korean_char(char* s);
int format_width_count(char* s, int width);
REVIEW* fc(REVIEW_TABLE* rt);
REVIEW* fi(REVIEW_TABLE* rt, int index);
void send_REVIEW_TABLE (int sockfd, REVIEW_TABLE* rt);
void receive_REVIEW_TABLE (int sockfd, REVIEW_TABLE* rt);

int main(int argc, char* argv[]) {
	REVIEW_TABLE rt;
	REVIEW temp;
	REVIEW_TABLE* sel = &rt;
	REQUEST req;
	int user_choice, user_choice2;
	int sel_m_a;
	int i;
	int count = 0;

	char user_criteria[7][20] = {"번호", "제목", "장르", "스토리점수", "음악점수", "캐스팅점수", "저장날짜"};

	int sockfd;
	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

	while(1) { //첫화면 - 1
		req.op_code = 0;
		req.arg[0] = '\0';

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

			sel_m_a = (user_choice-1)*DIV2;

			REVIEW* mar = &temp;
			mar->id_number = -1;
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
				req.op_code = OP_RECEIVE_INPUT_REVIEW*DIV + sel_m_a;
				write(sockfd, &req, sizeof(req));
				write(sockfd, mar, sizeof(REVIEW));
			}
			else if(user_choice == 2) {	
			}
		}
		else if(user_choice == 2) { //감상문 관리페이지 - 1.2
			printf("1. 영화감상문 목록으로\n2. 애니메이션 감상문 목록으로\n");
			get_input_range(&user_choice, 2, 1);
			sel_m_a = (user_choice-1)*DIV2;
			req.op_code = OP_SEND_REVIEW_TABLE*DIV + sel_m_a;
			write(sockfd, &req, sizeof(req));
			receive_REVIEW_TABLE (sockfd, sel);
			print_reviewtable(sel);
			free(sel->t);
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
						printf("%s을(를) 기준으로 정렬합니다.\n1. 내림차순\n2. 오름차순\n", user_criteria[user_choice-1]);
						get_input_range(&user_choice2, 2, 1);
						if (user_choice2 == 1) {
							req.op_code = (OP_DESC_SORT*DIV) + user_choice - 1 + sel_m_a;
							write(sockfd, &req, sizeof(req));
							receive_REVIEW_TABLE (sockfd, sel);
						}
						else {
							req.op_code = (OP_ASC_SORT*DIV) + user_choice - 1 + sel_m_a;
							write(sockfd, &req, sizeof(req));
							receive_REVIEW_TABLE (sockfd, sel);
						}
						print_reviewtable(sel);
						free(sel->t);
					}
					else if (user_choice == 2) {
						printf("같은 값으로 모아볼 항목을 적어주세요.\n");
						printf("1. 장르\n2. 스토리점수\n3. 음악점수\n4. 캐스팅점수\n");
						get_input_range(&user_choice, 4, 1);
						req.op_code = (OP_SAME*DIV) + user_choice - 1 + sel_m_a;
						printf("모아 볼 %s을(를) 적어주세요.\n", user_criteria[user_choice + 1]);
						scanf("%s", req.arg);
						write(sockfd, &req, sizeof(req));
						receive_REVIEW_TABLE (sockfd, sel);
						print_reviewtable(sel);
						free(sel->t);
					}
				}
				else if (user_choice == 2) { //선택감상문 보는페이지 - 1.2.2 
					printf("자세히 보고 싶은 감상문의 번호를 적어주세요.\n");
					req.op_code = OP_SEND_LAST_ID*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					read(sockfd, &count, sizeof(int));
					get_input_range(&user_choice, count-1, 1);
					memcpy(req.arg, &user_choice, sizeof(int));
					req.op_code = OP_SEND_REVIEW*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					read(sockfd, &temp, sizeof(REVIEW));
					while (temp.id_number == -1) {
						printf("입력하신 번호의 감상문은 없습니다. 정확한 번호를 다시 적어주세요.\n");
						get_input_range(&user_choice, count-1, 1);
						memcpy(req.arg, &user_choice, sizeof(int));
						req.op_code = OP_SEND_REVIEW*DIV + sel_m_a;
						write(sockfd, &req, sizeof(req));
						read(sockfd, &temp, sizeof(REVIEW));
					}	
					print_selectreview(&temp);
					printf("1. 목록화면으로 이동\n2. 첫화면으로 이동\n");
					get_input_range(&user_choice, 2, 1);
					if (user_choice == 1) {
						req.op_code = OP_SEND_REVIEW_TABLE*DIV + sel_m_a;
						write(sockfd, &req, sizeof(req));
						receive_REVIEW_TABLE (sockfd, sel);
						print_reviewtable(sel);
					}
					else if (user_choice == 2) break;
					free(sel->t);
				}
				else if (user_choice == 3) { //선택감상문 편집페이지 - 1.2.3
					printf("편집하고 싶은 감상문의 번호를 적어주세요.\n");
					req.op_code = OP_SEND_LAST_ID*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					read(sockfd, &count, sizeof(int));
					get_input_range(&user_choice, count-1, 1);
					memcpy(req.arg, &user_choice, sizeof(int));
					req.op_code = OP_SEND_REVIEW*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					read(sockfd, &temp, sizeof(REVIEW));
					while (temp.id_number == -1) {
						printf("입력하신 번호의 감상문은 없습니다. 정확한 번호를 다시 적어주세요.\n");
						get_input_range(&user_choice, count-1, 1);
						memcpy(req.arg, &user_choice, sizeof(int));
						req.op_code = OP_SEND_REVIEW*DIV + sel_m_a;
						write(sockfd, &req, sizeof(req));
						read(sockfd, &temp, sizeof(REVIEW));
					}

					printf("제목을 입력하세요 (단, 한글 10자 이내) : (이전에 입력한 내용: %s) ", temp.title);
					fgets(temp.title, TITLE_BYTE, stdin);
					temp.title[strlen(temp.title) - 1] = '\0';
					printf("장르를 입력하세요 (액션, SF, 멜로, 공포 중에서) : (이전에 입력한 내용: %s) ", temp.genre);
					scanf("%s", temp.genre);
					printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : (이전에 입력한 내용: %d) ", temp.story_score);
					get_input_range(&temp.story_score, 10, 0);
					printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : (이전에 입력한 내용: %d) ", temp.music_score);
					get_input_range(&temp.music_score, 10, 0);
					printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : (이전에 입력한 내용: %d) ", temp.casting_score);
					get_input_range(&temp.casting_score, 10, 0);
					printf("스토리를 입력하세요 (단, 한글 50자 이내) : (이전에 입력한 내용: %s) ", temp.story);
					fgets(temp.story, STORY_BYTE, stdin);
					printf("느낀점을 입력하세요 (단, 한글 500자 이내) : (이전에 입력한 내용: %s) ", temp.review);
					fgets(temp.review, REVIEW_BYTE, stdin);
					temp.timer = time(NULL);

					printf("입력이 완료되었습니다.\n1. 저장하겠습니다.\n2. 취소하겠습니다.");
					get_input_range(&i, 2, 1);

					if(i == 1) {
						req.op_code = OP_RECEIVE_EDIT_REVIEW*DIV + sel_m_a;
						write(sockfd, &req, sizeof(req));
						write(sockfd, &temp, sizeof(REVIEW));
					}
					else if(i == 2) {	
					}
					req.op_code = OP_SEND_REVIEW_TABLE*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					receive_REVIEW_TABLE (sockfd, sel);
					print_reviewtable(sel);
					free(sel->t);
				}
				else if (user_choice == 4) { //선택감상문 삭제페이지 - 1.2.4
					printf("삭제하고 싶은 감상문의 번호를 적어주세요.\n");
					req.op_code = OP_SEND_LAST_ID*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					read(sockfd, &count, sizeof(int));
					get_input_range(&user_choice, count-1, 1);
					memcpy(req.arg, &user_choice, sizeof(int));
					req.op_code = OP_SEND_REVIEW*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					read(sockfd, &temp, sizeof(REVIEW));
					while (temp.id_number == -1) {
						printf("입력하신 번호의 감상문은 없습니다. 정확한 번호를 다시 적어주세요.\n");
						get_input_range(&user_choice, count-1, 1);
						memcpy(req.arg, &user_choice, sizeof(int));
						req.op_code = OP_SEND_REVIEW*DIV + sel_m_a;
						write(sockfd, &req, sizeof(req));
						read(sockfd, &temp, sizeof(REVIEW));
					}
					printf("%s을(를) 선택하셨습니다.\n1. 삭제하겠습니다.\n2. 취소하겠습니다.", temp.title);
					get_input_range(&user_choice, 2, 1);

					if(user_choice == 1) {
						req.op_code = OP_RECEIVE_DEL_REVIEW*DIV + sel_m_a;
						write(sockfd, &req, sizeof(req));
						write(sockfd, &temp, sizeof(REVIEW));
					}
					else if(user_choice == 2) {	
					}
					req.op_code = OP_SEND_REVIEW_TABLE*DIV + sel_m_a;
					write(sockfd, &req, sizeof(req));
					receive_REVIEW_TABLE(sockfd, sel);
					print_reviewtable(sel);
					free(sel->t);
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

	printf("\n|%-8s|%-8s|%-22s|%-10s|%-15s|%-14s|%-15s|%s\n","순서","번호","제목","장르","스토리점수","음악점수","캐스팅점수","저장날짜");

	for(i=0; i<count; i++) {
		sprintf(buf, "|%%-6d|%%-6d|%%-%ds|%%-%ds|", format_width_count(a[i].title, KOREAN_WIDTH * TITLE_LENGTH), format_width_count(a[i].genre, KOREAN_WIDTH * GENRE_LENGTH));
		printf(buf, i+1, a[i].id_number,a[i].title,a[i].genre);
		print_star(a[i].story_score);
		printf("|");
		print_star(a[i].music_score);
		printf("|");
		print_star(a[i].casting_score);
		printf("|");
		t = localtime(&(a[i].timer));
		printf("%s\n", timeToString(t));
	}
	printf("\n");
}

void print_selectreview(REVIEW* a) { //특정감상문의 자세한 내용을 화면으로 출력하는 함수
	struct tm* t;
	t = localtime(&(a->timer));
	printf("\n|번호 : %d\n", a->id_number);
	printf("|제목 : %s\n|장르 : %s\n",a->title,a->genre);
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

void send_REVIEW_TABLE (int sockfd, REVIEW_TABLE* rt) {
	write(sockfd, &(rt->count), sizeof(int));
	write(sockfd, rt->t, sizeof(REVIEW)*rt->count);
}

void receive_REVIEW_TABLE (int sockfd, REVIEW_TABLE* rt) {
	read(sockfd, &(rt->count), sizeof(int));
	rt->t = (REVIEW*)malloc(sizeof(REVIEW)*(rt->count));
	read(sockfd, rt->t, sizeof(REVIEW)*(rt->count));
}

