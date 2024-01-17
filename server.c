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
#define PASSWORD "notahacker"

struct fd_name
{
    int has_name;
    char name[BUFFER_SIZE];
    int confirmed;
};

int validate_port(char* port)
{
    if((int) strtol(port, (char **)NULL, 10) <= 1024)
        return 0;
    else
        return 1;
}

int validate_name(struct fd_name* names, int size, char *name)
{
    for (int i = 0; i <= size; i++)
    {
        if(names[i].has_name == 1)
        {
            if (strcmp(names[i].name, strtok(name, "\r\n")) == 0)
            {
                return 1;
            }
        }
    }
    return 0;
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
        printf("To launch the program type: server port\n");
        exit(1);
    }

    int status;

    int listener;
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

        listener = status;

        if(status == -1)
        {
            printf("ERROR: socket failed.\n");
            continue;
        }

        setsockopt(listener, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(char));

        status = bind(listener, ptr->ai_addr, ptr->ai_addrlen);

        if(status == -1)
        {
            printf("ERROR: bind failed.\n");
            close(listener);
            continue;
        }

        break;
    }

    if(ptr == NULL)
    {
        printf("An error occurred. Unable to access the socket.\n");
        exit(1);
    }

    status = listen(listener, BACKLOG);

    if(status == -1)
    {
        printf("ERROR: listen failed.\n");
        close(listener);
        exit(1);
    }

    freeaddrinfo(res);

    printf("Waiting for incoming client connections...\n");

    fd_set master;
    fd_set read_fds;
    int fdmax;

    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    FD_SET(listener, &master);

    fdmax = listener;

    char buffer[BUFFER_SIZE];

    struct fd_name *fd_names = malloc(sizeof(*fd_names) * BUFFER_SIZE);

    while(1)
    {
        read_fds = master;

        if((select(fdmax + 1, &read_fds, NULL, NULL, NULL)) == -1)
        {
            printf("ERROR: select failed.\n");
            close(listener);
            exit(1);
        }

        for(int i = 0; i <= fdmax; ++i)
        {
            if(FD_ISSET(i, &read_fds))
            {
                if(i == listener)
                {
                    int newfd;
                    struct sockaddr_storage addr;
                    socklen_t addr_size;

                    addr_size = sizeof addr;

                    status = accept(listener, (struct sockaddr*) &addr, &addr_size);

                    newfd = status;

                    if(status == -1)
                    {
                        printf("ERROR: accept failed.\n");
                        continue;
                    }

                    FD_SET(newfd, &master);

                    if(newfd > fdmax)
                    {
                        fdmax = newfd;
                    }

                    (fd_names)[i].has_name = 0;

                    printf("\nNew connection on socket %d.\n", newfd);

                    status = send(newfd, "ATSIUSKVARDA\n", strlen("ATSIUSKVARDA\n"), 0);

                    if(status == -1)
                    {
                        printf("ERROR: send failed.\n");
                        close(listener);
                        exit(1);
                    }
                }
                else
                {
                    int bytes_recv;

                    bytes_recv = recv(i, buffer, BUFFER_SIZE, 0);

                    buffer[bytes_recv] = '\0';

                    if(bytes_recv <= 0)
                    {
                        if(bytes_recv == -1)
                        {
                            printf("ERROR: recv failed.\n");
                            close(listener);
                            exit(1);
                        }
                        if(bytes_recv == 0)
                        {
                            printf("\nConnection closed on socket %d. %s disconnected.\n\n", i, fd_names[i].name);

                            strcpy(buffer, "PRANESIMAS");
                            strcat(buffer, fd_names[i].name);
                            strcat(buffer, " has disconnected.\n");

                            fd_names[i].has_name = 0;

                            close(i);
                            FD_CLR(i, &master);

                            for(int j = 0; j <= fdmax; ++j)
                            {
                                if(FD_ISSET(j, &master))
                                {
                                    if((j != listener) && (j != i) && (strcmp(fd_names[j].name, "BOTAS") != 0))
                                    {
                                        status = send(j, buffer, strlen(buffer), 0);

                                        if(status == -1)
                                        {
                                            printf("ERROR: send failed.\n");
                                            close(listener);
                                            exit(1);
                                        }
                                    }
                                }
                            }
                        }
                    }
                    else if(fd_names[i].has_name == 0)
                    {
                        while(validate_name(fd_names, fdmax, buffer))
                        {
                            status = send(i, "ATSIUSKVARDA\n", strlen("ATSIUSKVARDA\n"), 0);

                            if(status == -1)
                            {
                                printf("ERROR: send failed.\n");
                                close(listener);
                                exit(1);
                            }

                            bytes_recv = recv(i, buffer, BUFFER_SIZE, 0);

                            if(bytes_recv == -1)
                            {
                                printf("ERROR: recv failed.\n");
                                close(listener);
                                exit(1);
                            }
                            if(bytes_recv == 0)
                            {
                                printf("Connection closed on socket %d.\n", i);

                                close(i);
                                FD_CLR(i, &master);
                            }
                        }

                        strcpy(fd_names[i].name, strtok(buffer, "\r\n"));

                        fd_names[i].has_name = 1;

                        fd_names[i].confirmed = 1;

                        printf("Socket %d - %s\n\n", i, buffer);

                        if(strncmp(fd_names[i].name, "BOTAS\n", 5) == 0)
                        {
                            fd_names[i].confirmed = 0;
                            printf("BOTAS is trying to join. Confirmation needed!\n");
                        }

                        status = send(i, "VARDASOK\n", strlen("VARDASOK\n"), 0);

                        if(status == -1)
                        {
                            printf("ERROR: send failed.\n");
                            close(listener);
                            exit(1);
                        }

                        strcpy(buffer, "PRANESIMAS");
                        strcat(buffer, fd_names[i].name);
                        strcat(buffer, " has connected.\n");

                        for(int j = 0; j <= fdmax; ++j)
                        {
                            if(FD_ISSET(j, &master))
                            {
                                if((j != listener) && (strcmp(fd_names[j].name, "BOTAS") != 0))
                                {
                                    status = send(j, buffer, strlen(buffer), 0);

                                    if(status == -1)
                                    {
                                        printf("ERROR: send failed.\n");
                                        close(listener);
                                        exit(1);
                                    }
                                }
                            }
                        }
                    }
                    else if((strncmp(fd_names[i].name, "BOTAS", 5) == 0) && (fd_names[i].confirmed == 0))
                    {
                        if(strncmp(buffer, PASSWORD, strlen(PASSWORD)) != 0)
                        {
                            printf("Authentication unsuccessful. Disconnected the imposter.\n");
                            fd_names[i].has_name = 0;
                            fd_names[i].confirmed = 0;
                            close(i);
                            FD_CLR(i, &master);

                            printf("BOTAS has disconnected.\n");
                        }
                        else
                        {
                            printf("Authentication successful!\n");
                            fd_names[i].confirmed = 1;
                        }
                        
                    }
                    else
                    {
                        printf("%s: %s", fd_names[i].name, buffer);

                        char temp[BUFFER_SIZE];

                        strcpy(temp, buffer);

                        strcpy(buffer, "PRANESIMAS");
                        strcat(buffer, fd_names[i].name);
                        strcat(buffer, ": ");
                        strcat(buffer, temp);

                        for(int j = 0; j <= fdmax; ++j)
                        {
                            if(FD_ISSET(j, &master))
                            {
                                if(j != listener)
                                {
                                    status = send(j, buffer, strlen(buffer), 0);

                                    if(status == -1)
                                    {
                                        printf("ERROR: send failed.\n");
                                        close(listener);
                                        exit(1);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    close(listener);

    free(port_def);
    free(fd_names);

    return 0;
}