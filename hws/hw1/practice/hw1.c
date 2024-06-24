#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <ctype.h>
#include <fcntl.h>
#include <string.h>


int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "Wrong number of arguments\n");
        exit(EXIT_FAILURE);
    }
    int cache_size = atoi(*(argv + 1));
    if (cache_size <= 0) {
        fprintf(stderr, "ERROR: Invalid Cache Size\n");
        exit(EXIT_FAILURE);
    }
    char** cache = calloc(cache_size, sizeof(char*));
    if (cache == NULL) {
        fprintf(stderr, "ERROR: Could not allocate memory for Cache\n");
        exit(EXIT_FAILURE);
    }
    for (int i = 2; i < argc; i++) {
        int file = open(*(argv+i), O_RDONLY);
        if (file == -1) {
            fprintf(stderr, "ERROR: Error opening file\n");
            continue;
        } 
        char c;
        ssize_t bytesRead;
        int word_length = 0;
        char* new_word = NULL;
        while((bytesRead=read(file, &c, 1))>0) {
            if (c == ' ' || !isalnum(c)) {
               if (word_length > 0) {
                    new_word = realloc(new_word, word_length + 1);
                    if (new_word == NULL) {
                        fprintf(stderr, "ERROR: Memory Allocation Error\n");
                        exit(EXIT_FAILURE);
                    }
                    *(new_word + word_length) = '\0';
                    word_length = 0;  // Reset word_length for the next word
                    int index = hash(new_word, cache_size);
                    if (strlen(new_word) >= 3 && isalnum(*new_word)) {
                        // Remove the duplicate declaration of 'index' here
                        if (*(cache + index) == NULL) {
                            *(cache + index) = calloc(strlen(new_word) + 1, sizeof(char));

                            printf("Word \"%s\" ==> %d (calloc)\n", new_word, index);
                        } else if (strlen(*(cache + index)) == strlen(new_word)) {
                            printf("Word \"%s\" ==> %d (nop)\n", new_word, index);
                        }
                        
                        else {
                            *(cache + index) = realloc(*(cache + index), strlen(new_word) + 1);
                            printf("Word \"%s\" ==> %d (realloc)\n", new_word, index);
                        }
                        strcpy(*(cache + index), new_word);
                    }
                    free(new_word);
               }
            } else {
                if (word_length == 0) {
                    new_word = calloc(1, sizeof(char));
                    *(new_word + word_length) = c;
                    if (new_word == NULL) {
                        fprintf(stderr, "ERROR: Memory Allocation Error\n");
                        exit(EXIT_FAILURE);
                    }
                } else {
                    new_word = realloc(new_word, (word_length+1) *sizeof(char));
                    *(new_word + word_length) = c;
                    if (new_word == NULL) {
                        fprintf(stderr, "ERROR: Memory Allocation Error\n");
                        exit(EXIT_FAILURE);
                    }
                }
                word_length++;
            }
        }
        if (word_length > 0) {  // Only construct new_word if word_length > 0
            new_word = realloc(new_word, word_length + 1);
            if (new_word == NULL) {
                fprintf(stderr, "ERROR: Memory allocation failed\n");
                return 1; // Or handle the error as needed
            }
            *(new_word+word_length) = '\0';
            word_length = 0;  // Reset word_length for the next word
            int index = hash(new_word, cache_size);
            if (strlen(new_word) >= 3 && isalnum(*new_word)) {
                // Remove the duplicate declaration of 'index' here
                if (*(cache + index) == NULL) {
                    *(cache + index) = calloc(strlen(new_word) + 1, sizeof(char));

                    printf("Word \"%s\" ==> %d (calloc)\n", new_word, index);
                } else if (strlen(*(cache + index)) == strlen(new_word)) {
                    printf("Word \"%s\" ==> %d (nop)\n", new_word, index);
                }   
                
                else {
                    *(cache + index) = realloc(*(cache + index), strlen(new_word) + 1);
                    printf("Word \"%s\" ==> %d (realloc)\n", new_word, index);
                }
                strcpy(*(cache + index), new_word);
            }
            free(new_word);
        
        close(file);
    }

    printf("\n");
    // Display the contents of the cache
    printf("Cache:\n");
    for (int i = 0; i < cache_size; i++) {
        if (*(cache + i) != NULL) {
            printf("%c%d%c ==> \"%s\"\n", 91, i, 93, *(cache + i));
        }
    }
    // Free the memory allocated for the cache
    for (int i = 0; i < cache_size; i++) {
        free(*(cache + i));
    }
    free(cache);

    return EXIT_SUCCESS;
}