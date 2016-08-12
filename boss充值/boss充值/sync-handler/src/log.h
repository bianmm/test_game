#ifndef _LOG_H
#define _LOG_H

#include <sys/types.h>

__BEGIN_DECLS
#if __GNUC__ == 2 && __GNUC_MINOR__ < 96
#define __builtin_expect(x, expected_value) (x)
#endif

#ifndef likely
#define likely(x)  __builtin_expect(!!(x), 1)
#endif
#ifndef unlikely
#define unlikely(x)  __builtin_expect(!!(x), 0)
#endif

#define APP_EMERG 		0  /* system is unusable               */
#define APP_ALERT		1  /* action must be taken immediately */
#define APP_CRIT		2  /* critical conditions              */
#define APP_ERROR		3  /* error conditions                 */
#define APP_WARNING		4  /* warning conditions               */
#define APP_NOTICE		5  /* normal but significant condition */
#define APP_INFO		6  /* informational                    */
#define APP_DEBUG		7  /* debug-level messages             */
#define APP_TRACE		8  /* trace-level messages             */

#define DETAIL(level, fmt, args...) \
	write_log (level, "[%s][%d]%s: " fmt "\n", __FILE__, __LINE__, __FUNCTION__, ##args)

#define SIMPLY(level, fmt, args...) write_log(level, fmt "\n", ##args)

#define ERROR_LOG(fmt, args...)	DETAIL(APP_ERROR, fmt, ##args)
#define CRIT_LOG(fmt, args...)	DETAIL(APP_CRIT, fmt, ##args)
#define ALERT_LOG(fmt, args...)	DETAIL(APP_ALERT, fmt, ##args)
#define EMERG_LOG(fmt, args...)	DETAIL(APP_EMERG, fmt, ##args)

#define TRACE_LOG(fmt,args...)	SIMPLY(APP_TRACE, fmt, ##args)
#define WARN_LOG(fmt, args...)	SIMPLY(APP_WARNING, fmt, ##args)
#define NOTI_LOG(fmt, args...)	SIMPLY(APP_NOTICE, fmt, ##args)
#define INFO_LOG(fmt, args...)	SIMPLY(APP_INFO, fmt, ##args)
#define DEBUG_LOG(fmt, args...)	SIMPLY(APP_DEBUG, fmt, ##args)
#define BOOT_LOG(OK, fmt, args...) do{ \
	boot_log(OK, 0, fmt, ##args); \
	return OK; \
}while (0)

/**
 * @brief
 * @fn
 *
 * @param
 * @param
 * @param
 * @return 0 on success,-1 on error
 */
int log_init (const char* dir, int lvl, u_int size, const char* pre_name);

void write_log (int lvl, const char* fmt, ...);

/**
 * @brief
 * @fn
 *
 * @param  0 if boot OK,others if failure
 * @param
 * @param
 * @return 0 on success,-1 on error
 */
void boot_log (int OK, int dummy, const char* fmt, ...);

__END_DECLS
#endif
