#include <sys/inotify.h>
#include <stdio.h>
#include <dirent.h>
#include <unistd.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include <libgen.h>

//設定每次read最多讀取1000個物件，這裡如果設定太小，可能會有「漏失」某些事件
#define BUF_LEN (1000 * (sizeof(struct inotify_event) + NAME_MAX + 1))
void printInotifyEvent(struct inotify_event *event);

//key-value的映射，陣列的形式，key就是第一個index
//最多可以映射1000個，value最多是10000個字母
char wd[1000][4000];
char str[4096]={0};

void printInotifyEvent(struct inotify_event *event)
{	
	int eatRet, k = 0;
	char buf[8192] = "./";
	
	//printf("@event = %p\n", event);
	//sprintf(buf, "來自[%s]的事件 ", wd[event->wd]);
	int l = strlen(buf);
	//底下是將所有的事件做檢查，照理說應該只會有一個事件
	//strncat(buf+strlen(buf), "{", 4095);
	if (event->mask & IN_ACCESS) {
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf , "ACCESS");
	}
	if (event->mask & IN_ATTRIB) {
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "ATTRIB");
	}
	if (event->mask & IN_CLOSE_WRITE){
		k = 1;
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "CLOSE_WRITE,CLOSE");
	}
	if (event->mask & IN_CLOSE_NOWRITE){
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "CLOSE_NOWRITE,CLOSE");
	}
	if (event->mask & IN_CREATE){
		k = 2;
		strcpy(str, event->name);
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "CREATE");
	}
	if (event->mask & IN_DELETE){
		k = 3;
		strcpy(str, event->name);
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "DELETE");
	}
	if (event->mask & IN_DELETE_SELF){
	 	k = 3;
	 	strcpy(str, event->name);
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "DELETE_SELF");
	}
	if (event->mask & IN_MODIFY)
	{
		k = 1;
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "MODIFY");
	}
	if (event->mask & IN_MOVE_SELF)
	{
		k = 2;
		strcpy(str, event->name);
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "MOVE_SELF");
	}
	if (event->mask & IN_MOVED_FROM)
	{
		k = 2;
		strcpy(str, event->name);
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "MOVED_FROM");
	}
	if (event->mask & IN_MOVED_TO)
	{
		k = 2;
		strcpy(str, event->name);
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "MOVED_TO");
	}
	if (event->mask & IN_OPEN)
	{
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "OPEN");
	}
	if (event->mask & IN_IGNORED)
	{
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "IGNORED");
	}
	if (event->mask & IN_ISDIR)
	{
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "ISDIR");
	}
	if (event->mask & IN_Q_OVERFLOW)
	{
		if(strlen(buf) != l) strcat(buf, ",");
		strcat(buf, "Q_OVERFLOW");
	}
	if (event->len > 0)
		eatRet = snprintf(buf + strlen(buf), 4095, " name = %s", event->name);
	else
		eatRet = snprintf(buf + strlen(buf), 4095, " name = null");
	printf("%s\n", buf);
	
	if( k == 1 ){ // CLOSE_WRITE & MODIFY  
		printf("程式碼發⽣異動，重新編譯\n");
		system("make");
	}
	else if( k == 2) { // CREATE & MOVE_TO & MOVE_FROM & MOVE_SELF
		printf("檔案異動, ");
		printf("將「%s」加入監聽，重新編譯\n", str);
		system("make");
	}
	else if( k == 3) { // DELETE & DELETE_SELF
		printf("檔案異動, ");
		printf("將「%s」刪除監聽, 重新編譯\n", str);
		system("make");
	}
}
int main(int argc, char **argv)
{
	//監聽的頻道
	int fd;
	int nRead, ret, i;
	char *eventPtr;
	char *inotify_entity = (char *)malloc(BUF_LEN);
	char Buf[4096] = {0};
	char *exename = basename("ls");
	strcat( Buf, exename );
	//跟作業系統要一個監聽專用的『頻道』，作業系統會幫我們建立一個檔案，
	//用這個檔案「送」資料給我們，並且自動開啟該「檔案/頻道」，並給它的fd
	fd = inotify_init();
	//設定在哪些檔案監聽哪些事件
	for (i = 1; i < argc; i++)
	{
		//inotify_add_watch，對檔案其路徑是『argv[i]』，監聽所有事件

		ret = inotify_add_watch(fd, argv[i], IN_ALL_EVENTS);
		if (ret == -1) {
			printf("argv[%d]=%s\n", i, argv[i]);
			perror("inotify_add_watch");
		} else {
			printf("監聽檔案 %s \n", argv[i]);
			chdir(argv[i]); //更改目前路徑
		}
		//這裡構成一個簡單的 key-value 的結構
		//key是 「watch  descriptor」， value是檔案名稱
		strcpy(wd[ret], argv[i]);
		//strcpy(goal, argv[i] );
	}

	strcat(Buf, " ");
	strcat(Buf, argv[i-1]);
	printf("開始監聽下列檔案：\n");
	system(Buf);// 使用ls將此觀測目錄下所有檔案印出
	//*/
	//使用一個while loop不斷地讀取 inotify_init() 所開啟的檔案 fd
	//fd 裏面就是我們要監聽的訊息
	while (1)
	{
		//一直讀取，作業系統開給我們的頻道，nRead是這次頻道中的資料量大小
		nRead = read(fd, inotify_entity, BUF_LEN);
		//底下的 for loop 不斷地將收進來的資料切割成『不定長度的』的 inotify_event
		printf("檔案異動訊息\n");
		for (eventPtr = inotify_entity; eventPtr < inotify_entity + nRead;)
		{	
			printInotifyEvent((struct inotify_event *)eventPtr);

			//目前這個物件的長度是 基本的inotiry_event的長度 ＋ name字串的長度
			//將eventPtr加上物件長度，就是下一個物件的開始位置
			eventPtr += sizeof(struct inotify_event) + ((struct inotify_event *)eventPtr)->len;
		}
	}
}
