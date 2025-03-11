#include <stdio.h>
#include <ctype.h>
#include <string.h>
#include "trim.h"

/* trim_left 함수: 문자열의 왼쪽 공백을 제거 */
char* trim_left(char* str) {
    while (*str) {
        if (isspace(*str)) {
            str++;
        }
        else {
            break;
        }
    }
    return str;
}

/* trim_right 함수: 문자열의 오른쪽 공백을 제거 */
char* trim_right(char* str) {
    int len = (int)strlen(str) - 1;

    while (len >= 0) {
        if (isspace(*(str + len))) {
            len--;
        }
        else {
            break;
        }
    }
    *(str + ++len) = '\0';
    return str;
}

/* trim 함수: 문자열의 양쪽 공백을 제거 */
char* trim(char* str) {
    return trim_left(trim_right(str));
}