#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <sys/ptrace.h>
#include <sys/wait.h>
#include <sys/user.h>
#include <sys/reg.h>
#include <signal.h>
#include "jsmn.h"

#define PORT_NUMBER 8001
#define BUF_SIZE 65536*16
#define FILE_NAME_SIZE 256

int port_bind(char* launcher_ip);

uint32_t change_endian(uint32_t src);
int check_ip_arg(char* start_ip, char* end_ip);
int check_range(char* start_ip, char* end_ip, char* object_ip);

int jsonParse(char* filePath, char* jsonContents);
int jsoneq(const char *json, jsmntok_t *tok, const char *s);
void makeFile(char* filePath, char* nameContents, size_t fileSize, unsigned char* fileContents);

int verify(char* base64_output);

int executeFile(char* filePath);
void debug(pid_t pid);

int main(int argc, char** argv) {
    int r;
    int server_socket;
    int client_socket;
    int client_address_size;
    struct sockaddr_in client_address;
    char buffer[BUF_SIZE];
    char filePath[FILE_NAME_SIZE];

    /* check the number of arguments */
    if (argc != 4) {
        fprintf(stderr, "Usage: %s <launcher_ip> <start_ip> <end_ip>\n", argv[0]);
        exit(1);
    }

    /* check argument is correct */
    if (check_ip_arg(argv[2], argv[3]) == -1) {
        fprintf(stderr, "IP Argument Error\n");
        exit(1);
    }

    server_socket = port_bind(argv[1]);

    /* port bind error */
    if (server_socket == -1) {
        exit(1);
    }

    while(1) {
        client_address_size = sizeof(client_address);
        client_socket = accept(server_socket, (struct sockaddr*) &client_address, &client_address_size);

        /* accept error */
        if (client_socket == -1) {
            fprintf(stderr, "accept() error\n");
            continue;
        }

        /* check notary program's ip */
        if (check_range(argv[2], argv[3], inet_ntoa(client_address.sin_addr)) == -1) {
            fprintf(stderr, "range error\n");
            close(client_socket);
            continue;
        }

        printf("DEBUG] Hello %s\n", inet_ntoa(client_address.sin_addr));

        /* get JSON format from notary program */
        while(1) {
            memset(buffer, 0, BUF_SIZE);
            memset(filePath, 0, FILE_NAME_SIZE);

            r = read(client_socket, buffer, BUF_SIZE);

            if (r <= 0) {
                break;
            }

            if (jsonParse(filePath, buffer) == 0) {
                printf("DEBUG] Client: %s\n", filePath);
            }

        }

        close(client_socket);

    }
    close(server_socket);

    return 0;

}

/* port bind function */
int port_bind(char* launcher_ip) {
    int server_socket;
    struct sockaddr_in server_address;

    server_socket = socket(PF_INET, SOCK_STREAM, 0);

    if (server_socket == -1) {
        fprintf(stderr, "socket() error\n");
        return -1;
    }

    memset(&server_address, 0, sizeof(server_address));
    server_address.sin_family = AF_INET;
    server_address.sin_addr.s_addr = inet_addr(launcher_ip);
    server_address.sin_port = htons(PORT_NUMBER);

    if (bind(server_socket, (struct sockaddr*) &server_address, sizeof(server_address)) == -1) {
        fprintf(stderr, "bind() error\n");
        close(server_socket);
        return -1;
    }

    if (listen(server_socket, 5) == -1) {
        fprintf(stderr, "listen() error\n");
        close(server_socket);
        return -1;
    }

    return server_socket;
}

/* change endian function to check ip range */
uint32_t change_endian(uint32_t src) {
    uint32_t a, b, c, d, result;

    a = (src >> 24) & 0xFF;
    b = ((src >> 16) & 0xFF) << 8;
    c = ((src >> 8) & 0xFF) << 16;
    d = (src & 0xFF) << 24;
    result = a | b | c | d;

    return result;
}

/* check ip arguments is correct */
int check_ip_arg(char* start_ip, char* end_ip) {
    uint32_t start = change_endian(inet_addr(start_ip));
    uint32_t end = change_endian(inet_addr(end_ip));

    if (start <= end) {
        return 0;
    }

    return -1;
}

/* check ip range */
int check_range(char* start_ip, char* end_ip, char* object_ip) {
    uint32_t start = change_endian(inet_addr(start_ip));
    uint32_t end = change_endian(inet_addr(end_ip));
    uint32_t object = change_endian(inet_addr(object_ip));

    if (object >= start && object <= end) {
         return 0;
    }

    return -1;
}

/* parsing json's content name */
int jsoneq(const char *json, jsmntok_t *tok, const char *s) {
    if (tok->type == JSMN_STRING && (int) strlen(s) == tok->end - tok->start && 
        strncmp(json + tok->start, s, tok->end - tok->start) == 0) {
        return 0;
    }
    return -1;
}

