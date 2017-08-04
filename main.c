
#include <linux/rfkill.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>


#define LEDNAME "asus-wireless::airplane"
#define NDEVS 64

int main() {
	int ret = 0;
	int afd = open("/sys/class/leds/" LEDNAME "/trigger", O_WRONLY);
	if (afd == -1) return errno;
	write(afd, "none", 4);
	close(afd);

	int rf_fd = open("/dev/rfkill", O_RDONLY);
	struct rfkill_event ev;
	size_t devcount = 0;
	size_t bcount = 0;
	int states[NDEVS];
	memset(states, 0, NDEVS*sizeof(int));

	while (read(rf_fd, (void*)&ev, sizeof(ev)) == RFKILL_EVENT_SIZE_V1) {
		if (ev.idx >= NDEVS) continue;
		if (ev.op == RFKILL_OP_ADD) {
			++devcount;
			bcount += ev.hard || ev.soft;
		} else {
			if (ev.hard || ev.soft) {
				if (!states[ev.idx]) {
					states[ev.idx] = 1;
					bcount++;
				}
			} else {
				bcount--;
				states[ev.idx] = 0;
			}
		}
		int afd = open("/sys/class/leds/" LEDNAME "/brightness", O_WRONLY);
		if (afd == -1) {
			ret = errno;
			goto end;
		}
		write(afd, (bcount == devcount)?"1":"0", 1);
		close(afd);
	}
end:
	close(rf_fd);
	return ret;
}
