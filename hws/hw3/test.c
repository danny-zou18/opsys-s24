#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <signal.h>
#include <ctype.h>


#define BUFFER_SIZE 1024
#define MAX_WORD_LENGTH 5
#define MAX_GUESSES 6

extern int total_guesses;
extern int total_wins;
extern int total_losses;
extern char **words;
int total_words;

pthread_mutex_t lock;
int server_fd;

void *handle_client(void *arg);
void cleanup_server(int signo);

int wordle_server( int argc, char ** argv ) {
    if (argc != 5) {
        fprintf(stderr, "ERROR: Invalid argument(s)\n");
        fprintf(stderr, "USAGE: hw3.out <listener-port> <seed> <dictionary-filename> <num-words>\n");
        return EXIT_FAILURE;
    }

    int listener_port = atoi(*(argv+1));
    int seed = atoi(*(argv+2));
    char *dictionary_filename = *(argv+3);
    int num_words = atoi(*(argv+4));

    total_words = num_words;

    int client_socket;
    socklen_t addr_size;
    struct sockaddr_in server_addr, client_addr; // Change "SA_IN" to "struct sockaddr_in"

    srand(seed);

    total_guesses = 0;
    total_wins = 0;
    total_losses = 0;

    words = calloc(num_words + 1, sizeof(char *));

    FILE *file = fopen(dictionary_filename, "r");
    if (!file) {
        fprintf(stderr, "ERROR: Failed to open dictionary file\n");
        return EXIT_FAILURE;
    }

    for (int i = 0; i < num_words; i++) {
        words[i] = calloc(MAX_WORD_LENGTH + 1, sizeof(char));
        if (fscanf(file, "%5s", words[i]) != 1) {
            perror("ERROR: Failed to read word from dictionary file");
            fclose(file);
            exit(EXIT_FAILURE);
        }
        // Convert word to lowercase
        for (char *p = words[i]; *p; p++) *p = tolower(*p);
    }
    fclose(file);

    pthread_mutex_init(&lock, NULL);
    
    // Ignore certain signals
    signal(SIGINT, SIG_IGN);
    signal(SIGTERM, SIG_IGN);
    signal(SIGUSR2, SIG_IGN);
    
    // Setup signal handler for graceful shutdown
    signal(SIGUSR1, cleanup_server);

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0) {
        fprintf(stderr, "ERROR: Socket failed\n");
        return EXIT_FAILURE;
    }

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(listener_port);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0) {
        fprintf(stderr, "ERROR: Bind failed\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    // Listen for incoming connections
    if (listen(server_fd, 3) < 0) {
        fprintf(stderr, "ERROR: Listen failed\n");
        close(server_fd);
        return EXIT_FAILURE;
    }

    printf("MAIN: opened %s (%d words)\n", dictionary_filename, num_words);
    printf("MAIN: seeded pseudo-random number generator with %d\n", seed);
    printf("MAIN: Wordle server listening on port {%d}\n", listener_port);

    addr_size = sizeof(client_addr);

    while (1) {
        if ((client_socket = accept(server_fd, (struct sockaddr *)&client_addr, &addr_size)) < 0) {
            perror("ERROR: Accept failed");
            close(server_fd);
            exit(EXIT_FAILURE);
        }
        printf("MAIN: rcvd incoming connection request\n");
        pthread_t tid;
        int *client_sock = malloc(sizeof(int));
        *client_sock = client_socket;
        pthread_create(&tid, NULL, handle_client, (void *)client_sock);
    }

    pthread_mutex_destroy(&lock);
    for (int i = 0; words[i] != NULL; i++) {
        free(words[i]);
    }
    free(words);
    close(server_fd);
    
    return EXIT_SUCCESS;
}
void *handle_client(void *arg) {
    int client_sock = *(int *)arg;
    free(arg);
    char buffer[BUFFER_SIZE] = {0};
    int valread;

    // Select a random word
    int word_index = rand() % total_words;
    char *hidden_word = words[word_index];
    int guesses_left = MAX_GUESSES;
    int game_won = 0;

    printf("THREAD %lu: selected word: %s\n", pthread_self(), hidden_word);
    
    while ((valread = read(client_sock, buffer, MAX_WORD_LENGTH)) > 0) {
        buffer[valread] = '\0';
        for (char *p = buffer; *p; p++) *p = tolower(*p);
        
        printf("THREAD %lu: waiting for guess\n", pthread_self());
        printf("THREAD %lu: rcvd guess: %s\n", pthread_self(), buffer);
        
        // Validate the guess
        int valid_guess = 0;
        for (char **ptr = words; *ptr; ptr++) {
            if (strcmp(*ptr, buffer) == 0) {
                valid_guess = 1;
                break;
            }
        }
        
        char response[8] = {0};
        response[0] = valid_guess ? 'Y' : 'N';
        if (valid_guess) {
            *(short *)(response + 1) = htons(--guesses_left);
        } else {
            *(short *)(response + 1) = htons(guesses_left);
        }
        
        if (valid_guess) {
            // Generate result string
            char result[6] = {0};
            int hidden_letter_count[26] = {0};
            for (int i = 0; i < MAX_WORD_LENGTH; i++) {
                if (buffer[i] == hidden_word[i]) {
                    result[i] = toupper(buffer[i]);
                } else {
                    result[i] = '-';
                    hidden_letter_count[hidden_word[i] - 'a']++;
                }
            }
            
            for (int i = 0; i < MAX_WORD_LENGTH; i++) {
                if (result[i] == '-' && hidden_letter_count[buffer[i] - 'a'] > 0) {
                    result[i] = tolower(buffer[i]);
                    hidden_letter_count[buffer[i] - 'a']--;
                }
            }
            
            strncpy(response + 3, result, MAX_WORD_LENGTH);
            printf("THREAD %lu: sending reply: %s (%d guesses left)\n", pthread_self(), result, guesses_left);
            
            char* lowercased_result = malloc(MAX_WORD_LENGTH + 1);
            for (int i = 0; i < MAX_WORD_LENGTH; i++) {
                lowercased_result[i] = tolower(result[i]);
            }
            lowercased_result[MAX_WORD_LENGTH] = '\0';

            if (strcmp(lowercased_result, hidden_word) == 0) {
                game_won = 1;
                pthread_mutex_lock(&lock);
                total_wins++;
                pthread_mutex_unlock(&lock);
                break;
            }
        } else {
            strncpy(response + 3, "?????", MAX_WORD_LENGTH);
            printf("THREAD %lu: invalid guess; sending reply: ????? (%d guesses left)\n", pthread_self(), guesses_left);
        }
        
        send(client_sock, response, 8, 0);
        
        pthread_mutex_lock(&lock);
        total_guesses++;
        pthread_mutex_unlock(&lock);
        
        if (guesses_left == 0) {
            pthread_mutex_lock(&lock);
            total_losses++;
            pthread_mutex_unlock(&lock);
            break;
        }
    }
    
    if (game_won || guesses_left == 0) {
        printf("THREAD %lu: game over; word was %s!\n", pthread_self(), hidden_word);
    } else if (guesses_left > 0) {
        printf("THREAD %lu: client gave up; closing TCP connection...\n", pthread_self());
    }

    close(client_sock);
    return NULL;
}

void cleanup_server(int signo) {
    printf("MAIN: SIGUSR1 rcvd; Wordle server shutting down...\n");
    close(server_fd);
    pthread_mutex_destroy(&lock);
    for (int i = 0; words[i] != NULL; i++) {
        free(words[i]);
    }
    free(words);
    exit(EXIT_SUCCESS);
}