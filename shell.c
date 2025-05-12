#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <ctype.h>

void pwd(void);
void show_prompt(void);
int cd(char *[]);
void pipeline(char *);

int main(void)
{
    char line[1024];
    char *input[64];
    while (1) // 쉘 유지
    {
        int i = 0;
        show_prompt(); // 리눅스쉘 형태 구현
        fgets(line, sizeof(line), stdin);  // 사용자로부터 입력 받기
        line[strcspn(line, "\n")] = 0; // 개행문자 제거
        char *splitCmds[10];
        int splitCount = 0;

        // 1단계: ; 기준으로 나누기
        char *savePtr;
        char *cmdToken = strtok_r(line, ";", &savePtr); // ;를 기준으로 첫번째 주소 반환
        while (cmdToken != NULL && splitCount < 10) // ;가 끝날 때까지 반복
        {
            splitCmds[splitCount++] = cmdToken; // splitCmds 에 cmdToken의 주소값 저장
            cmdToken = strtok_r(NULL, ";", &savePtr); // ;를 기준으로 다음 주소 반환
        }

        for (i = 0; i < splitCount; i++) // 논리연산자 (&&, ||)까지 처리
        {
            char *logic_cmds[10]; // 명령어를 기억하는 배열
            char *logic_ops[10]; // 논리연산자를 기억하는 배열
            int logic_count = 0;

            char *p = splitCmds[i];
            while (*p != '\0') // 문자열이 끝날 때까지 반복
            {
                char *start = p;
                while (*p != '\0' && !(p[0] == '&' && p[1] == '&') && !(p[0] == '|' && p[1] == '|')) // &&나 ||를 만날 때까지 반복
                    p++; // 포인터를 전진
                int len = p - start; // 포인터가 전진한 길이
                char *cmd = (char *)malloc(len + 1); // len + 1 크기만큼의 동적메모리 할당 이후 그 주소를 cmd에 반환
                strncpy(cmd, start, len); // 문자열을 cmd에 복사하기
                cmd[len] = '\0'; // strncpy는 널문자를 보장하지 않기에 강제로 널문자 삽입
                logic_cmds[logic_count] = cmd; // logic_cmds 포인터에 cmd에 저장된 주소 저장

                if (*p == '&' && *(p + 1) == '&') // && 논리연산자가 있으면
                {
                    logic_ops[logic_count] = "&&"; 
                    p += 2; // &&가 2칸이기에 포인터를 2칸 전진
                } 
                else if (*p == '|' && *(p + 1) == '|') // || 논리연산자가 있으면
                {
                    logic_ops[logic_count] = "||";
                    p += 2; // ||가 2칸이기에 포인터를 2칸 전진
                } 
                else // 논리 연산자 없음
                {
                    logic_ops[logic_count] = NULL; 
                }

                logic_count++; // 다음 논리연산자 있는지 체크를 위해 1칸 올리기
            }

            int goNext = 1;

            for (int k = 0; k < logic_count; k++)    // 논리 연산자에 따라 실행 여부 결정
            {
                if (k > 0) 
                {
                    if (strcmp(logic_ops[k - 1], "&&") == 0 && !goNext) // && 연산자일 때 앞의 명령어가 실패하면 탈출
                        break;
                    if (strcmp(logic_ops[k - 1], "||") == 0 && goNext) // || 연산자일 때 앞의 명령어가 성공하면 탈출
                        break;
                }

                int background = 0;
                int len = strlen(logic_cmds[k]); // 첫번째 명령어 길이 저장
                while (len > 0 && isspace(logic_cmds[k][len - 1])) // 몇번째 명령어인지 체크하고 그 앞이 공백인지 체크
                {
                    logic_cmds[k][--len] = '\0'; // 공백 자리에 널문자 덮어 씌우기
                }
                if (len > 0 && logic_cmds[k][len - 1] == '&') // 몇번째 명령어인지 체크하고 백그라운드 실행인지 체크
                {
                    background = 1;
                    logic_cmds[k][len - 1] = '\0'; // 공백자리에 널문자 덮어 씌우기
                }

               
                 if (strchr(logic_cmds[k], '|') != NULL) // 파이프라인이 있으면
                    {
                        pipeline(logic_cmds[k]); 
                        goNext = 1; 
                        continue;
                    }
                // 공백으로 파싱
                char *token = strtok(logic_cmds[k], " "); // 명령어를 공백 기준으로 끊고 토큰 반환
                int j = 0;
                while (token != NULL) 
                {
                    input[j++] = token; // input에 토큰 저장
                    token = strtok(NULL, " "); // 다음 토큰으로 갱신
                }
                input[j] = NULL; // 문자열 끝에는 널문자 삽입

                if (input[0] == NULL)
                    continue;

                if (strcmp(input[0], "exit") == 0) // 명령어의 첫번째 부분이 exit인지 체크
                {
                    exit(0);
                }
                else if (strcmp(input[0], "pwd") == 0) // 명령어의 첫번째 부분이 pwd인지 체크
                {
                    pwd();
                    goNext = 1; 
                }
                else if (strcmp(input[0], "cd") == 0) // 명령어의 첫번째 부분이 cd인지 체크
                {
                    if (input[1] == NULL) 
                    {
                        chdir(getenv("HOME")); // cd만 쳤을 경우
                        goNext = 1;
                    }
                    else 
                    {
                        if (strcmp(input[1], "~") == 0) // ~를 쳤을 경우
                        {
                            input[1] = getenv("HOME"); // ~를 HOME으로 변환
                        }
                        {
                            if (chdir(input[1]) != 0) // cd가 성공했는지 체크(실패하면 -1 반환)
                            {
                                perror("cd error"); // 오류 메시지 전송
                                goNext = 0;
                            } 
                            else 
                            {
                                goNext = 1;
                            }
                        }
                    }
                }
                  else if (strcmp(input[0], "hello") == 0) // 직접 만든 명령어
                  {
                    char *user = getenv("USER"); // user 환경변수를 저장함
                    if (user == NULL) 
                    user = "User";
                    printf("Hello, %s! This is my mini shell.\n", user); 
                    goNext = 1;
                }
                else
                {
                    pid_t pid = fork(); // 자식 디렉토리 생성 후 자식의 pid값 저장
                    if (pid < 0) // 생성 실패 체크
                    {
                        perror("fork");
                        goNext = 0;
                    }
                    else if (pid == 0) // 자식이 진행하는 분기
                    {
                        execvp(input[0], input); // 명령어 실행
                        perror("execvp"); // 성공 시 실행 안 됨
                        exit(1);
                    }
                    else
                    {
                        if (!background) // 백그라운드 실행이 아닐 때
                        {
                            int status;
                            waitpid(pid, &status, 0); // 자식이 끝날 때까지 기다리기
                            if (WIFEXITED(status) && WEXITSTATUS(status) == 0) // 자식이 정상적으로 종료 되었는지 체크
                                goNext = 1;
                            else
                                goNext = 0;
                        }
                        else
                        {
                            printf("[백그라운드 실행] pid: %d\n", pid);
                            goNext = 1;
                        }
                    }
                }
            }
            for (int k = 0; k < logic_count; k++)
            {
                free(logic_cmds[k]); // 동적 메모리 해제
            }
        }
    }

    return 0;
}

