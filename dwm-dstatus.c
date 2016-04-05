#define _BSD_SOURCE
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <strings.h>
#include <sys/time.h>
#include <time.h>
#include <sys/types.h>
#include <sys/wait.h>

#include <X11/Xlib.h>

#include "helpers_defs.h"

#define MAX_BUFFER_SIZE 100

char *tzcroatia = "Europe/Zagreb";
char *tzgmt = "GMT";

static Display *dpy;

char *
smprintf(char *fmt, ...)
{
	va_list fmtargs;
	char *ret;
	int len;

	va_start(fmtargs, fmt);
	len = vsnprintf(NULL, 0, fmt, fmtargs);
	va_end(fmtargs);

	ret = malloc(++len);
	if (ret == NULL) {
		perror("malloc");
		exit(1);
	}

	va_start(fmtargs, fmt);
	vsnprintf(ret, len, fmt, fmtargs);
	va_end(fmtargs);

	return ret;
}

void
settz(char *tzname)
{
	setenv("TZ", tzname, 1);
}

char *
mktimes(char *fmt, char *tzname)
{
	char buf[129];
	time_t tim;
	struct tm *timtm;

	memset(buf, 0, sizeof(buf));
	settz(tzname);
	tim = time(NULL);
	timtm = localtime(&tim);
	if (timtm == NULL) {
		perror("localtime");
		exit(1);
	}

	if (!strftime(buf, sizeof(buf)-1, fmt, timtm)) {
		fprintf(stderr, "strftime == 0\n");
		exit(1);
	}

	return smprintf("%s", buf);
}

void
setstatus(char *str)
{
	XStoreName(dpy, DefaultRootWindow(dpy), str);
	XSync(dpy, False);
}

char *
loadavg(void)
{
	double avgs[3];

	if (getloadavg(avgs, 3) < 0) {
		perror("getloadavg");
		exit(1);
	}

	return smprintf("%.2f %.2f %.2f", avgs[0], avgs[1], avgs[2]);
}

void
read_str(const char *path, char *buffer, size_t sz)
{
	FILE *file_handle;
	char ch = 0;
	int idx = 0;

	if (!(file_handle = fopen(path, "r"))) return;

	while ((ch = fgetc(file_handle)) != EOF &&
			ch != '\0' && ch != '\n' && idx < sz) {
		buffer[idx++] = ch;
	}

	buffer[idx] = '\0';

	fclose(file_handle);
}

float
read_float(const char *path, char *buffer, size_t sz)
{
	read_str(path, buffer, sz);
	return atof(buffer);
}

float
get_battery_quality()
{
	char *buffer = malloc(MAX_BUFFER_SIZE);
	float ben_full = read_float(BAT_ENERGY_FULL, buffer, MAX_BUFFER_SIZE);
	float ben_full_design = read_float(BAT_ENERGY_FULL_DESIGN, buffer, MAX_BUFFER_SIZE);

	free(buffer);
	return ben_full/ben_full_design;
}

int
main(void)
{
	char *status;
	char *avgs;
	char *tmcro;
	char *bpct = malloc(MAX_BUFFER_SIZE);
	float bqlty = get_battery_quality();

	if (!(dpy = XOpenDisplay(NULL))) {
		fprintf(stderr, "dwmstatus: cannot open display.\n");
		return 1;
	}

	for (;;sleep(1)) {
		avgs = loadavg();
		read_str(BAT_NOW, bpct, MAX_BUFFER_SIZE);
		tmcro = mktimes("%H:%M:%S", tzcroatia);

		status = smprintf("L:%s T:%s B:%s BQ:%.2f",
				avgs, tmcro, bpct, bqlty);
		setstatus(status);
		free(avgs);
		free(tmcro);
		free(status);
	}

	free(bpct);
	XCloseDisplay(dpy);

	return 0;
}

