#include <stdio.h>
#include <string.h>

int main(void)
{
	char title[10][21];
	char genre[10][5];
	int story_score[10];
	int music_score[10];
	int casting_score[10];
	char story[10][101];
	char review[10][1001];
	char pic1[10][100];
	char pic2[10][100];
	char pic3[10][100];
	int i;
	int count=2;

	for(i=0; i<count; i++) {
		printf("제목을 입력하세요 (단, 한글 10자 이내) : ");
		fgets(title[i], 21, stdin);
		printf("장르를 입력하세요 (액션, SF, 멜로, 공포 중에서)) : ");
		scanf("%s", genre[i]);
		printf("스토리 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
		scanf("%d", &story_score[i]);
	        printf("음악 점수를를 입력하세요 (단, 0~10까지의 정수 입력) : ");
		scanf("%d", &music_score[i]);
		printf("캐스팅 점수를 입력하세요 (단, 0~10까지의 정수 입력) : ");
		scanf("%d", &casting_score[i]);
		getchar();
		printf("스토리를 입력하세요 (단, 한글 50자 이내) : ");
		fgets(story[i], 101, stdin);
		printf("느낀점을 입력하세요 (단, 한글 500자 이내) : ");
		fgets(review[i], 1001, stdin);
	}

	//strcpy(title[0], "코드기어스");
	//strcpy(genre[0], "S/F");
	//story_score[0] = 10;
	//music_score[0] = 8;
	//casting_score[0] = 9;
	//strcpy(story[0], "루루슈 킹왕짱 개쌤 기아스");
	//strcpy(review[0], "우리 형이 좋아해요.");
//	pic1 = "image/image1.png";
//	pic2 = "image/image2.png";
//	pic3 = "image/image3.png";
	
        //strcpy(title[1], "슈타인즈게이트");
        //strcpy(genre[1], "S/F");
        //story_score[1] = 10;
        //music_score[1] = 7;
        //casting_score[1] = 3;
        //strcpy(story[1], "0과1");
        //strcpy(review[1], "우리 형이 좋아해요2.");

	for(i=0; i<count; i++) {
		printf("제목 :  %s\t,장르 : %s\t,스토리점수 : %d\t,음악점수 : %d\t,그림체점수 : %d\t,스토리 : %s\t,느낀점 : %s\t\n",title[i],genre[i],story_score[i],music_score[i],casting_score[i],story[i],review[i]);
	}

	return 0;
}
