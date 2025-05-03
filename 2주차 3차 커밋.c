#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void pwd(void);
void show_prompt(void);
void cd(char *[]);



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
        char *token = strtok(line, " "); // 첫 토큰
        while (token != NULL) //token에 주소가 저장되었는지 체크
        {
            input[i++] = token;           // 토큰 포인터 저장
            token = strtok(NULL, " ");    // 다음 토큰
        }
        input[i] = NULL; //문자열의 끝인 것을 표시
        if (strcmp(input[0], "exit") == 0) //exit 입력 받으면 반복문 종료 
        { 
            break;
        }
        else if(strcmp(input [0], "pwd") == 0) //pwd 실행
        {
            pwd();
        }   
        else if(strcmp(input [0], "cd") == 0)
        {
            cd(input);

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
            perror("cd error"); // 디렉토리 미존재 시 경고고
         }
    }

}