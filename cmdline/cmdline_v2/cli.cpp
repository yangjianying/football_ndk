// SPDX-License-Identifier: GPL-2.0+
/*
 * (C) Copyright 2000
 * Wolfgang Denk, DENX Software Engineering, wd@denx.de.
 *
 * Add to readline cmdline-editing by
 * (C) Copyright 2005
 * JinHua Luo, GuangDong Linux Center, <luo.jinhua@gd-linux.com>
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <stdio.h>
#include <termios.h>
#include <unistd.h>
#include <fcntl.h>

#include "utils/football_debugger.h"

//#include <common.h>
#include "cli.h"
//#include <cli_hush.h>
//#include <console.h>
//#include <env.h>
//#include <fdtdec.h>
//#include <malloc.h>

//DECLARE_GLOBAL_DATA_PTR;

#undef __CLASS__
#define __CLASS__ "cli"

#if 1  // #ifdef CONFIG_CMDLINE
/*
 * Run a command using the selected parser.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * @return 0 on success, or != 0 on error.
 */
int cli::run_command(const char *cmd, int flag)
{
#if 1  // #if !CONFIG_IS_ENABLED(HUSH_PARSER)
	/*
	 * cli_run_command can return 0 or 1 for success, so clean up
	 * its result.
	 */
	if (cli_simple_run_command(cmd, flag) == -1)
		return 1;

	return 0;
#else
	int hush_flags = FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP;

	if (flag & CMD_FLAG_ENV)
		hush_flags |= FLAG_CONT_ON_NEWLINE;
	return parse_string_outer(cmd, hush_flags);
#endif
}

/*
 * Run a command using the selected parser, and check if it is repeatable.
 *
 * @param cmd	Command to run
 * @param flag	Execution flags (CMD_FLAG_...)
 * @return 0 (not repeatable) or 1 (repeatable) on success, -1 on error.
 */
int cli::run_command_repeatable(const char *cmd, int flag)
{
#if 1  // #ifndef CONFIG_HUSH_PARSER
	return cli_simple_run_command(cmd, flag);
#else
	/*
	 * parse_string_outer() returns 1 for failure, so clean up
	 * its result.
	 */
	if (parse_string_outer(cmd,
			       FLAG_PARSE_SEMICOLON | FLAG_EXIT_FROM_LOOP))
		return -1;

	return 0;
#endif
}
#endif /* CONFIG_CMDLINE */

int cli::run_command_list(const char *cmd, int len, int flag)
{
	int need_buff = 1;
	char *buff = (char *)cmd;	/* cast away const */
	int rcode = 0;

	if (len == -1) {
		len = strlen(cmd);
#if 0  // #ifdef CONFIG_HUSH_PARSER
		/* hush will never change our string */
		need_buff = 0;
#else
		/* the built-in parser will change our string if it sees \n */
		need_buff = strchr(cmd, '\n') != NULL;
#endif
	}
	if (need_buff) {
		buff = (char *)malloc(len + 1);
		if (!buff)
			return 1;
		memcpy(buff, cmd, len);
		buff[len] = '\0';
	}
#if 0  // #ifdef CONFIG_HUSH_PARSER
	rcode = parse_string_outer(buff, FLAG_PARSE_SEMICOLON);
#else
	/*
	 * This function will overwrite any \n it sees with a \0, which
	 * is why it can't work with a const char *. Here we are making
	 * using of internal knowledge of this function, to avoid always
	 * doing a malloc() which is actually required only in a case that
	 * is pretty rare.
	 */
#if 1  // #ifdef CONFIG_CMDLINE
	rcode = cli_simple_run_command_list(buff, flag);
#else
	rcode = board_run_command(buff);
#endif
#endif
	if (need_buff)
		free(buff);

	return rcode;
}

/****************************************************************************/

#if 0  // #if defined(CONFIG_CMD_RUN)
int do_run(cmd_tbl_t *cmdtp, int flag, int argc, char * const argv[])
{
	int i;

	if (argc < 2)
		return CMD_RET_USAGE;

	for (i = 1; i < argc; ++i) {
		char *arg;

		arg = env_get(argv[i]);
		if (arg == NULL) {
			DLOGD("## Error: \"%s\" not defined\n", argv[i]);
			return 1;
		}

		if (run_command(arg, flag | CMD_FLAG_ENV) != 0)
			return 1;
	}
	return 0;
}
#endif

#if 0  // #if CONFIG_IS_ENABLED(OF_CONTROL)
bool cli_process_fdt(const char **cmdp)
{
	/* Allow the fdt to override the boot command */
	char *env = fdtdec_get_config_string(gd->fdt_blob, "bootcmd");
	if (env)
		*cmdp = env;
	/*
	 * If the bootsecure option was chosen, use secure_boot_cmd().
	 * Always use 'env' in this case, since bootsecure requres that the
	 * bootcmd was specified in the FDT too.
	 */
	return fdtdec_get_config_int(gd->fdt_blob, "bootsecure", 0) != 0;
}

