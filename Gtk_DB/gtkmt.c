#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdio.h>
#include <time.h>
#include <gtk/gtk.h>

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
#define OP_SEND_REVIEW 1
#define OP_RECEIVE_INPUT_REVIEW 2
#define OP_RECEIVE_EDIT_REVIEW 3
#define OP_RECEIVE_DEL_REVIEW 4
#define OP_DESC_SORT 5
#define OP_ASC_SORT 6
#define OP_SAME 7
#define DIV 10
#define DIV2 100

GtkBuilder* gtkBuilder = NULL;

typedef struct REVIEW {
	unsigned int id;
	char title[TITLE_BYTE];
	char genre[GENRE_BYTE];
	int story_score,music_score,casting_score;
	char story[STORY_BYTE];
	char review[REVIEW_BYTE];
	char pic[5][100];
	char timer[20];
} REVIEW;

typedef struct REVIEW_TABLE {
	int count;
	struct REVIEW* t;
} REVIEW_TABLE;

typedef struct REQUEST {
	int op_code;
	char arg[80];
} REQUEST;

void print_reviewtable(REVIEW_TABLE* rt);
void print_selectreview(REVIEW* a);
void print_star(int score);
int count_korean_char(char* s);
int format_width_count(char* s, int width);
REVIEW* fc(REVIEW_TABLE* rt);
REVIEW* fi(REVIEW_TABLE* rt, int index);
void send_REVIEW_TABLE (int sockfd, REVIEW_TABLE* rt);
void receive_REVIEW_TABLE (int sockfd, REVIEW_TABLE* rt);
int check_subchar(char* str);
void remove_all_child (GtkContainer *container);


const char user_criteria[7][20] = {"번호", "제목", "장르", "스토리점수", "음악점수", "캐스팅점수", "저장날짜"};

int sel_m_a = 0;
int sockfd;

int main(int argc, char* argv[]) {
	GtkWidget  *window;

	struct sockaddr_in servaddr;

	sockfd = socket(AF_INET, SOCK_STREAM, 0);

	memset(&servaddr, 0, sizeof(servaddr));
	servaddr.sin_addr.s_addr = inet_addr(argv[1]);
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(atoi(argv[2]));

	connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));

    gtk_init(&argc, &argv);
    gtkBuilder = gtk_builder_new();
    gtk_builder_add_from_file(gtkBuilder, "mt_main.ui", NULL);
    window = GTK_WIDGET( gtk_builder_get_object( gtkBuilder, "window") );
    gtk_builder_connect_signals( gtkBuilder, NULL);

    gtk_widget_show(window);
    gtk_main();

    return 0;
}

G_MODULE_EXPORT void m_a_togglebutton_toggled (GtkToggleButton *togglebutton) { //메인화면에서 영화, 애니메이션 가르는 토글버튼 클릭
	GtkToggleButton* m_toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, "m_toggle"));
	GtkToggleButton* a_toggle = GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, "a_toggle"));
	GtkToggleButton* another = (togglebutton == m_toggle)?a_toggle:m_toggle;
	int cur = (togglebutton == m_toggle)?0:1;

	if(sel_m_a == cur) {
		gtk_toggle_button_set_active(togglebutton, TRUE);
	 	return;
	}

	sel_m_a = (cur == 0)?!gtk_toggle_button_get_active(togglebutton):gtk_toggle_button_get_active(togglebutton);

	if(gtk_toggle_button_get_active(togglebutton) ==  gtk_toggle_button_get_active(another))	
		gtk_toggle_button_set_active(another, !gtk_toggle_button_get_active(another));
	g_print("cur: %d\n", sel_m_a);
}

G_MODULE_EXPORT void return_main_page () { //메인화면으로 가는 버튼
	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "main_fixed")));
}

G_MODULE_EXPORT void return_table_page () { //테이블화면으로 가는 버튼
	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "table_fixed")));
}

G_MODULE_EXPORT void input_button_clicked () { //메인화면에서 input버튼 클릭할때
	GtkTextBuffer *buffer = NULL;

	gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "title_input_entry")), "");
	gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "genre_input_entry")), "");
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, "sscore_rb0")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, "mscore_rb0")), TRUE);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, "cscore_rb0")), TRUE);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "story_input_textview")), buffer);
	gtk_text_view_set_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "review_input_textview")), buffer);

	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "input_grid")));
}

