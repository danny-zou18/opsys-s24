#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <fcntl.h>
#include <unistd.h>

// Function to calculate the hash value for a word
int hash(const char* word, int cache_size) {
    int sum = 0;
    for (int i = 0; i < strlen(word); i++) {
        sum += *(word + i);
    }
    return sum % cache_size;
}

int main(int argc, char** argv) {
    if (argc < 3) {
        fprintf(stderr, "ERROR: Insufficient command-line arguments\n");
        return 1;
    }

    int cache_size = atoi(*(argv + 1));
    if (cache_size <= 0) {
        fprintf(stderr, "ERROR: Invalid cache size\n");
        return 1;
    }

    // Dynamically allocate the cache array
    char** cache = calloc(cache_size, sizeof(char*));
    if (cache == NULL) {
        fprintf(stderr, "ERROR: Failed to allocate memory for cache\n");
        return 1;
    }

    // Process each input file
    for (int i = 2; i < argc; i++) {
        int file = open(*(argv+i), O_RDONLY);
        if (file == -1) {
            fprintf(stderr, "ERROR: Failed to open file %s\n", *(argv+i));
            continue;
        }

        char c;
        ssize_t bytes_read;
        int word_length = 0;
        char* new_word = NULL;
        while ((bytes_read = read(file, &c, 1)) > 0) {
            if (c == ' ' || !isalnum(c)) {
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
                }
            } else {
                if (word_length == 0) {
                    new_word = calloc(1, sizeof(char));
                    *(new_word+word_length) = c;
                    if (new_word == NULL) {
                        fprintf(stderr, "ERROR: Memory allocation failed\n");
                        return 1; // Or handle the error as needed
                    }
                } else {
                    new_word = realloc(new_word, word_length + 1);
                    *(new_word+word_length) = c;
                    if (new_word == NULL) {
                        fprintf(stderr, "ERROR: Memory allocation failed\n");
                        return 1; // Or handle the error as needed
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
        }
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

    return 0;
}
