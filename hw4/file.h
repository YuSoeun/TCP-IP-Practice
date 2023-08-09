#ifndef __FILE_H__
#define __FILE_H__
#include "trie.h"

char** split(char* str, const char* delimiter, int* count);
void openFileAndSaveTrie(char filename[WORD_SIZE], Trie* trie);

#endif