G_MODULE_EXPORT void save_input_button_clicked () { //input화면에서 저장버튼을 눌렀을떄
	REQUEST req;
	REVIEW temp;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	char str[20];
	int i;

	temp.id = -1;
	const gchar *text1 = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "title_input_entry")));
	strcpy(temp.title, text1);
	const gchar *text2 = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "genre_input_entry")));
	strcpy(temp.genre, text2);
	for (i=0; i<11; i++) {
		sprintf(str, "sscore_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.story_score = i;
			break;
		}
	}
	for (i=0; i<11; i++) {
		sprintf(str, "mscore_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.music_score = i;
			break;
		}
	}
	for (i=0; i<11; i++) {
		sprintf(str, "cscore_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.casting_score = i;
			break;
		}
	}
	buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "story_input_textview"))));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	const gchar *text3 = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	strcpy(temp.story, text3);
	buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "review_input_textview"))));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	const gchar *text4 = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	strcpy(temp.review, text4);
	temp.timer[0] = '\0';

	req.op_code = (OP_RECEIVE_INPUT_REVIEW*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	write(sockfd, &temp, sizeof(REVIEW));

	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "main_fixed")));
}

G_MODULE_EXPORT void table_button_clicked () { //메인화면에서 table버튼 클릭할때
	REVIEW_TABLE rt;
	REQUEST req;
	char buf[200];
	int i;

	req.op_code = (OP_SEND_REVIEW_TABLE*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	receive_REVIEW_TABLE (sockfd, &rt);
	REVIEW* a = rt.t;

	print_reviewtable(&rt);

    GtkListBox* table = GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox"));
    remove_all_child (GTK_CONTAINER(table));
    GtkWidget* temp;

    for (i=0; i<rt.count; i++) {
		sprintf(buf, "%d | %s | %s | %d | %d | %d | %s", a[i].id, a[i].title, a[i].genre, a[i].story_score, a[i].music_score, a[i].casting_score, a[i].timer);
		temp = gtk_label_new(buf);
		gtk_list_box_insert(table, temp, i);
		gtk_widget_show(temp);
		if (i == 0) gtk_list_box_select_row (table, GTK_LIST_BOX_ROW (gtk_widget_get_parent(temp)));
    }

	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "table_fixed")));
}

G_MODULE_EXPORT void sort_desc (GtkButton *button) { //내림차순 정렬
	REVIEW_TABLE rt;
	REVIEW_TABLE* sel = &rt;
	REQUEST req;
	char buf[200];
	int i;
	int user_choice;

	if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_id_desc"))) {
		user_choice = 0;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_title_desc"))) {
		user_choice = 1;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_genre_desc"))) {
		user_choice = 2;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_sscore_desc"))) {
		user_choice = 3;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_mscore_desc"))) {
		user_choice = 4;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_cscore_desc"))) {
		user_choice = 5;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_timer_desc"))) {
		user_choice = 6;
	}

 	req.op_code = (OP_DESC_SORT*DIV) + user_choice + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	receive_REVIEW_TABLE (sockfd, sel);

	REVIEW* a = rt.t;

    GtkListBox* table = GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox"));
    remove_all_child (GTK_CONTAINER(table));
    GtkWidget* temp;

    for (i=0; i<rt.count; i++) {
    	sprintf(buf, "%d | %s | %s | %d | %d | %d | %s", a[i].id, a[i].title, a[i].genre, a[i].story_score, a[i].music_score, a[i].casting_score, a[i].timer);
		temp = gtk_label_new(buf);
		gtk_list_box_insert(table, temp, i);
		gtk_widget_show(temp);
		if (i == 0) gtk_list_box_select_row (table, GTK_LIST_BOX_ROW (gtk_widget_get_parent(temp)));
    }
}

G_MODULE_EXPORT void sort_asc (GtkButton *button) { //오름차순 정렬
	REVIEW_TABLE rt;
	REVIEW_TABLE* sel = &rt;
	REQUEST req;
	char buf[200];
	int i;
	int user_choice;

	if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_id_asc"))) {
		user_choice = 0;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_title_asc"))) {
		user_choice = 1;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_genre_asc"))) {
		user_choice = 2;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_sscore_asc"))) {
		user_choice = 3;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_mscore_asc"))) {
		user_choice = 4;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_cscore_asc"))) {
		user_choice = 5;
	}
	else if (button == GTK_BUTTON(gtk_builder_get_object(gtkBuilder, "sort_timer_asc"))) {
		user_choice = 6;
	}

 	req.op_code = (OP_ASC_SORT*DIV) + user_choice + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	receive_REVIEW_TABLE (sockfd, sel);

	REVIEW* a = rt.t;

    GtkListBox* table = GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox"));
    remove_all_child (GTK_CONTAINER(table));
    GtkWidget* temp;

    for (i=0; i<rt.count; i++) {
    	sprintf(buf, "%d | %s | %s | %d | %d | %d | %s", a[i].id, a[i].title, a[i].genre, a[i].story_score, a[i].music_score, a[i].casting_score, a[i].timer);
		temp = gtk_label_new(buf);
		gtk_list_box_insert(table, temp, i);
		gtk_widget_show(temp);
		if (i == 0) gtk_list_box_select_row (table, GTK_LIST_BOX_ROW (gtk_widget_get_parent(temp)));
    }
}

