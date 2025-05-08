#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>


void pwd(void);
void show_prompt(void);
void cd(char *[]);
void run_pipeline(char *);


int main(void)
{
    char line[1024];
    char *input[64];
    while(1)
    {
        int i = 0;
        show_prompt(); //리눅스쉘 형태 구현
        fgets(line, sizeof(line), stdin);  //사용자로부터 입력 받기
        line[strcspn(line, "\n")] = 0; //개행문자 제거
        if (strchr(line, '|') != NULL) //파이프 있는 지 체크
        {
            run_pipeline(line);
            continue;  // 파이프 처리 끝나면 다음 루프
        }
        char *cmds[10];
        int cmd_count = 0;
        char *cmd = strtok(line, ";"); 
        while (cmd != NULL && cmd_count < 10) 
        {
            cmds[cmd_count++] = cmd;    // 잘린 명령어 저장
            cmd = strtok(NULL, ";");   // 다음 명령어 잘라내기
        }
        for (i = 0; i < cmd_count; i++) // 명령어 개수만큼 반복
        {
            char *token = strtok(cmds[i], " "); //공백 기준으로 파싱
            int j = 0;
            while (token != NULL) 
            {
                input[j++] = token;
                token = strtok(NULL, " ");
            }
            input[j] = NULL;
            if (input[0] == NULL) 
            continue;
            if (strcmp(input[0], "exit") == 0) 
            {
                exit(0);
            } 
            else if (strcmp(input[0], "pwd") == 0) 
            {
                pwd();
            } 
            else if (strcmp(input[0], "cd") == 0) 
            {
                cd(input);
            } 
            else //외부 명령어
            {
                pid_t pid = fork();
                if (pid < 0) 
                {
                    perror("fork");
                } 
                else if (pid == 0) 
                {
                    execvp(input[0], input);
                    perror("execvp");
                    exit(1);
                } 
                else 
                {
                    wait(NULL);
                }
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
    char *user = getenv("USER"); //사용자 이름 저장
    gethostname(hostname, sizeof(hostname)); //호스트 이름 저장
    getcwd(cwd, sizeof(cwd)); //현재 디렉토리 저장
    if(home && strstr(cwd, home) == cwd) // 홈디렉토리 또는 그 하위 디렉토리인지 체크
    {
        printf("%s@%s:~%s$ ", user, hostname, cwd + strlen(home)); // home을 ~로 바꾸고 뒤 주소 출력
    }
    else
    { 
        printf("%s@%s:%s$ ", user, hostname, cwd); //주소 그대로 출력
    }


}


void pwd(void)
{
 char cwd[1024];
 getcwd(cwd, sizeof(cwd));
 printf("%s \n", cwd);
}

void cd(char *input[])
{
    if (input[1] == NULL) // 뒤 명령어 없음
    {
        chdir(getenv("HOME")); // 홈디렉토리로 이동
    } 
    else
    {
        if (chdir(input[1]) != 0) // 디렉토리 이동 및 체크
         {
            perror("cd error"); // 디렉토리 미존재 시 경고
         }
    }

}

void run_pipeline(char *line)
{
    char *cmds[10]; 
    int cmd_count = 0;
    char *cmd = strtok(line, "|"); //파이프를 기준으로 line 분할
    while (cmd != NULL && cmd_count < 10)
    {
        cmds[cmd_count++] = cmd; //cmds에 파이프를 기준으로 명령어의 주소를 저장함
        cmd = strtok(NULL, "|"); //파이프 뒤의 명령어의 주소로 cmd를 갱신
    }

    int pipes[2][2];  // 앞의 인덱스는 명령어 기준 앞 파이프와 뒤 파이프 구분용 뒤의 인덱스는 입력과 출력 구분용
    pid_t pid;
    
    for (int i = 0; i < cmd_count; i++)
    {
        if (i < cmd_count - 1) // 다음 명령이 있으면 파이프 생성
        {
            if (pipe(pipes[i % 2]) < 0) 
            {
                perror("pipe");
                exit(1);
            }
        }

        pid = fork();
        if (pid < 0) //오류 체크
        {
            perror("fork");
            exit(1);
        }

        if (pid == 0) // 자식 프로세스 작동
        { 
            if (i != 0) 
            {
                dup2(pipes[(i + 1) % 2][0], 0); // 앞의 파이프를 입력으로 받기 
            }
            if (i < cmd_count - 1) 
            {
                dup2(pipes[i % 2][1], 1); // 뒤의 파이프를 출력으로 받기
            }
            // 모든 파이프 닫기
            close(pipes[0][0]); close(pipes[0][1]);
            close(pipes[1][0]); close(pipes[1][1]);

            char *args[10];
            int j = 0;
            char *arg = strtok(cmds[i], " "); // 명령어를 공백으로 파싱하기
            while (arg != NULL) 
            {
                args[j++] = arg;
                arg = strtok(NULL, " ");
            }
            args[j] = NULL; // 문자열 끝을 널문자로 구분

            execvp(args[0], args);
            perror("execvp");
            exit(1);
        }
    }

    // 부모는 파이프 닫기
    close(pipes[0][0]); close(pipes[0][1]);
    close(pipes[1][0]); close(pipes[1][1]);

    for (int i = 0; i < cmd_count; i++) 
    {
        wait(NULL); //자식 모두 끝날 때까지 대기
    }
}
