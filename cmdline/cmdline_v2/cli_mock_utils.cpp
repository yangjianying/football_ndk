#include <stdio.h>
#include <stdlib.h>
#include <string.h>
//#include <conio.h>  // getch

#include "cli.h"

#if 1  // puts in libc
int cli_puts(const char *s) {
	CLI_UNUSED_(s);
	while(*s) { putchar(*s++); }
	return 0;
}
#endif

void putc(const char c) {
	CLI_UNUSED_(c);
	putchar(c);
}
int getc(void) {
	return getchar();
	//return getch();  // not exist
}

static int ctrlc_was_pressed = 0;
int had_ctrlc (void)
{
	return ctrlc_was_pressed;
}

void clear_ctrlc(void)
{
	ctrlc_was_pressed = 0;
}

char *env_get(char *name) {
	CLI_UNUSED_(name);
	return NULL;
}

