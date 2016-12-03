#include <stdio.h>
#include <string.h>

char star(int* score,int count);

int main(void)
{
		char title[10][21];
		char genre[10][5];
		int story_score[10],music_score[10],casting_score[10];
		int* pss = story_score;
		int* pms = music_score;
		int* pcs = casting_score;
//		char story_star[10][21]={0,},music_star[10][21]={0,},casting_star[10][21]={0,};
		char story[10][101];
		char review[10][1001];
		char pic[10][5][100];
		int user_choice;
		int i;
		int count;

		count = 0;
				
//		FILE *fp = fopen("M_data.txt", "r");
//		if (fp != NULL) {
//				while(!feof(fp)) {
//						if(fgets(title[count], 21, fp) == NULL) break;
//						fgets(title[count], 21, fp);
//						fgets(genre[count], 5, fp);
//						count++;
//				}
//				fclose(fp);
//		}

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
						gets(genre[count]);
						printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						scanf("%d", &story_score[count]);
						while(story_score[count]>10 || story_score[count]<0) {
								printf("0~10사이의 정수의 값을 다시 입력해주세요!");
								scanf("%d", &story_score[count]);
						}
//						for(i=0; i<story_score[count]; i++) {
//								strcat(story_star[count], "★");
//						}
//						for(i=0; i<10-story_score[count]; i++) {
//								strcat(story_star[count], "☆");
//						}
						printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						scanf("%d", &music_score[count]);
						while(music_score[count]>10 || music_score[count]<0) {
								printf("0~10사이의 정수의 값을 다시 입력해주세요!");
								scanf("%d", &music_score[count]);
						}
//						for(i=0; i<music_score[count]; i++) {
//								strcat(music_star[count], "★");
//						}
//						for(i=0; i<10-music_score[count]; i++) {
//								strcat(music_star[count], "☆");
//						}
						printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
						scanf("%d", &casting_score[count]);
						while(casting_score[count]>10 || casting_score[count]<0) {
								printf("0~10사이의 정수의 값을 다시 입력해주세요!");
								scanf("%d", &casting_score[count]);
						}
//						for(i=0; i<casting_score[count]; i++) {
//								strcat(casting_star[count], "★");
//						}
//						for(i=0; i<10-casting_score[count]; i++) {
//								strcat(casting_star[count], "☆");
//						}
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
								count ++;
//								FILE *fp = fopen("M_data.txt", "w");
//
//								for(i=0; i < count; i++) {
//										fputs(title[i], fp);
//										fputs(genre[i], fp);
//								}
//								fclose(fp);
						}
						else if(user_choice == 2) {	
						}	
				}
				else if(user_choice == 2) {
						for(i=0; i<count; i++) {
								printf("제목 : %s\t,장르 : %s\t,스토리점수 : %s\t,음악점수 : %s\t,그림체점수 : %s\t,스토리 : %s\t,느낀점 : %s\t\n",title[i],genre[i],star(pss, i),star(pms, i),star(pcs, i),story[i],review[i]);
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

char star(int* score,int count) {
	int i;
	char star[21] = {0,};
	for(i=0; i<score[count]; i++) {
		strcat(star, "★");
	}
	for(i=0; i<10-score[count]; i++) {
		strcat(star, "☆");
	}
	return star[21];
}
