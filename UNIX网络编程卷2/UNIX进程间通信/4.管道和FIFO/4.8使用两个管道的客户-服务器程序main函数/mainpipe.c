#include	"unpipc.h"
//定义两个函数,client用于完成客户的操作,server用于完成服务器的操作
void	client(int, int), server(int, int); 

int
main(int argc, char **argv)
{
	int		pipe1[2], pipe2[2]; 
	pid_t	childpid;
	//创建两个半双工管道,一个用于读，一个用于写
	Pipe(pipe1);	/* create two pipes */
	Pipe(pipe2);
	//创建子进程
	if ( (childpid = Fork()) == 0) {		/* child 再子进程中 */
		Close(pipe1[1]); //关闭管道1的写端,管道1只用来读
		Close(pipe2[0]); //关闭管道2的读端,管道2只用来写
		//子进程为服务器进程,从管道1读取文件路径并打开它,读出这个文件的内容并写到管道2中
		server(pipe1[0], pipe2[1]); 
		exit(0); 
	}
		/* 4parent 在父进程中*/
	Close(pipe1[0]); //关闭管道1的读端,管道1只用来写
	Close(pipe2[1]); //关闭管道2的写端,管道2只用来读
	//父进程为客户进程,把文件名写入管道1,等待服务器读取文件内容并从管道2把文件内容传送过来
	client(pipe2[0], pipe1[1]);

	Waitpid(childpid, NULL, 0);		/* wait for child to terminate 回收子进程的资源 */
	exit(0);
}
