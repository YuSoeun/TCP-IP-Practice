/*
    자동 완성 기능 기반 Search Engine 구현
    1. 서버의 Listen 포트 번호와 검색어 데이터베이스 파일을 command line argument로 넣어 프로그램을 실행
       - 서버는 Multi-Thread로 구현
     $ ./search_server 9090 data.txt
    2. 파일로부터 검색어와 검색어의 검색 횟수를 읽어 온다. Pohang 30000
    3. Client에서 검색어 입력
       - 연관 검색어 검색 횟수에 대해 정렬하여 실시간으로 표시 (Trie DS 사용, 최대 10개)
       - TCP nodelay option 줘보기
       - 검색어가 중간에 나오거나 끝에 나오는 경우 고려 (kmp algorithim)
    4. 검색어에 해당되는 부분은 임의의 색깔
*/

#include "Console.h"

#define BUF_SIZE 1024

int main()
{
    char alpha;
    char search_word[BUF_SIZE] = {};
    int len = 0;

    clrscr();

    while (1) {
        EnableCursor(0);
        DrawBorderLine('-', '|', 5);
        gotoxy(2, 4);
        printf("연관 검색어 List");
        
        EnableCursor(1);
        gotoxy(1, 1);
        printf("\033[38;2;159;75;153mSearch Word: ");

        // 사용자에게 char 입력
        gotoxy(14, 1);
        printf("\033[38;2;255;255;255m%s", search_word);
        alpha = getch();
        clrscr();
        printf("%c", alpha);
        
        if (alpha == 127 || alpha == 8) {
            len--;
        } else { 
            search_word[len++] = alpha;
        }
        search_word[len] = 0;
        gotoxy(3, 6);
        printf("word: %s", search_word);

        // TODO: sever에게 검색어 보내기
    }

    return 0;
}