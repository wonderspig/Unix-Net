/* include rtt1 */
#include	"unprtt.h"

int		rtt_d_flag = 0;		/* debug flag; can be set by caller */

/*
 * Calculate the RTO value based on current estimators:
 *		smoothed RTT plus four times the deviation
 */
 //RTT_RTOCALC宏用rtt估算因子加上4倍平均偏差估算因子计算出RTO
#define	RTT_RTOCALC(ptr) ((ptr)->rtt_srtt + (4.0 * (ptr)->rtt_rttvar))
//rtt_minmax函数确保RTO在unprtt.h头文件中定义的上下界
static float
rtt_minmax(float rto)
{
	if (rto < RTT_RXTMIN)
		rto = RTT_RXTMIN;
	else if (rto > RTT_RXTMAX)
		rto = RTT_RXTMAX;
	return(rto);
}
//rtt_init函数在首次发送分组前调用
void
rtt_init(struct rtt_info *ptr)
{
	struct timeval	tv;

	Gettimeofday(&tv, NULL); //返回当前时间,填写在tv结构体中
	ptr->rtt_base = tv.tv_sec;		/* # sec since 1/1/1970 at start 只保存秒数*/

	ptr->rtt_rtt    = 0; //计算出RTT去服务器的往返时间初始值为0
	ptr->rtt_srtt   = 0; //RTT估算因子
	ptr->rtt_rttvar = 0.75; //RTT平均偏差因子
	ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr)); //通过估算因子和平均偏差因子算出RTO重传超时时间为3秒
		/* first RTO at (srtt + (4 * rttvar)) = 3 seconds */
}
/* end rtt1 */

/*
 * Return the current timestamp.
 * Our timestamps are 32-bit integers that count milliseconds since
 * rtt_init() was called.
 */

/* include rtt_ts */
//rtt_ts函数返回当前的时间戳(返回的是毫秒单位)
uint32_t
rtt_ts(struct rtt_info *ptr)
{
	uint32_t		ts;
	struct timeval	tv;

	Gettimeofday(&tv, NULL); //获得当前时间
	//返回值的计算方式为:
	//当前的时间-init时候的时间(秒钟换算成毫秒)+当前时间戳的微秒数(转换成毫秒)
	ts = ((tv.tv_sec - ptr->rtt_base) * 1000) + (tv.tv_usec / 1000);
	return(ts);
}
//rtt_newpack函数把重传计数器设置为0
//每次发送一个新的分组时,都应该调用这个函数
void
rtt_newpack(struct rtt_info *ptr)
{
	ptr->rtt_nrexmt = 0;
}
//rtt_start函数以秒为单位返回当前RTO,超时重传时间
int
rtt_start(struct rtt_info *ptr)
{
	return((int) (ptr->rtt_rto + 0.5));		/* round float to int 四舍五入转换成int */
		/* 4return value can be used as: alarm(rtt_start(&foo)) */
}
/* end rtt_ts */

/*
 * A response was received.
 * Stop the timer and update the appropriate values in the structure
 * based on this packet's RTT.  We calculate the RTT, then update the
 * estimators of the RTT and its mean deviation.
 * This function should be called right after turning off the
 * timer with alarm(0), or right after a timeout occurs.
 */

/* include rtt_stop */
//rtt_stop函数在收到应答之后更新估算因子,并计算新的RTO
void
rtt_stop(struct rtt_info *ptr, uint32_t ms)
{
	double		delta;

	ptr->rtt_rtt = ms / 1000.0;		/* measured RTT in seconds 把RTT转换成秒*/

	/*
	 * Update our estimators of RTT and mean deviation of RTT.
	 * See Jacobson's SIGCOMM '88 paper, Appendix A, for the details.
	 * We use floating point here for simplicity.
	 */

	delta = ptr->rtt_rtt - ptr->rtt_srtt;
	ptr->rtt_srtt += delta / 8;		/* g = 1/8 */

	if (delta < 0.0)
		delta = -delta;				/* |delta| */

	ptr->rtt_rttvar += (delta - ptr->rtt_rttvar) / 4;	/* h = 1/4 */

	ptr->rtt_rto = rtt_minmax(RTT_RTOCALC(ptr)); //更新TRO的值
}
/* end rtt_stop */

/*
 * A timeout has occurred.
 * Return -1 if it's time to give up, else return 0.
 */

/* include rtt_timeout */
//rtt_timeout函数在重传定时器到时时调用
int
rtt_timeout(struct rtt_info *ptr)
{
	ptr->rtt_rto *= 2;		/* next RTO 把重传时间加倍 */

	if (++ptr->rtt_nrexmt > RTT_MAXNREXMT) //如果重传次数超过最大重传次数
		return(-1);			/* time to give up for this packet */
	return(0);
}
/* end rtt_timeout */

/*
 * Print debugging information on stderr, if the "rtt_d_flag" is nonzero.
 */
//rtt_debug负责调试信息的打印
void
rtt_debug(struct rtt_info *ptr)
{
	if (rtt_d_flag == 0)
		return;

	fprintf(stderr, "rtt = %.3f, srtt = %.3f, rttvar = %.3f, rto = %.3f\n",
			ptr->rtt_rtt, ptr->rtt_srtt, ptr->rtt_rttvar, ptr->rtt_rto);
	fflush(stderr);
}