void show_prompt(void) 
{
    char hostname[1024];
    char cwd[1024];
    char *home = getenv("HOME");
    char *user = getenv("USER"); // 사용자 이름 저장
    gethostname(hostname, sizeof(hostname)); // 호스트 이름 저장
    getcwd(cwd, sizeof(cwd)); // 현재 디렉토리 저장
    if (home && strstr(cwd, home) == cwd) // 홈디렉토리 또는 그 하위 디렉토리인지 체크
    {
        printf("%s@%s:~%s$ ", user, hostname, cwd + strlen(home)); // home을 ~로 바꾸고 뒤 주소 출력
    }
    else
    {
        printf("%s@%s:%s$ ", user, hostname, cwd); // 주소 그대로 출력
    }
}

void pwd(void)
{
    char cwd[1024];
    getcwd(cwd, sizeof(cwd)); // 현재 디렉토리 저장
    printf("%s \n", cwd);
}

int cd(char *input[])
{
    if (input[1] == NULL) // 뒤 명령어 없음
    {
        if (chdir(getenv("HOME")) != 0) // 이동 실패했는지 체크
        {
            perror("cd error"); // 오류 메시지 발송
            return 1;
        }
        return 0;
    }
    else // 뒤 명령어 있음
    {
        if (chdir(input[1]) != 0) // 이동 실패했는지 체크
        {
            perror("cd error"); // 오류 메시지 발송
            return 1;
        }
        return 0;
    }
}

void pipeline(char *line)
{
    char *cmds[10];
    int cmd_count = 0;
    char *cmd = strtok(line, "|"); // 파이프를 기준으로 line 분할
    while (cmd != NULL && cmd_count < 10)
    {
        cmds[cmd_count++] = cmd; // cmds에 파이프를 기준으로 명령어의 주소를 저장함
        cmd = strtok(NULL, "|"); // 파이프 뒤의 명령어의 주소로 cmd를 갱신
    }

    int pipes[2][2];  // 앞의 인덱스는 명령어 기준 앞 파이프와 뒤 파이프 구분용 뒤의 인덱스는 입력과 출력 구분용
    pid_t pid;

    for (int i = 0; i < cmd_count; i++) // 토큰 개수만큼 반복
    {
        if (i < cmd_count - 1) // 다음 명령이 있으면 파이프 생성
        {
            if (pipe(pipes[i % 2]) < 0) // 파이프 생성 후 성공 여부 체크
            {
                perror("pipe"); // 오류 메시지 발송
                exit(1);
            }
        }

        pid = fork(); // 자식 프로세스 생성 후 pid 값 저장
        if (pid < 0) // 오류 체크
        {
            perror("fork"); // 오류 메시지 발송
            exit(1);
        }

        if (pid == 0) // 자식 프로세스 작동
        {
            if (i != 0) // 두번째 토큰부터
            {
                dup2(pipes[(i + 1) % 2][0], 0); // 앞의 파이프를 입력으로 받기
            }
            if (i < cmd_count - 1) // 마지막 명령어 빼고
            {
                dup2(pipes[i % 2][1], 1); // 뒤의 파이프를 출력으로 설정
            }
            // 모든 파이프 닫기
            close(pipes[0][0]); close(pipes[0][1]);
            close(pipes[1][0]); close(pipes[1][1]);

            char *args[10];
            int j = 0;
            char *arg = strtok(cmds[i], " "); // 명령어를 공백으로 파싱하기
            while (arg != NULL) // 명령어가 있으면
            {
                args[j++] = arg; // 명령어 저장
                arg = strtok(NULL, " "); // 공백을 기준으로 토큰 갱신
            }
            args[j] = NULL; // 문자열 끝을 널문자로 채우기

            execvp(args[0], args); // 코드 덮어쓰고 명령어 실행
            perror("execvp"); // 명령어 실행이 안 되었을 시 오류 메시지 발송
            exit(1);
        }
    }

    // 부모는 파이프 닫기
    close(pipes[0][0]); close(pipes[0][1]);
    close(pipes[1][0]); close(pipes[1][1]);

    for (int i = 0; i < cmd_count; i++)
    {
        wait(NULL); // 자식이 모두 끝날 때까지 대기
    }
}
