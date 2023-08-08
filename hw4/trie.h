/* 참고:
    https://hongjw1938.tistory.com/24 
    https://www.techiedelight.com/ko/trie-implementation-insert-search-delete/
*/

#ifndef __TRIE_H__
#define __TRIE_H__

#define ALPHABET_SIZE 26    // TODO: space는 예외로 27
#define BUF_SIZE 1024

typedef struct node
{
    struct node* child[ALPHABET_SIZE];  // 뒤로 연결되는 문자열 a-z 소문자를 index화하여 저장(26개)
    int isTerminal;                     // 현재 노드가 문자 완성이 되는 노드인지 여부 (0:false, 1:true)
    int childNum;                       // 현재 노드에 연결된 문자열의 개수
    char val;                           // 현재 노드의 값
} Node;

typedef struct trie
{
    Node* root;
    int size;
} Trie;

Trie * getNewTrie();
Node * getNewNode(char c);
int charToInt(char c);
char intToChar(int i);
void insert(Trie *trie, char* str);
int search(Trie* trie, char * str);
int deletion(Node *cur, char* str);

#endif