G_MODULE_EXPORT void table_view_button () { //테이블화면에서 자세히보기 버튼 클릭할때
	REVIEW temp;
	REQUEST req;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	char buf[200];
	int select_review_id;

	GtkLabel* label = GTK_LABEL(gtk_container_get_children(GTK_CONTAINER(gtk_list_box_get_selected_row (GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox")))))->data);
	select_review_id = atoi(gtk_label_get_text(label));

	memcpy(req.arg, &select_review_id, sizeof(int));

	req.op_code = (OP_SEND_REVIEW*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	read(sockfd, &temp, sizeof(REVIEW));

	print_selectreview(&temp);

	sprintf(buf, "%d", temp.id);

	gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "id_view_label2")), buf);
	gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "title_view_label2")), temp.title);
	gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "genre_view_label2")), temp.genre);
	sprintf(buf, "%d", temp.story_score);	
	gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "sscore_view_label2")), buf);
	sprintf(buf, "%d", temp.music_score);
	gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "mscore_view_label2")), buf);
	sprintf(buf, "%d", temp.casting_score);
	gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "cscore_view_label2")), buf);

	buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, temp.story, strlen(temp.story));
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "story_view_textview")), buffer);
	g_object_unref(buffer);
	
	buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, temp.review, strlen(temp.review));
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "review_view_textview")), buffer);
	g_object_unref(buffer);

	gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "savetime_view_label2")), temp.timer);
	
	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "view_fixed")));
}

G_MODULE_EXPORT void table_edit_button () { //테이블화면에서 편집하기 버튼 클릭할때
	REVIEW temp;
	REQUEST req;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	char buf[200];
	int select_review_id;
	char str[20];
	int i;

	GtkLabel* label = GTK_LABEL(gtk_container_get_children(GTK_CONTAINER(gtk_list_box_get_selected_row (GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox")))))->data);
	select_review_id = atoi(gtk_label_get_text(label));

	memcpy(req.arg, &select_review_id, sizeof(int));

	req.op_code = (OP_SEND_REVIEW*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	read(sockfd, &temp, sizeof(REVIEW));

	print_selectreview(&temp);

	gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "title_edit_entry")), temp.title);
	gtk_entry_set_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "genre_edit_entry")), temp.genre);
	for (i=0; i<11; i++) {
		sprintf(str, "sscore_edit_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.story_score = i;
			break;
		}
	}
	sprintf(str, "sscore_edit_rb%d", temp.story_score);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str)), TRUE);
	for (i=0; i<11; i++) {
		sprintf(str, "mscore_edit_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.music_score = i;
			break;
		}
	}
	sprintf(str, "mscore_edit_rb%d", temp.music_score);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str)), TRUE);
	for (i=0; i<11; i++) {
		sprintf(str, "cscore_edit_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.casting_score = i;
			break;
		}
	}
	sprintf(str, "cscore_edit_rb%d", temp.casting_score);
	gtk_toggle_button_set_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str)), TRUE);

	buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, temp.story, strlen(temp.story));
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "story_edit_textview")), buffer);
	g_object_unref(buffer);
	
	buffer = gtk_text_buffer_new(NULL);
	gtk_text_buffer_set_text(buffer, temp.review, strlen(temp.review));
	gtk_text_view_set_buffer (GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "review_edit_textview")), buffer);
	g_object_unref(buffer);

	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "edit_grid")));
}

