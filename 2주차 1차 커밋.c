#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

void pwd(void);
void show_prompt(void);




int main(void)
{
    char line[1024];
    while(1)
    {
        show_prompt(); //리눅스쉘 형태 구현
        fgets(line, sizeof(line), stdin);  //사용자로부터 입력 받기
        if (strcmp(line, "exit\n") == 0) //exit 입력 받으면 반복문 종료 
        break;
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
 printf("%s$ \n", cwd);
}