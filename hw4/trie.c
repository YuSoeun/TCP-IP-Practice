#include <stdio.h>
#include <stdlib.h>
#include "trie.h"

Trie * getNewTrie()
{
    Trie* trie = (Trie *)malloc(sizeof(Trie));
    trie->root = getNewNode(' ');

    return trie;
}

Node * getNewNode(char c)
{
    Node * node = (Node*)malloc(sizeof(Node));
    node->isTerminal = 0;
    node->childNum = 0;
    node->val = c;

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

void insert(Trie *trie, char* str)
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
    
    cur->isTerminal = 1;
}

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
int deletion(Node *cur, char* str)
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