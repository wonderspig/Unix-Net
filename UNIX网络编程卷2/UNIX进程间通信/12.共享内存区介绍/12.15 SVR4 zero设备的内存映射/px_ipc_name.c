/* include px_ipc_name */
#include	"unpipc.h"

char *
px_ipc_name(const char *name)
{
	char	*dir, *dst, *slash;
	//PATH_MAX为路径名的最大长度
	if ( (dst = malloc(PATH_MAX)) == NULL) //创建一个能够存放最长文件名地址的内存空间
		return(NULL); //申请失败返回NULL

		/* 4can override default directory with environment variable */
	if ( (dir = getenv("PX_IPC_NAME")) == NULL) { //环境变量PX_IPC_NAME是否存在
#ifdef	POSIX_IPC_PREFIX //如果环境变量PX_IPC_NAME存在,且定义了POSIX_IPC_PREFIX宏
		dir = POSIX_IPC_PREFIX;		/* from "config.h" */
#else //没有定义POSIX_IPC_PREFIX宏,则默认配置
		dir = "/tmp/";				/* default */
#endif
	}
		/* 4dir must end in a slash */
	//如果dir字符串中,的最后一个字符是"/",则slash变量没有字符,否则slash变量为"/"
	slash = (dir[strlen(dir) - 1] == '/') ? "" : "/";
	//到这里的语义为把字符串格式化成: dir取到的路径名,如果是以"/"结尾的,则直接把传参进来的字符串拼接在dir之后
	//否则在dir的后面加上一个"/",再把传参进来的字符串拼接在dir之后
	snprintf(dst, PATH_MAX, "%s%s%s", dir, slash, name);

	return(dst);			/* caller can free() this pointer 返回格式化好的字符串dst */
}
/* end px_ipc_name */

char *
Px_ipc_name(const char *name)
{
	char	*ptr;

	if ( (ptr = px_ipc_name(name)) == NULL)
		err_sys("px_ipc_name error for %s", name);
	return(ptr);
}
