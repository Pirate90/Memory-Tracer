#include <stdio.h>

int main(void)
{
	char title[21];
	char genre[5];
	int story_score;
	int music_score;
	int casting_score;
	char story[101];
	char review[1001];
	char pic1[100];
	char pic2[100];
	char pic3[100];

//	title = "코드기어스";
	strcpy(title, "코드기어스");
//	genre = "S/F";
	strcpy(genre, "S/F");
	story_score = 10;
	music_score = 8;
	casting_score = 9;
//	story = "루루슈 킹왕짱 개쌤 기아스";
	strcpy(story, "루루슈 킹왕짱 개쌤 기아스");
//	review = "우리 형이 좋아해요.";
	strcpy(review, "우리 형이 좋아해요.");
//	pic1 = "image/image1.png";
//	pic2 = "image/image2.png";
//	pic3 = "image/image3.png";

	printf("제목 :  %s\t,장르 : %s\t,스토리점수 : %d\t,음악점수 : %d\t,그림체점수 : %d\t,스토리 : %s\t,느낀점 : %s\t",title,genre,story_score,music_score,casting_score,story,review);

	return 0;
}
