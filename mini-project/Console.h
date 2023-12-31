/*
	Console.h: declarations for console-related functions
*/
#ifndef	__CONSOL__
#define	__CONSOL__

void clrscr(void);              // clear screen
void clrline(int y);            // clear line
void gotoxy(int x, int y);     	// move cursor to (x, y) coordinate

int getWindowWidth();       	// get width of current console window
int getWindowHeight();      	// get width of current console window

void DrawLine(int x1, int y1, int x2, int y2, char c);
void DrawBorderLine(char c1, char c2, char y1);
void swap(int *pa, int *pb);

void EnableCursor(int enable);
void swap(int *pa, int *pb);

void MySleep(int msec);
void MyPause();

int kbhit();
int getch();

#endif	//	__CONSOL__