G_MODULE_EXPORT void save_edit_button_clicked () { //편집화면에서 저장버튼 클릭할떄
	REVIEW_TABLE rt;
	REQUEST req;
	REVIEW temp;
	GtkTextIter start, end;
	GtkTextBuffer *buffer;
	char buf[200];
	char str[20];
	int i;
	int select_review_id;

	GtkLabel* label = GTK_LABEL(gtk_container_get_children(GTK_CONTAINER(gtk_list_box_get_selected_row (GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox")))))->data);
	select_review_id = atoi(gtk_label_get_text(label));

	memcpy(req.arg, &select_review_id, sizeof(int));

	temp.id = select_review_id;
	const gchar *text1 = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "title_edit_entry")));
	strcpy(temp.title, text1);
	const gchar *text2 = gtk_entry_get_text(GTK_ENTRY(gtk_builder_get_object(gtkBuilder, "genre_edit_entry")));
	strcpy(temp.genre, text2);
	for (i=0; i<11; i++) {
		sprintf(str, "sscore_edit_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.story_score = i;
			break;
		}
	}
	for (i=0; i<11; i++) {
		sprintf(str, "mscore_edit_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.music_score = i;
			break;
		}
	}
	for (i=0; i<11; i++) {
		sprintf(str, "cscore_edit_rb%d", i);
		if (gtk_toggle_button_get_active(GTK_TOGGLE_BUTTON(gtk_builder_get_object(gtkBuilder, str))) == TRUE) {
			temp.casting_score = i;
			break;
		}
	}
	buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "story_edit_textview"))));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	const gchar *text3 = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	strcpy(temp.story, text3);
	buffer = GTK_TEXT_BUFFER(gtk_text_view_get_buffer(GTK_TEXT_VIEW(gtk_builder_get_object(gtkBuilder, "review_edit_textview"))));
	gtk_text_buffer_get_bounds (buffer, &start, &end);
	const gchar *text4 = gtk_text_buffer_get_text (buffer, &start, &end, FALSE);
	strcpy(temp.review, text4);
	temp.timer[0] = '\0';

	req.op_code = (OP_RECEIVE_EDIT_REVIEW*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	write(sockfd, &temp, sizeof(REVIEW));

	req.op_code = (OP_SEND_REVIEW_TABLE*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	receive_REVIEW_TABLE (sockfd, &rt);
	REVIEW* a = rt.t;

	print_reviewtable(&rt);

	GtkListBox* table = GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox"));
    remove_all_child (GTK_CONTAINER(table));
    GtkWidget* w_temp;

    for (i=0; i<rt.count; i++) {
		sprintf(buf, "%d | %s | %s | %d | %d | %d | %s", a[i].id, a[i].title, a[i].genre, a[i].story_score, a[i].music_score, a[i].casting_score, a[i].timer);
		w_temp = gtk_label_new(buf);
		gtk_list_box_insert(table, w_temp, i);
		gtk_widget_show(w_temp);
		if (i == 0) gtk_list_box_select_row (table, GTK_LIST_BOX_ROW (gtk_widget_get_parent(w_temp)));
    }

	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "table_fixed")));
}

