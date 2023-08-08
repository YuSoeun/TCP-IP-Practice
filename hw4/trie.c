#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "trie.h"

Trie* getNewTrie()
{
    Trie* trie = (Trie *)malloc(sizeof(Trie));
    trie->root = getNewNode(' ');
    trie->rslt_cnt = 0;

    return trie;
}

Node* getNewNode(char c)
{
    Node* node = (Node *)malloc(sizeof(Node));
    node->isTerminal = 0;
    node->childNum = 0;
    node->val = c;
    node->search_cnt = 0;

    for (int i = 0; i < ALPHABET_SIZE; i++) {
        node->child[i] = 0;
    }

    return node;
}

int charToInt(char c)
{
    return c - 'a';
}

char intToChar(int i)
{
    return i + 'a';
}

void insert(Trie* trie, char* str, int search_cnt)
{
    Node* cur = trie->root;

    while (*str != 0){
        int index = charToInt(*str); // 문자열 일부 숫자로 변환
        
        // child 새로 추가
        if (cur->child[index] == 0) {
            cur->child[index] = getNewNode(*str);
            cur->childNum++;
        }
        cur = cur->child[index];
        str++;
    }
    cur->search_cnt = search_cnt;
    cur->isTerminal = 1;
}

char** result;
int result_cnt;

/* str을 포함하는 모든 문자열 list 반환 */
char** getStringsContainChar(Trie* trie, char* str)
{
    char word[BUF_SIZE] = {0};
    result_cnt = 0;

    result = (char **)malloc((int)sizeof(char *) * WORD_CNT);
    for (int i = 0; i < WORD_CNT; i++) {
        result[i] = (char *)malloc(BUF_SIZE);
    }

    traverseAndFindChar(trie->root, str, word, 0);
    trie->rslt_cnt = result_cnt;

    return result;
}

void traverseAndFindChar(Node* cur, char* str, char* cur_word, int isContain)
{
    int index = charToInt(*str);
   
    // printf("str: %c, cur: %c\n", *str, cur->val);

    if (*str == 0) 
        isContain = 1;

    if (isContain && cur->isTerminal) {
        // printf("cpy: %s\n", cur_word);
        memcpy(result[result_cnt], cur_word, BUF_SIZE);
        result_cnt++;
    }

    if (cur->childNum == 0)
        return;
    
    for (int i = 0; i < ALPHABET_SIZE; i++) {
        if (cur->child[i] != 0) {
            if (i == index && !isContain) {
                strncat(cur_word, &(cur->child[i]->val), 1);
                traverseAndFindChar(cur->child[i], str + 1, cur_word, isContain);
            } else {
                if (isContain)
                    strncat(cur_word, &(cur->child[i]->val), 1);
                traverseAndFindChar(cur->child[i], str, cur_word, isContain);
            }
        }
    }

    return;
}

/* str을 포함하는 문자 list 반환 */
int search(Trie* trie, char * str)
{
    Node* cur = trie->root;

    if (cur->childNum == 0)
        return 0;
    
    while (*str != 0){
        int index = charToInt(*str);

        if (cur->child[index] == 0) {
            return 0;
        }
        cur = cur->child[index];
        str++;
    }
    
    return cur->isTerminal;
}       

// Trie에서 문자열을 삭제하는 재귀 함수 0: 부모 node 삭제, 1: 부모삭제 X
int deletion(Node* cur, char* str)
{
    int index = charToInt(*str);
    if (cur->childNum == 0)
        return 0;

    // 문자열의 끝에 도달하지 않은 경우
    if (*str != 0) {
        // 문자열이 1을 반환하면 현재 노드를 삭제
        if (cur != 0 && cur->child[index] != 0 &&
                    deletion(cur->child[index], str + 1) &&
                    cur->isTerminal == 0) {
            if (cur->childNum == 0) {
                free(cur);
                cur = 0;
                return 1;
            }
            else {
                return 0;
            }
        }
    }

    // 문자열의 끝에 도달한 경우
    if (*str == 0 && cur->isTerminal) {
        // 현재 노드가 리프 노드이고 자식이 없는 경우
        if (cur->childNum == 0) {
            free(cur);
            cur = 0;
            return 1;
        }
 
        // 현재 노드가 리프 노드이고 자식이 있는 경우
        else {
            cur->isTerminal = 0;
            return 0;
        }
    }
 
    return 0;
}