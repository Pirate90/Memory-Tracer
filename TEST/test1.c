#include <stdio.h>
#include <string.h>

void insert_score(int* score);
void star(int score,char* starr);

int main(void) {
		char title[10][21];
		char genre[10][9];
		int story_score[10],music_score[10],casting_score[10];
		char story_star[10][31]={0,},music_star[10][31]={0,},casting_star[10][31]={0,};
		char story[10][101];
		char review[10][1001];
		char pic[10][5][100];
		int user_choice;
		int i;
		int count = 0;

		FILE *fp = fopen("M_data.dat", "rb");
		if (fp != NULL) {
				fread(&count, 4, 1, fp);
				fread(title, 21, 10, fp);
				fread(genre, 9, 10, fp);
				fread(story_score, 4, 10, fp);
				fread(music_score, 4, 10, fp);
				fread(casting_score, 4, 10, fp);
				for (i=0; i<count; i++) {
						star(story_score[i], story_star[i]);
						star(music_score[i], music_star[i]);
						star(casting_score[i], casting_star[i]);
				}
				fread(story, 101, 10, fp);
				fread(review, 1001, 10, fp);
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
						fgets(title[count], 21, stdin);
						printf("장르를 입력하세요 (액션, SF, 멜로, 공포 중에서)) : ");
						scanf("%s", genre[count]);
						printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						insert_score(&story_score[count]);
						star(story_score[count], story_star[count]);
						printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						insert_score(&music_score[count]);
						star(music_score[count], music_star[count]);
						printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						insert_score(&casting_score[count]);
						star(casting_score[count], casting_star[count]);
						getchar();
						printf("스토리를 입력하세요 (단, 한글 50자 이내) : ");
						fgets(story[count], 101, stdin);
						printf("느낀점을 입력하세요 (단, 한글 500자 이내) : ");
						fgets(review[count], 1001, stdin);

						printf("입력이 완료되었습니다.\n 1.저장하겠습니다.\n 2.취소하겠습니다.");
						scanf("%d", &user_choice);
						while(user_choice>2 || user_choice<1) {
								printf("1이나 2중에 하나의 값만 다시 입력해주세요!");
								scanf("%d", &user_choice);
						}
						if(user_choice == 1) {
								FILE *fp = fopen("M_data.dat", "wb");
								count ++;
								fwrite(&count, 4, 1, fp);
								fwrite(title, 21, 10, fp);
								fwrite(genre, 9, 10, fp);
								fwrite(&story_score, 4, 10, fp);
								fwrite(&music_score, 4, 10, fp);
								fwrite(&casting_score, 4, 10, fp);
								fwrite(story, 101, 10, fp);
								fwrite(review, 1001, 10, fp);
								fclose(fp);
						}
						else if(user_choice == 2) {	
						}	
				}
				else if(user_choice == 2) {
						for(i=0; i<count; i++) {
								printf("제목 : %s\t,장르 : %s\t,스토리점수 : %s\t,음악점수 : %s\t,그림체점수 : %s\t,스토리 : %s\t,느낀점 : %s\t\n",title[i],genre[i],story_star[i],music_star[i],casting_star[i],story[i],review[i]);
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

void star(int score,char* starr) {
		int i;
		for(i=0; i<score; i++)
				strcat(starr, "★");
		for(i=0; i<10-score; i++)
				strcat(starr, "☆");
}
