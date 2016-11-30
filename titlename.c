#include <stdio.h>

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
	int count=2;

//	title = "코드기어스";
	strcpy(title[0], "코드기어스");
//	genre = "S/F";
	strcpy(genre[0], "S/F");
	story_score[0] = 10;
	music_score[0] = 8;
	casting_score[0] = 9;
//	story = "루루슈 킹왕짱 개쌤 기아스";
	strcpy(story[0], "루루슈 킹왕짱 개쌤 기아스");
//	review = "우리 형이 좋아해요.";
	strcpy(review[0], "우리 형이 좋아해요.");
//	pic1 = "image/image1.png";
//	pic2 = "image/image2.png";
//	pic3 = "image/image3.png";

	
        strcpy(title[1], "슈타인즈게이트");
//      genre = "S/F";
        strcpy(genre[1], "S/F");
        story_score[1] = 10;
        music_score[1] = 7;
        casting_score[1] = 3;
//      story = "루루슈 킹왕짱 개쌤 기아스";
        strcpy(story[1], "0과1");
//      review = "우리 형이 좋아해요.";
        strcpy(review[1], "우리 형이 좋아해요2.");

	int i;

	for(i=0; i<count; i++) {
		printf("제목 :  %s\t,장르 : %s\t,스토리점수 : %d\t,음악점수 : %d\t,그림체점수 : %d\t,스토리 : %s\t,느낀점 : %s\t\n",title[i],genre[i],story_score[i],music_score[i],casting_score[i],story[i],review[i]);
	}

	return 0;
}