/*
 * Runs the given boot command securely.  Specifically:
 * - Doesn't run the command with the shell (run_command or parse_string_outer),
 *   since that's a lot of code surface that an attacker might exploit.
 *   Because of this, we don't do any argument parsing--the secure boot command
 *   has to be a full-fledged u-boot command.
 * - Doesn't check for keypresses before booting, since that could be a
 *   security hole; also disables Ctrl-C.
 * - Doesn't allow the command to return.
 *
 * Upon any failures, this function will drop into an infinite loop after
 * printing the error message to console.
 */
void cli_secure_boot_cmd(const char *cmd)
{
#ifdef CONFIG_CMDLINE
	cmd_tbl_t *cmdtp;
#endif
	int rc;

	if (!cmd) {
		DLOGD("## Error: Secure boot command not specified\n");
		goto err;
	}

	/* Disable Ctrl-C just in case some command is used that checks it. */
	disable_ctrlc(1);

	/* Find the command directly. */
#ifdef CONFIG_CMDLINE
	cmdtp = find_cmd(cmd);
	if (!cmdtp) {
		DLOGD("## Error: \"%s\" not defined\n", cmd);
		goto err;
	}

	/* Run the command, forcing no flags and faking argc and argv. */
	rc = (cmdtp->cmd)(cmdtp, 0, 1, (char **)&cmd);

#else
	rc = board_run_command(cmd);
#endif

	/* Shouldn't ever return from boot command. */
	DLOGD("## Error: \"%s\" returned (code %d)\n", cmd, rc);

err:
	/*
	 * Not a whole lot to do here.  Rebooting won't help much, since we'll
	 * just end up right back here.  Just loop.
	 */
	hang();
}
#endif /* CONFIG_IS_ENABLED(OF_CONTROL) */

void cli::cli_loop(const char *prompt_)
{
//	bootstage_mark(BOOTSTAGE_ID_ENTER_CLI_LOOP);
#if 0  // #ifdef CONFIG_HUSH_PARSER
	parse_file_outer();
	/* This point is never reached */
	for (;;);
#elif defined(CONFIG_CMDLINE)
	cli_simple_loop(prompt_);
#else
	DLOGD("## U-Boot command line is disabled. Please enable CONFIG_CMDLINE\n");
#endif /*CONFIG_HUSH_PARSER*/
}

void cli::cli_init(void)
{
#if 0  // #ifdef CONFIG_HUSH_PARSER
	u_boot_hush_start();
#endif

#if 0  // #if defined(CONFIG_HUSH_INIT_VAR)
	hush_init_var();
#endif
}

cli::cli(int flags):mFlags(flags) {
	DLOGD( "cli::%s flags:%08x \r\n", __func__, flags);

	hist = new hist_();
	hist->hist_init();  // frankie, 2020.03.03 move here
	cli_init();

	struct termios newt;
	tcgetattr( STDIN_FILENO, &oldt );

	if ((mFlags & cli_FLAG_no_init_console) == 0) {
		newt = oldt;
		newt.c_lflag &=~(ICANON | ECHO);
		tcsetattr( STDIN_FILENO, TCSANOW, &newt );
	}

	if (mFlags & cli_FLAG_non_blocking_getchar) {
		// set STDIN_FILENO non-blocking
		fcntl(STDIN_FILENO, F_SETFL, fcntl(STDIN_FILENO, F_GETFL, 0) | O_NONBLOCK);
	}

}
cli::~cli() {
	DLOGD( "cli::%s \r\n", __func__);

	tcsetattr( STDIN_FILENO, TCSANOW, &oldt );
		
	delete hist;
}

int cli::cli_intercept_command_repeatable(const char *cmd, int flag) {
	CLI_UNUSED_(cmd);
	CLI_UNUSED_(flag);
	return 0;
}

int cli::cli_cmd_process_(int flag, int argc, char * const argv[],
			       int *repeatable, ulong *ticks) {
	CLI_UNUSED_(flag);
	CLI_UNUSED_(argc);
	CLI_UNUSED_(argv);
	CLI_UNUSED_(repeatable);
	CLI_UNUSED_(ticks);
	return 0;
}
void cli::cli_cmd_empty() {
}

int cli::run_cli_command(const char *command_) {
	int flag = 0;
	char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0 };
	strlcpy(lastcommand, command_, CONFIG_SYS_CBSIZE + 1);
	int rc = run_command_repeatable(lastcommand, flag);
	return rc;
}

int cli::check_cli_command_matched(const char *command_, const char *cmd_) {
	int flag = 0;
	char lastcommand[CONFIG_SYS_CBSIZE + 1] = { 0 };
	strlcpy(lastcommand, command_, CONFIG_SYS_CBSIZE + 1);
	int rc = cli_simple_check_command(lastcommand, cmd_, flag);
	if (rc <= 0) {
		return 0;
	}
	return 1;
}

/* static */ void cli::test() {
	cli *c = new cli();
	c->cli_init();
	c->cli_loop("test>>");
	delete c;
}