/* get name and contents */
int jsonParse(char* filePath, char* jsonContents) {
    int i, r;
    jsmn_parser p;
    jsmntok_t t[128];
    char* nameContents;
    char* bodyContents;
    char* fileContents;
    size_t nameSize = 0;
    size_t bodySize = 0;
    size_t fileSize = 0;

    jsmn_init(&p);
    r = jsmn_parse(&p, jsonContents, strlen(jsonContents), t, sizeof(t)/sizeof(t[0]));
    if (r < 0) {
        fprintf(stderr, "Failed to parse JSON: %d\n", r);
        return -1;
    }

    if (r < 1 || t[0].type != JSMN_OBJECT) {
        fprintf(stderr, "Object expected\n");
        return -1;
    }

    /* get name and contents */
    for (i = 1; i < r; i+=2) {
        if (jsoneq(jsonContents, &t[i], "name") == 0) {
            nameSize = t[i+1].end - t[i+1].start + 1;
            nameContents = (char*)malloc(nameSize*sizeof(char));
            memcpy(nameContents, jsonContents + t[i+1].start, nameSize - 1);
            nameContents[nameSize] = '\0';
            printf("DEBUG] name: %s\n", nameContents);
        } else if (jsoneq(jsonContents, &t[i], "body") == 0) {
            bodySize = t[i+1].end - t[i+1].start + 1;
            bodyContents = (char*)malloc(bodySize*sizeof(char));
            memcpy(bodyContents, jsonContents + t[i+1].start, bodySize - 1);
            bodyContents[bodySize] = '\0';
            printf("DEBUG] body: %d\n", (int)  bodySize-1);
        }
    }

    /* make base64 file using name and contents */
    makeFile(filePath, nameContents, bodySize-1, bodyContents);

    /* check pgp key and execute file in contents */
    verify(filePath);

    free(nameContents);
    free(bodyContents);

    return 0;
}

/* make base64 file */
void makeFile(char* filePath, char* nameContents, size_t fileSize, unsigned char* fileContents) {
    FILE* fp;

    strcpy(filePath, "./");
    strcat(filePath, nameContents);

    fp = fopen(filePath, "w");

    fwrite(fileContents, 1, fileSize, fp);

    fclose(fp);
}

/* verify and excute file */
int verify(char* base64_output){
    char cmdline[150];
    char gpg_output[100];
    char exe_output[100];
    char buff[1024];
    FILE *fp;

    /* convert base64 to gpg file */
    strcpy(gpg_output, base64_output);
    strcat(gpg_output, ".gpg");

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "base64 --decode %s > %s", base64_output, gpg_output);
    printf("%s\n", cmdline);
    fp = popen(cmdline, "r");
    if(fp == NULL) { // ERROR
        perror("base64 error");
        pclose(fp);
        return -1;
    }
    pclose(fp);

    /* convert gpg to exe file with verification */
    strcpy(exe_output, base64_output);
    strcat(exe_output, ".exe");

    memset(cmdline, 0, sizeof(cmdline));
    sprintf(cmdline, "gpg --output %s --decrypt %s 2>&1", exe_output, gpg_output);
    printf("%s\n", cmdline);
    fp = popen(cmdline, "r");
    if(fp == NULL) { // ERROR
        perror("gpg error");
        pclose(fp);
        return -1;
    }

    fgets(buff, sizeof(buff), fp);
    //printf("buff: %s\n", buff);

	int idx = 0;
	int result = 0;
    char* good = "gpg: Good signature from \"IS521_notary <IS521_notary@kaist.ac.kr>\"";
	while(fgets(buff, sizeof(buff), fp) != NULL){
        int i;
        int flag = 1;
	    for(i = 0 ; i < strlen(good) ; i++){
	        if(buff[i] != good[i]){
	            flag = 0;
            }
	    }
        result = result | flag; 
	    printf(">> %s", buff);
	}
    pclose(fp);

    /* give permission to execute */
    if (chmod(exe_output, 0755) == -1) {
        fprintf(stderr, "chmod err\n");
    }

    /* execute the file */
    executeFile(exe_output);

    return 0;
}

/* make process to execute file */
int executeFile(char* filePath) {
    pid_t child;
    child = fork();
    if(child == 0){ // childe
        chroot("./");
        ptrace(PTRACE_TRACEME, 0, 0, 0);
        execl(filePath, filePath, 0);
    } else if (child < 0){
        perror("fork");
    }else{ // parent
        debug(child);
    }

    return 0;
}

/* watch the file using ptrace */
void debug(pid_t pid) {
    int syscall_num, status;
    struct user_regs_struct regs;
    waitpid(pid, &status, 0);
    ptrace(PTRACE_SYSCALL, pid, 0, 0);
    while(WIFSTOPPED(status)){
        ptrace(PTRACE_GETREGS, pid, 0, &regs);
        syscall_num = regs.orig_eax;
        if(syscall_num == -1){ // Error occurred
            printf("Error occurred at runtime\n");
            kill(pid, 9);
            return;
        }
        if(syscall_num == 11){ // execve
            printf("SYSCALL #%d called\n", syscall_num);
            kill(pid, 9);
            return;
        }
        ptrace(PTRACE_SYSCALL, pid, 0, 0);
        waitpid(pid, &status, 0);
    }	
    return;
    //printf("NONONO\n");
}
