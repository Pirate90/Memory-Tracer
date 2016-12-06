#include <stdio.h>
#include <string.h>
#define AN 10//Array Number

typedef struct REVIEW {
		char title[21];
		char genre[9];
		int story_score,music_score,casting_score;
		char story[101];
		char review[1001];
		char pic[5][100];
} REVIEW;

void insert_score(int* score);
void print_reviewtable(REVIEW* a);
void print_star(int score);

int main(int argc, char* argv[]) {
		REVIEW m[AN];
		FILE *fp;
		int user_choice;
		int i;
		int count = 0;

		if (argc == 1)
				fp = fopen("M_data.dat", "rb");
		else 
				fp = fopen(argv[1], "rb");

		if (fp != NULL) {
				fread(&count, 4, 1, fp);
				fread(m, sizeof(REVIEW), count, fp);
				fclose(fp);
		}

		while(1) {
				printf("Memory Tracer에 오신것을 환영합니다.\n");
				printf("메뉴를 선택하세요.\n");
				printf("1. 새로운 감상문 입력하기\n");
				printf("2. 감상문목록 관리하기\n");
				printf("3. 추천 목록보기\n");
				printf("4. 종료하기\n");

				printf("당신의 선택은?\n");
				scanf("%d", &user_choice);
				getchar();

				if(user_choice == 1) {
						printf("제목을 입력하세요 (단, 한글 10자 이내) : ");
						fgets(m[count].title, 21, stdin);
						printf("장르를 입력하세요 (액션, SF, 멜로, 공포 중에서)) : ");
						scanf("%s", m[count].genre);
						printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						insert_score(&m[count].story_score);
						printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						insert_score(&m[count].music_score);
						printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						insert_score(&m[count].casting_score);
						getchar();
						printf("스토리를 입력하세요 (단, 한글 50자 이내) : ");
						fgets(m[count].story, 101, stdin);
						printf("느낀점을 입력하세요 (단, 한글 500자 이내) : ");
						fgets(m[count].review, 1001, stdin);

						printf("입력이 완료되었습니다.\n 1.저장하겠습니다.\n 2.취소하겠습니다.");
						scanf("%d", &user_choice);
						while(user_choice>2 || user_choice<1) {
								printf("1이나 2중에 하나의 값만 다시 입력해주세요!");
								scanf("%d", &user_choice);
						}
						if(user_choice == 1) {
								if (argc == 1)
										fp = fopen("M_data.dat", "wb");
								else
										fp = fopen(argv[1], "wb");
								count ++;
								fwrite(&count, 4, 1, fp);
								fwrite(m, sizeof(REVIEW), count, fp);
								fclose(fp);
						}
						else if(user_choice == 2) {	
						}	
				}
				else if(user_choice == 2) {
						for(i=0; i<count; i++) {
								print_reviewtable(&m[i]);
						}
				}
				else if(user_choice == 3) {
						printf("추천하는 목록은 --- 입니다.");
				}
				else if(user_choice == 4) {
						break;
				}
		}
		return 0;
}



void insert_score(int* score) {
		scanf("%d", score);
		while((*score) > 10 || (*score) < 0) {
				printf("0~10사이의 정수의 값을 다시 입력해주세요!");
				scanf("%d", score);
		}
}

void print_reviewtable(REVIEW* a) {
		printf("제목 : %s\t,장르 : %s\t,",a->title,a->genre);
		printf("스토리점수 : ");
		print_star(a->story_score);
		printf("\t,음악점수 : ");
		print_star(a->music_score);
		printf("\t,캐스팅점수 : ");
		print_star(a->casting_score);
		printf("\t,스토리 : %s\t,느낀점 : %s\t\n",a->story,a->review);
}

void print_star(int score) {
		int i;
		for(i=0; i<score; i++)
				printf ("★");
		for(i=0; i<10-score; i++)
				printf ("☆");
}
