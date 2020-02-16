#ifndef __CLI_PREDEF_H__
#define __CLI_PREDEF_H__

#include <stdio.h>
#include <termios.h>
#include <unistd.h>

#undef CLI_UNUSED_
#define CLI_UNUSED_(x) ((void)x)

#define __maybe_unused

typedef uint64_t ulong;

#define debug(fmt, args...) do{}while(0)

#define debug_cond(cond, fmt, args...)			\
	do {						\
		if (1) {					\
			/*log(LOG_CATEGORY, LOGL_DEBUG, fmt, ##args); */ \
		}											\
	} while (0)


#include <ctype.h>

#if 0
/*
out/soong/.intermediates/bionic/libc/libc.llndk/android_arm64_armv8-a_kryo300_vendor_shared/gen/include/ctype.h:66:5: note: to match this '('
int isblank(int __ch);
*/
/*
 * Rather than doubling the size of the _ctype lookup table to hold a 'blank'
 * flag, just check for space or tab.
 */
#define isblank(c)	(c == ' ' || c == '\t')
#endif


#define CONFIG_CMDLINE

#define CONFIG_SYS_MAXARGS (64)
#define CONFIG_SYS_CBSIZE (1024)
#define CONFIG_SYS_PROMPT "cmdtool>>"


/*
 * Command Flags:
 */
#define CMD_FLAG_REPEAT		0x0001	/* repeat last command		*/
#define CMD_FLAG_BOOTD		0x0002	/* command is from bootd	*/
#define CMD_FLAG_ENV		0x0004	/* command is from the environment */


int cli_puts(const char *s);  // in libc
void putc(const char c);
int getc(void);
int had_ctrlc (void);
void clear_ctrlc(void);

char *env_get(char *name);

#define puts cli_puts

#endif