G_MODULE_EXPORT void table_del_button () {
	REVIEW_TABLE rt;
	REVIEW temp;
	REQUEST req;
	char buf[200];
	int select_review_id;
	int i;

	GtkLabel* label = GTK_LABEL(gtk_container_get_children(GTK_CONTAINER(gtk_list_box_get_selected_row (GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox")))))->data);
	select_review_id = atoi(gtk_label_get_text(label));

	memcpy(req.arg, &select_review_id, sizeof(int));

	// sprintf(buf, "%s번 감상문을 지우시겠습니까?", req.arg);
	// gtk_label_set_label (GTK_LABEL(gtk_builder_get_object(gtkBuilder, "del_dialog_label")), buf);

	// GtkDialog* dialog = GTK_DIALOG(gtk_builder_get_object(gtkBuilder, "del_dialog"));

	// gtk_widget_show(dialog);

	req.op_code = (OP_RECEIVE_DEL_REVIEW*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	write(sockfd, &temp, sizeof(REVIEW));

	req.op_code = (OP_SEND_REVIEW_TABLE*DIV) + (sel_m_a*DIV2);
	write(sockfd, &req, sizeof(req));
	receive_REVIEW_TABLE (sockfd, &rt);
	REVIEW* a = rt.t;

	print_reviewtable(&rt);

	GtkListBox* table = GTK_LIST_BOX(gtk_builder_get_object(gtkBuilder, "review_table_listbox"));
    remove_all_child (GTK_CONTAINER(table));
    GtkWidget* w_temp;

    for (i=0; i<rt.count; i++) {
		sprintf(buf, "%d | %s | %s | %d | %d | %d | %s", a[i].id, a[i].title, a[i].genre, a[i].story_score, a[i].music_score, a[i].casting_score, a[i].timer);
		w_temp = gtk_label_new(buf);
		gtk_list_box_insert(table, w_temp, i);
		gtk_widget_show(w_temp);
		if (i == 0) gtk_list_box_select_row (table, GTK_LIST_BOX_ROW (gtk_widget_get_parent(w_temp)));
    }

	gtk_stack_set_visible_child(GTK_STACK(gtk_builder_get_object(gtkBuilder, "stack")), GTK_WIDGET(gtk_builder_get_object(gtkBuilder, "table_fixed")));
}

G_MODULE_EXPORT void recom_button_clicked () { //메인화면에서 추천버튼 클릭할때

}

G_MODULE_EXPORT void quit_button_clicked () { //메인화면에서 종료버튼 클릭할때
    g_object_unref(G_OBJECT(gtkBuilder));
    gtk_main_quit ();
}

// 					else if (user_choice == 2) {
// 						printf("같은 값으로 모아볼 항목을 적어주세요.\n");
// 						printf("1. 장르\n2. 스토리점수\n3. 음악점수\n4. 캐스팅점수\n");
// 						get_input_range(&user_choice, 4, 1);
// 						req.op_code = (OP_SAME*DIV) + user_choice - 1 + sel_m_a;
// 						printf("모아 볼 %s을(를) 적어주세요.\n", user_criteria[user_choice + 1]);
// 						scanf("%s", req.arg);
// 						write(sockfd, &req, sizeof(req));
// 						receive_REVIEW_TABLE (sockfd, sel);
// 						print_reviewtable(sel);
// 						free(sel->t);

void print_reviewtable(REVIEW_TABLE* rt) { //감상문테이블을 화면으로 출력하는 함수
	int i;
	struct tm* t;
	char buf[100];
	int count = rt->count;
	REVIEW* a = rt->t;

	g_print("\n|%-8s|%-8s|%-22s|%-10s|%-15s|%-14s|%-15s|%s\n","순서","번호","제목","장르","스토리점수","음악점수","캐스팅점수","저장날짜");

	for(i=0; i<count; i++) {
		sprintf(buf, "|%%-6d|%%-6d|%%-%ds|%%-%ds|", format_width_count(a[i].title, KOREAN_WIDTH * TITLE_LENGTH), format_width_count(a[i].genre, KOREAN_WIDTH * GENRE_LENGTH));
		g_print(buf, i+1, a[i].id,a[i].title,a[i].genre);
		print_star(a[i].story_score);
		g_print("|");
		print_star(a[i].music_score);
		g_print("|");
		print_star(a[i].casting_score);
		g_print("|");
		g_print("%s\n", a[i].timer);
	}
	g_print("\n");
}

void print_selectreview(REVIEW* a) { //특정감상문의 자세한 내용을 화면으로 출력하는 함수
	g_print("\n|번호 : %d\n", a->id);
	g_print("|제목 : %s\n|장르 : %s\n",a->title,a->genre);
	g_print("|스토리점수 : ");
	print_star(a->story_score);
	g_print("\n|음악점수   : ");
	print_star(a->music_score);
	g_print("\n|캐스팅점수 : ");
	print_star(a->casting_score);
	g_print("\n|스토리 : %s|느낀점 : %s",a->story,a->review);
	g_print("|저장날짜 : %s\n\n", a->timer);
}

void print_star(int score) { //점수를 별표로 변환하여 화면으로 출력하는 함수
	int i;
	for(i=0; i<score; i++)
		g_print ("★");
	for(i=0; i<10-score; i++)
		g_print ("☆");
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

int check_subchar(char* str) {
	int length = strlen(str);
	int i;	
	if((str[length-1] & 0xC0) != 0x80)
		return -1; //한글 아닌 경우
	else if((str[length-3] & 0xF0) == 0xE0 && (str[length-2] & 0xC0) == 0x80) {
		//한글
		int kor_char = ((((int)str[length-3] & 0x0F) << 12) | (((int)str[length-2] & 0x3F) << 6) | ((int)str[length-1] & 0x3F)) - 0xAC00;
		return kor_char % 28;	//받침 있는 경우 0이 아닌 값, 받침 없는 경우 0 출력
	}

	return -2;	//한글과 영어 둘다 아닌듯
}

void remove_all_child (GtkContainer *container){
	GList *children, *iter;

	children = gtk_container_get_children(container);
	for(iter = children; iter != NULL; iter = g_list_next(iter))
	  gtk_widget_destroy(GTK_WIDGET(iter->data));
	g_list_free(children);
}
