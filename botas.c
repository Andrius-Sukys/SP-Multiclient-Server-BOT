#define _GNU_SOURCE

#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <stdlib.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <netinet/in.h>
#include <sys/wait.h>
#include <signal.h>
#include <sys/select.h>

#define BACKLOG 10
#define BUFFER_SIZE 1024

int validate_port(char* port)
{
    if((int) strtol(port, (char **)NULL, 10) <= 1024)
        return 0;
    else
        return 1;
}

int main(int argc, char* argv[])
{
    char* port_def = malloc(sizeof(char) * 6);

    if(argc == 2)
    {
        if(!validate_port(argv[1]))
        {
            printf("Port number must be higher than 1024!\n");
            exit(1); 
        }

        port_def = argv[1];
    }
    else
    {
        printf("To launch the program type: botas port\n");
        exit(1);
    }

    int status;

    int sockfd;
    int yes = 1;

    struct addrinfo hints;
    struct addrinfo* res;
    struct addrinfo* ptr;

    memset(&hints, 0, sizeof hints);

    hints.ai_family = AF_INET6;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;

    status = getaddrinfo(NULL, port_def, &hints, &res);

    if(status != 0)
    {
        printf("ERROR: getaddrinfo failed.\n");
        exit(1);
    }

    for(ptr = res; ptr != NULL; ptr = ptr->ai_next)
    {
        status = socket(ptr->ai_family, ptr->ai_socktype, ptr->ai_protocol);

        sockfd = status;

        setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(char));

        status = connect(sockfd, ptr->ai_addr, ptr->ai_addrlen);

        if(status == -1)
        {
            printf("ERROR: connect failed.\n");
            close(sockfd);
            continue;
        }

        break;
    }

    if(ptr == NULL)
    {
        printf("An error occurred. Unable to access the socket.\n");
        exit(1);
    }

    printf("Connected.\n");

    status = send(sockfd, "BOTAS\n", strlen("BOTAS\n"), 0);

    char password[BUFFER_SIZE];

    printf("In order to track activity, enter the password: ");

    scanf("%s", password);

    password[strlen(password)] = '\0';
    
    status = send (sockfd, password, strlen(password), 0);

    fclose(fopen("logs.txt", "w"));

    char buffer[BUFFER_SIZE];

    int total_messages = 0;

    FILE *log;

    while(1)
    {
        int bytes_recv;
        bytes_recv = recv(sockfd, buffer, BUFFER_SIZE, 0);
        
        buffer[bytes_recv] = '\0';

        if(bytes_recv <= 0)
        {
            if(bytes_recv == -1)
            {
                printf("ERROR: recv failed.\n");
                close(sockfd);
                exit(1);
            }
            if(bytes_recv == 0)
            {
                printf("Server closed connection.\n");
                close(sockfd);
                exit(1);
            }
        }
        else
        {
            fflush(stdout);

            char name[BUFFER_SIZE];

            char msg[BUFFER_SIZE];

            char temp[BUFFER_SIZE];

            char msg_check[BUFFER_SIZE];

            char name_check[BUFFER_SIZE];

            if((strncmp(buffer, "ATSIUSKVARDA", 12) != 0) && (strncmp(buffer, "VARDASOK", 8) != 0))
            {
                total_messages++;

                int len = strlen(buffer);

                if((len > 0) && (buffer[len - 1] == '\n'))
                {
                    buffer[len - 1] = '\n';
                    if((len > 1) && buffer[len - 2] == '\r')
                    {
                        buffer[len - 1] = '\0';
                        buffer[len - 2] = '\n';
                    }
                }

                log = fopen("logs.txt", "a");
                fprintf(log, "%s", buffer);
                printf(buffer, "%s");
                fclose(log);

                strcpy(temp, buffer);

                char *ptr_name = strstr(buffer, "PRANESIMAS");

                if(ptr != NULL)
                {
                    char *token_name = strtok(ptr_name + strlen("PRANESIMAS"), ":");
                    strcpy(name, token_name);
                    strcpy(buffer, temp);
                }

                char *ptr_msg = strstr(buffer, ":");
                
                if(ptr_msg != NULL)
                {
                    char *token_msg = strtok(ptr_msg + 1, "\n");
                    strcpy(msg, token_msg);
                    strcat(msg, "\n");
                    strcpy(buffer, temp);
                }

                if((strncmp(msg, " @BOTAS #\n", 10) == 0))
                {
                    int total_words = 0;

                    FILE *fp = fopen("logs.txt", "r");

                    for(int i = 0; i < total_messages; ++i)
                    {
                        fgets(temp, sizeof temp, fp);

                        char *ptr = strstr(temp, ":");
                
                        if(ptr != NULL)
                        {
                            char *token = strtok(ptr + 1, "\n");
                            strcpy(msg_check, token);
                            strcat(msg_check, "\n");
                        }

                        char *ptr_2 = strstr(temp, "PRANESIMAS");

                        if(ptr_2 != NULL)
                        {
                            char *token = strtok(ptr_2 + strlen("PRANESIMAS"), ":");
                            strcpy(name_check, token);
                        }

                        if((strncmp(msg_check, " @BOTAS #\n", 10) == 0) ||
                           (strncmp(msg_check, " @BOTAS #MAX\n", 13) == 0) ||
                           (strncmp(msg_check, " @BOTAS #MIN\n", 13) == 0) ||
                           (strcmp(name, name_check) != 0))
                        {
                            continue;
                        }
                        else
                        {
                            memmove(msg_check, msg_check + 1, strlen(msg_check));

                            int len = strlen(msg_check);

                            for(int j = 0; j < len; ++j)
                            {
                                if((msg_check[j] != ' ') && (msg_check[j] != '\t') && (msg_check[j] != '\n'))
                                {
                                    if((j == 0) || (msg_check[j-1] == ' ') || (msg_check[j-1] == '\t') || (msg_check[j-1] == '\n'))
                                    {
                                        total_words++;
                                    }
                                }
                            }
                        }
                    }

                    char number[BUFFER_SIZE];

                    sprintf(number, "%d", total_words);

                    strcpy(buffer, name);
                    strcat(buffer, " has sent ");
                    strcat(buffer, number);
                    strcat(buffer, " words.\n");
                    
                    status = send(sockfd, buffer, strlen(buffer), 0);

                    if(status == -1)
                    {
                        printf("ERROR: send failed.\n");
                        close(sockfd);
                        exit(1);
                    }

                    total_words = 0;

                    fclose(fp);
                }

                if(strncmp(msg, " @BOTAS #MAX\n", 13) == 0)
                {
                    char longest[BUFFER_SIZE] = "";

                    char longest_length[BUFFER_SIZE];

                    FILE *fp = fopen("logs.txt", "r");

                    for(int i = 0; i < total_messages; ++i)
                    {
                        fgets(temp, sizeof temp, fp);

                        char *ptr = strstr(temp, ":");
                
                        if(ptr != NULL)
                        {
                            char *token = strtok(ptr + 1, "\n");
                            strcpy(msg_check, token);
                            strcat(msg_check, "\n");
                        }

                        char *ptr_2 = strstr(temp, "PRANESIMAS");

                        if(ptr_2 != NULL)
                        {
                            char *token = strtok(ptr_2 + strlen("PRANESIMAS"), ":");
                            strcpy(name_check, token);
                        }

                        if((strncmp(msg_check, " @BOTAS #\n", 10) == 0) ||
                           (strncmp(msg_check, " @BOTAS #MAX\n", 13) == 0) ||
                           (strncmp(msg_check, " @BOTAS #MIN\n", 13) == 0) ||
                           (strcmp(name, name_check) != 0))
                        {
                            continue;
                        }
                        else
                        {
                            memmove(msg_check, msg_check + 1, strlen(msg_check));
                            
                            int len = strlen(msg_check);

                            int index = 0;

                            int current_length = 0, max_length = 0;

                            for(int j = 0; j < len; ++j)
                            {
                                if((msg_check[j] != ' ') && (msg_check[j] != '\t') && (msg_check[j] != '\n') && (msg_check[j] != '\0'))
                                {
                                    current_length++;
                                    continue;
                                }

                                if(current_length > max_length)
                                {
                                    max_length = current_length;
                                    index = j - max_length;
                                }

                                current_length = 0;
                            }

                            int k;

                            if(max_length > strlen(longest))
                            {
                                for(k = 0; k < max_length; ++k)
                                {   
                                    longest[k] = msg_check[index + k];
                                }

                                longest[k] = '\0';
                            }
                        }
                    }

                    sprintf(longest_length, "%ld", strlen(longest));

                    strcpy(buffer, name);
                    strcat(buffer, "'s longest word is [");
                    strcat(buffer, longest);
                    strcat(buffer, "], length: ");
                    strcat(buffer, longest_length);
                    strcat(buffer, " letters.\n");
                    
                    status = send(sockfd, buffer, strlen(buffer), 0);

                    if(status == -1)
                    {
                        printf("ERROR: send failed.\n");
                        close(sockfd);
                        exit(1);
                    }

                    strcpy(longest, "");

                    fclose(fp);
                }

                if(strncmp(msg, " @BOTAS #MIN\n", 13) == 0)
                {
                    char shortest[BUFFER_SIZE] = "";

                    char shortest_length[BUFFER_SIZE];

                    FILE *fp = fopen("logs.txt", "r");

                    for(int i = 0; i < total_messages; ++i)
                    {
                        fgets(temp, sizeof temp, fp);

                        char *ptr = strstr(temp, ":");
                
                        if(ptr != NULL)
                        {
                            char *token = strtok(ptr + 1, "\n");
                            strcpy(msg_check, token);
                            strcat(msg_check, "\n");
                        }

                        char *ptr_2 = strstr(temp, "PRANESIMAS");

                        if(ptr_2 != NULL)
                        {
                            char *token = strtok(ptr_2 + strlen("PRANESIMAS"), ":");
                            strcpy(name_check, token);
                        }

                        if((strncmp(msg_check, " @BOTAS #\n", 10) == 0) ||
                           (strncmp(msg_check, " @BOTAS #MAX\n", 13) == 0) ||
                           (strncmp(msg_check, " @BOTAS #MIN\n", 13) == 0) ||
                           (strcmp(name, name_check) != 0))
                        {
                            continue;
                        }
                        else
                        {
                            memmove(msg_check, msg_check + 1, strlen(msg_check));

                            int len = strlen(msg_check);

                            int flag = -1;

                            for(int j = 0; j < len; ++j)
                            {
                                if((msg_check[j] != ' ') && (msg_check[j] != '\t') && (msg_check[j] != '\n') && (msg_check[j] != '\0'))
                                {
                                    flag = j;
                                    break;
                                }
                            }

                            if(flag != -1)
                            {
                                int j = 0;

                                while((msg_check[flag + j] != ' ') && (msg_check[flag + j] != '\t') && (msg_check[flag + j] != '\n') && (msg_check[flag + j] != '\0'))
                                {   
                                    shortest[j] = msg_check[flag + j];
                                    j++;
                                }

                                shortest[j] = '\0';

                                break;
                            }
                            else
                            {
                                continue;
                            }
                        }
                    }

                    fclose(fp);

                    sprintf(shortest_length, "%ld", strlen(shortest));

                    if(strlen(shortest) > 0)
                    {
                        fp = fopen("logs.txt", "r");

                        for(int i = 0; i < total_messages; ++i)
                        {
                            fgets(temp, sizeof temp, fp);

                            char *ptr = strstr(temp, ":");
                    
                            if(ptr != NULL)
                            {
                                char *token = strtok(ptr + 1, "\n");
                                strcpy(msg_check, token);
                                strcat(msg_check, "\n");
                            }

                            char *ptr_2 = strstr(temp, "PRANESIMAS");

                            if(ptr_2 != NULL)
                            {
                                char *token = strtok(ptr_2 + strlen("PRANESIMAS"), ":");
                                strcpy(name_check, token);
                            }

                            if((strncmp(msg_check, " @BOTAS #\n", 10) == 0) ||
                               (strncmp(msg_check, " @BOTAS #MAX\n", 13) == 0) ||
                               (strncmp(msg_check, " @BOTAS #MIN\n", 13) == 0) ||
                               (strcmp(name, name_check) != 0))
                            {
                                continue;
                            }
                            else
                            {
                                memmove(msg_check, msg_check + 1, strlen(msg_check));

                                int len = strlen(msg_check);

                                int index = 0;

                                int current_length = 0, min_length = strlen(shortest);

                                for(int j = 0; j < len; ++j)
                                {
                                    if((msg_check[j] != ' ') && (msg_check[j] != '\t') && (msg_check[j] != '\n') && (msg_check[j] != '\0'))
                                    {
                                        current_length++;
                                    }
                                    else
                                    {
                                        if((current_length > 0) && (msg_check[j-1] != ' ') && (msg_check[j-1] != '\t') && (msg_check[j-1] != '\n') && (msg_check[j-1] != '\0'))
                                        {
                                            if(current_length < min_length)
                                            {
                                                min_length = current_length;
                                                index = j - min_length;
                                            }
                                        }

                                        current_length = 0;
                                    }
                                }

                                int k;

                                if(min_length < strlen(shortest))
                                {
                                    for(k = 0; k < min_length; ++k)
                                    {   
                                        shortest[k] = msg_check[index + k];
                                    }

                                    shortest[k] = '\0';
                                } 
                            }
                        }

                        fclose(fp);

                        sprintf(shortest_length, "%ld", strlen(shortest));
                    }

                    strcpy(buffer, name);
                    strcat(buffer, "'s shortest word is [");
                    strcat(buffer, shortest);
                    strcat(buffer, "], length: ");
                    strcat(buffer, shortest_length);
                    strcat(buffer, " letters.\n");
                    
                    status = send(sockfd, buffer, strlen(buffer), 0);

                    if(status == -1)
                    {
                        printf("ERROR: send failed.\n");
                        close(sockfd);
                        exit(1);
                    }
                }
            }
        }
    }

    close(sockfd);

    return 0;
    
}