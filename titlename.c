#include <stdio.h>
#include <string.h>

int main(void)
{
	char title[10][21];
	char genre[10][5];
	int story_score[10],music_score[10],casting_score[10];
	char story[10][101];
	char review[10][1001];
	char pic1[10][100];
	char pic2[10][100];
	char pic3[10][100];
	int user_choice;
	int i;
	int count=2;

	while(1) {
		printf("Memory Tracer에 오신것을 환영합니다.\n");
		printf("메뉴를 선택하세요.\n");
		printf("1. 새로운 감상문 입력하기\n");
		printf("2. 감상문목록 관리하기\n");
		printf("3. 추천 목록보기\n");
		printf("4. 종료하기\n");

		printf("당신의 선택은?\n");
		scanf("%d", &user_choice);

		if(user_choice == 1) {
			for(i=0; i<count; i++) {
				printf("제목을 입력하세요 (단, 한글 10자 이내) : ");
				fgets(title[i], 21, stdin);
				printf("장르를 입력하세요 (액션, SF, 멜로, 공포 중에서)) : ");
				scanf("%s", genre[i]);
				printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
				scanf("%d", &story_score[i]);
					while(story_score[i]>10) {
						printf("0~10사이의 정수의 값을 다시 입력해주세요!");
						scanf("%d", &story_score[i]);
					}
				printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : ");
				scanf("%d", &music_score[i]);
		                        while(music_score[i]>10) {
        		                        printf("0~10사이의 정수의 값을 다시 입력해주세요!");
                		                scanf("%d", &music_score[i]);
		                        }
				printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
				scanf("%d", &casting_score[i]);
        		                while(casting_score[i]>10) {
                		                printf("0~10사이의 정수의 값을 다시 입력해주세요!");
                		                scanf("%d", &casting_score[i]);
                		        }
				getchar();
				printf("스토리를 입력하세요 (단, 한글 50자 이내) : ");
				fgets(story[i], 101, stdin);
				printf("느낀점을 입력하세요 (단, 한글 500자 이내) : ");
				fgets(review[i], 1001, stdin);
			}
		}
		else if(user_choice == 2) {
			for(i=0; i<count; i++) {
				printf("제목 :  %s\t,장르 : %s\t,스토리점수 : %d\t,음악점수 : %d\t,그림체점수 : %d\t,스토리 : %s\t,느낀점 : %s\t\n",title[i],genre[i],story_score[i],music_score[i],casting_score[i],story[i],review[i]);
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
