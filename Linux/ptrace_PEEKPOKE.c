#include <stdio.h>
#include <stdlib.h>
#include <sys/ptrace.h>
#include <sys/user.h>

int main(int argc, char * argv[])
{
	int pid, i, data_read = 0;
	struct user_regs_struct regs = {0,};

	if(argc != 2)
	{
		printf("usage : %s pid\n", argv[0]);
		return 0;
	}
	
	pid = atoi(argv[1]);

	//attach
	if(ptrace(PTRACE_ATTACH, pid, 0, 0) == -1)
	{
		printf("ptrace attach fail\n");
		return 0;
	}

	//대상 프로세스가 attach되고 멈추길 기다림
	wait();

	//레지스터 값 얻기
	ptrace(PTRACE_GETREGS, pid, 0, &regs);
	printf("esp : %p\n", regs.esp);

	//스택에 값 쓰기
	ptrace(PTRACE_POKEDATA, pid, regs.esp, 0xcafebade);

	//스택 값 읽기
	for(i = 0; i<10; i++)
	{
		data_read = ptrace(PTRACE_PEEKDATA, pid, regs.esp - i*4, NULL);
		printf("%08x\n", data_read);
	}	
	
	//detach
	ptrace(PTRACE_DETACH, pid, 0, 0);
}




