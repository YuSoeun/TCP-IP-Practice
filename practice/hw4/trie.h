/* 참고:
    https://hongjw1938.tistory.com/24 
    https://www.techiedelight.com/ko/trie-implementation-insert-search-delete/
*/

#ifndef __TRIE_H__
#define __TRIE_H__

#define ALPHABET_SIZE 27
#define BUF_SIZE 1024
#define WORD_SIZE 512
#define WORD_CNT 100

typedef struct node
{
    struct node* child[ALPHABET_SIZE];  // 뒤로 연결되는 문자열 a-z + ' '를 index화하여 저장 (27개)
    int isTerminal;                     // 현재 노드가 문자 완성이 되는 노드인지 여부 (0:false, 1:true)
    int childNum;                       // 현재 노드에 연결된 문자열의 개수
    char val;                           // 현재 노드의 값
    int search_cnt;                     // search 횟수 값
} Node;

typedef struct trie
{
    Node* root;
    int rslt_cnt;
} Trie;

typedef struct result
{
    char word[WORD_CNT];
    int cnt;
} Result;

Trie * getNewTrie();
Node * getNewNode(char c);
int charToInt(char c);
char intToChar(int i);
void insert(Trie* trie, char* str, int search_cnt);
int search(Trie* trie, char * str);
int deletion(Node *cur, char* str);
Result ** getStringsContainChar(Trie* trie, char* str);
void traverseAndFindChar(Node* cur, char* str, char* cur_word, int isContain);  // private
int search(Trie* trie, char * str);

#endif