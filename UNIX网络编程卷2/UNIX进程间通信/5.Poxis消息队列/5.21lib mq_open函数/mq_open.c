/* include mq_open1 */
#include	"unpipc.h"
#include	"mqueue.h"

#include	<stdarg.h>
#define		MAX_TRIES	10	/* for waiting for initialization */

struct mymq_attr	defattr = { 0, 128, 1024, 0 };

mymqd_t
mymq_open(const char *pathname, int oflag, ...)
{
	int		i, fd, nonblock, created, save_errno;
	long	msgsize, filesize, index;
	va_list	ap;
	mode_t	mode;
	int8_t	*mptr; //共享内存区mmap的指针
	struct stat	statbuff; //文件属性
	struct mymq_hdr	*mqhdr; //整个消息队列链表的控制信息结构体(该结构体位于mmap共享存储文件的开始处)
	struct mymsg_hdr	*msghdr; //消息队列每条消息的控制信息结构体(该结构位于每条消息的开始处)
	struct mymq_attr	*attr; //消息队列的属性
	struct mymq_info	*mqinfo; //mq_open每次调用返回给调用者的结构体(也就是消息队列描述符)
	pthread_mutexattr_t	mattr;
	pthread_condattr_t	cattr;

	created = 0; //标志量,消息队列是否已经创建,初始化位0,表示消息队列未创建
	nonblock = oflag & O_NONBLOCK; //标志量,消息队列是否时非阻塞的,调用者的oflag中开启O_NONBLOCK,初始化为1,否则初始化为0
	oflag &= ~O_NONBLOCK; //oflag中去掉非阻塞位
	mptr = (int8_t *) MAP_FAILED; //mptr值初始化为还没有进行mmap映射
	mqinfo = NULL; //mqinfo结构表示消息队列的描述符,初始化为NULL
again:
	if (oflag & O_CREAT) { 	//调用者要求创建消息队列
		va_start(ap, oflag);		/* init ap to final named argument 初始化va_list结构体,使其存放变长参数于其中 */

		//va_arg宏返回变长参数的第一个参数,并把指针向后移动一个"va_mode_t"的距离
		//va_mode_t是为了可移植性而自行生成的变量,mode参数为unsigned short,为16位整型
		//有些系统的编译器会把mode扩展为32位整型
		//如果在这种情况下,指针向后移动mode的距离,那就是移动了16位,对于扩展成32位的系统,那就不能有效的指向下一个可变参数
		//所以我们把va_mode_t类型，在扩展成32位的系统上,定义为int,在不扩展成32位的系统上,定义为unsigned short,达到可移植的目的

		//可变参数的第一个参数为创建消息队列的权限位,我们去除消息队列的"用户可执行位"
		//因为创建一个消息队列(只不过创建一个文件用来用作mmap的映射而已)和初始化一个消息队列这两个动作并不是原子进行的
		//所以在创建一个消息队列之后,打开其用户执行位
		//表示这个消息队列还没有初始化,在初始化消息队列之后,再关闭其用户执行位,表示这个消息队列初始化好了。
		//所以,在此地,我们预先关闭第三个参数的用户执行位,以免产生语义上得分歧
		mode = va_arg(ap, va_mode_t) & ~S_IXUSR; 
		//可变参数的第二个参数为消息队列的属性
		attr = va_arg(ap, struct mymq_attr *);
		va_end(ap); //va_list结构体使用完了,释放这个结构体所占用的资源

			/* 4open and specify O_EXCL and user-execute */
		//打开调用者所要求的路径,以调用者指定的模式,外加读写,排他(用来验证文件是否由本线程创建)模式
		//再加上调用者附上的权限位外加执行权限位(表示消息队列还没有初始化)来打开一个文件(该文件用来mmap映射)

		//调用者在oflag中可能指定O_CREAT,O_CREAT|O_EXCL,没有指定O_CREAT,O_EXCL这两个标志中的任何一个三种情况
		//如果调用者在oflag中只指定了O_CREAT:语义为如果消息队列不存在则创建,如果存在则打开这个消息队列
		//如果调用者在oflag中只指定了O_CREAT|O_EXCL,那么调用者必须创建一个消息队列,如存在则返回错误
		//如果调用者没有指定O_CREAT,O_EXCL这两个标志中的任何一个,则调用者不创建消息队列,只是单纯的打开一个消息队列
		//所以,为了符合这三种语义,在open返回EEXIST排他性错误时,我们检测oflag中是否开启O_EXCL位,如果oflag中开启了O_EXCL位
		//则说明调用者希望自己创建一个消息队列,现在文件已存在,我们返回调用者-1,表示错误
		//如果调用者的oflag中没有指定O_EXCL位,那就跳到exists去直接打开这个消息队列,就当O_CREAT位从来没有指定过
		fd = open(pathname, oflag | O_EXCL | O_RDWR, mode | S_IXUSR);
		if (fd < 0) {
			if (errno == EEXIST && (oflag & O_EXCL) == 0) //这个O_EXCL是调用者指定的,那就出错,是我们自己指定的,就打开这个已存在的消息队列
				goto exists;		/* already exists, OK */
			else
				return((mymqd_t) -1);
		}
		created = 1; //标志量置为1,表示调用者要求创建消息队列
			/* 4first one to create the file initializes it */
		if (attr == NULL) //调用者如果传参进来的消息队列属性为NULL
			attr = &defattr; //我们赋予它默认的消息队列属性
		else { //否则,此分支代表调用者指定了消息队列的属性
				//我们呢检查最大消息数量,没每个消息的最大大小这两个值,这两个值必须同时指定,否则出错
			if (attr->mq_maxmsg <= 0 || attr->mq_msgsize <= 0) {
				errno = EINVAL;
				goto err;
			}
		}
/* end mq_open1 */
/* include mq_open2 */
			/* 4calculate and set the file size */
		msgsize = MSGSIZE(attr->mq_msgsize); //每个消息的大小,向上取整到一个long的倍数的大小
		//文件的大小等于文件开头结构体mymq_hdr(用来控制整个消息队列链表的结构体)+最大消息数目*每条消息的大小
		//其中,每条消息大小=mymsg_hdr(每条消息开头的控制结构体)+消息正文本身的大小(以向上取整为sizeof(long)的倍数)
		filesize = sizeof(struct mymq_hdr) + (attr->mq_maxmsg *
				   (sizeof(struct mymsg_hdr) + msgsize)); 
		//用lseek设置文件大小,SEEK_SET宏表示从文件开始处计算(少一个字节是因为后面要写一个字节来扩展文件大小,写一个字节之后,文件指针会再向后移动一位)
		if (lseek(fd, filesize - 1, SEEK_SET) == -1) 
			goto err;
		if (write(fd, "", 1) == -1) //当前文件指针的位置写一个字节,此时文件指针向后移动一位,正好构成一个filesize大小的一个文件
			goto err;

			/* 4memory map the file */
		mptr = mmap(NULL, filesize, PROT_READ | PROT_WRITE,
					MAP_SHARED, fd, 0); //映射文件,模式为读写,映射大小为filesize,拥有进程间共享属性
		if (mptr == MAP_FAILED) //这个值是初始化时候赋予的,如果现在还等于这个值,代表mmap返回了NULL,出错了
			goto err;

			/* 4allocate one mymq_info{} for the queue */
/* *INDENT-OFF* */
		if ( (mqinfo = malloc(sizeof(struct mymq_info))) == NULL) //动态分配为每次调用mq_open而返回给调用者的结构体(消息队列描述符)
			goto err;
/* *INDENT-ON* */

		//初始化"消息队列描述符结构体""

		//把共享存储区的指针分别填入"消息队列描述符结构体的相应成员"中
		//和我们维护的"整个消息队列链表的控制信息结构体"(这个结构体位于整个mmap映射地址的开始处,所以如此附值是合理的)
		mqinfo->mqi_hdr = mqhdr = (struct mymq_hdr *) mptr; 
		mqinfo->mqi_magic = MQI_MAGIC; //设置宏,代表改结构体已经使用
		mqinfo->mqi_flags = nonblock; //把是否阻塞的标志填入"消息队列描述符结构体的相应成员"

			/* 4initialize header at beginning of file */
			/* 4create free list with all messages on it */
		//初始化"整个消息队列链表的控制信息结构体"

		mqhdr->mqh_attr.mq_flags = 0; //标志初始化为0
		mqhdr->mqh_attr.mq_maxmsg = attr->mq_maxmsg; //这个消息队列最多有多少条消息
		mqhdr->mqh_attr.mq_msgsize = attr->mq_msgsize; //这个消息队列,每条消息的最大大小
		mqhdr->mqh_attr.mq_curmsgs = 0; //当前消息队列中有多少条消息(刚刚用mp_open打开,所以当前消息队列中的消息数一定为0)
		mqhdr->mqh_nwait = 0; //阻塞于mq_receive调用中的进程或线程个数
		mqhdr->mqh_pid = 0; //如果注册进程接受通知,则此成员保存接收消息进程的pid
		mqhdr->mqh_head = 0; //消息链表头初始化为0
		index = sizeof(struct mymq_hdr); //链表中消息的索引指针:索引指针指向"整个消息队列链表的控制信息结构体"的最后一个字节(第一个消息的开始处)
		//把所有消息都加到空闲链表中(index的属性其实是一个指针的加减值)
		mqhdr->mqh_free = index; //空闲链表的指针索引指向"整个消息队列链表的控制信息结构体"最后一个字节(第一个消息的开始处)
		for (i = 0; i < attr->mq_maxmsg - 1; i++) { //初始化"空闲消息链表"的指针(循环中没有初始化最后一个消息)
			msghdr = (struct mymsg_hdr *) &mptr[index];
			index += sizeof(struct mymsg_hdr) + msgsize;
			msghdr->msg_next = index;
		}
		//初始化消息链表中最后一个消息
		msghdr = (struct mymsg_hdr *) &mptr[index];
		msghdr->msg_next = 0;		/* end of free list */

			/* 4initialize mutex & condition variable 初始化互斥量*/
		if ( (i = pthread_mutexattr_init(&mattr)) != 0) //初始化互斥量属性结构体
			goto pthreaderr;
		pthread_mutexattr_setpshared(&mattr, PTHREAD_PROCESS_SHARED); //给互斥量属性加上进程间共享属性
		i = pthread_mutex_init(&mqhdr->mqh_lock, &mattr); //用初始化好的属性,初始化互斥量
		pthread_mutexattr_destroy(&mattr);	/* be sure to destroy 销毁互斥量属性结构体*/
		if (i != 0) //初始化互斥量失败的情况下
			goto pthreaderr;
		//初始化条件变量
		if ( (i = pthread_condattr_init(&cattr)) != 0) //初始化条件变量属性结构体
			goto pthreaderr;
		pthread_condattr_setpshared(&cattr, PTHREAD_PROCESS_SHARED); //为条件变量属性加入进程间共享属性
		i = pthread_cond_init(&mqhdr->mqh_wait, &cattr); //初始化条件变量
		pthread_condattr_destroy(&cattr);	/* be sure to destroy 销毁条件变量属性结构体*/
		if (i != 0) //初始化条件变量失败的情况下
			goto pthreaderr;

			/* 4initialization complete, turn off user-execute bit */
		if (fchmod(fd, mode) == -1) //初始化完毕,关闭用户执行位
			goto err;
		close(fd); //关闭文件(已经映射完了,文件没用了)
		return((mymqd_t) mqinfo); //返回调用者消息队列描述符
	}
/* end mq_open2 */
/* include mq_open3 */
exists:
		/* 4open the file then memory map */
		//代码走到这里,要么O_CREAT标志未指定,要么O_CREAT标志指定了,但是队列已存在
	if ( (fd = open(pathname, O_RDWR)) < 0) { //已读写的方式打开文件(再其他进程中,mmap映射的文件)
		if (errno == ENOENT && (oflag & O_CREAT)) //如果打开失败,切调用者指定了O_CREAT标志
			goto again; //返回指定O_CREAT标志的if语句处
		goto err; //open出错,且调用者没有指定O_CREAT标志
	}

		/* 4make certain initialization is complete */
	//检查消息队列是否被初始化,检查的次数为MAX_TRIES
	for (i = 0; i < MAX_TRIES; i++) { 
		if (stat(pathname, &statbuff) == -1) { //获得文件的属性,填写入statbuff结构体
			if (errno == ENOENT && (oflag & O_CREAT)) { //文件路经不存在,且调用者指定了O_CREAT标志
				close(fd); //关闭该文件
				goto again; //返回指定O_CREAT标志的if语句处
			}
			goto err; //否则出错
		}
		if ((statbuff.st_mode & S_IXUSR) == 0) //检查文件属性结构体中的模式成员,如果有执行位则说明消息队列已经初始化,break出循环
			break;
		sleep(1); //都则睡一秒后进行下一次检验
	}
	if (i == MAX_TRIES) { //此条件代表循环已经检查了//返回指定O_CREAT标志的if语句处次却仍然没有执行位(仍然没有初始化消息队列完成完成)
		errno = ETIMEDOUT; //出错
		goto err;
	}

	filesize = statbuff.st_size; //给维护文件大小的变量附值
	mptr = mmap(NULL, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0); //按这个文件大小读写方式映射文件,属性为进程间共享
	if (mptr == MAP_FAILED) //这个值是初始化时候赋予的,如果现在还等于这个值,代表mmap返回了NULL,出错了
		goto err;
	close(fd); //映射完成,关闭这个文件

		/* 4allocate one mymq_info{} for each open */
/* *INDENT-OFF* */
	if ( (mqinfo = malloc(sizeof(struct mymq_info))) == NULL) //动态分配为每次调用mq_open而返回给调用者的结构体(消息队列描述符)
		goto err;
/* *INDENT-ON* 初始化消息队列描述符结构体 */

	//把共享存储区的指针分别填入"消息队列描述符结构体的相应成员"中
	//和我们维护的"整个消息队列链表的控制信息结构体"(这个结构体位于整个mmap映射地址的开始处,所以如此附值是合理的)
	mqinfo->mqi_hdr = (struct mymq_hdr *) mptr; 
	mqinfo->mqi_magic = MQI_MAGIC; //设置宏,代表改结构体已经使用
	mqinfo->mqi_flags = nonblock; //把是否阻塞的标志填入"消息队列描述符结构体的相应成员"
	return((mymqd_t) mqinfo); //返回调用者"消息队列描述符结构体"
/* $$.bp$$ */
pthreaderr:
	errno = i; //把线程函数返回的错误码附值给errno
err:
		/* 4don't let following function calls change errno */
	save_errno = errno; //备份errno
	if (created) //如果文件已经创建
		unlink(pathname); //删除这个文件
	if (mptr != MAP_FAILED) //如文件已经映射
		munmap(mptr, filesize); //解除这个文件的映射
	if (mqinfo != NULL) //如果"返回消息队列的描述符已经动态分配"
		free(mqinfo); //释放这个消息队列描述符的空间
	close(fd); //关闭这个文件
	errno = save_errno; //把备份的errno附值回去
	return((mymqd_t) -1); //返回调用者-1代表出错
}
/* end mq_open3 */
//包裹函数
mymqd_t
Mymq_open(const char *pathname, int oflag, ...)
{
	mymqd_t	mqd;
	va_list	ap;
	mode_t	mode;
	struct mymq_attr	*attr;

	if (oflag & O_CREAT) { //如果调用者指定了O_CREAT标志
		va_start(ap, oflag);		/* init ap to final named argument 把变长参数存入ap中*/
		mode = va_arg(ap, va_mode_t); //取出第一个变长参数的值,长度为va_mode_t
		attr = va_arg(ap, struct mymq_attr *); //取出第二个变长参数,长度为struct mymq_attr *
		if ( (mqd = mymq_open(pathname, oflag, mode, attr)) == (mymqd_t) -1) //参数齐整了,调用mymq_open
			err_sys("mymq_open error for %s", pathname);
		va_end(ap); //释放ap结构体
	} else {
		if ( (mqd = mymq_open(pathname, oflag)) == (mymqd_t) -1) //没有O_CREAT标记就不可能有变长参数
			err_sys("mymq_open error for %s", pathname);
	}
	return(mqd); //返回消息队列描述符
}
