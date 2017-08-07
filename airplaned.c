
#include <linux/rfkill.h>
#include <linux/ioctl.h>
#include <linux/input.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <unistd.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <stropts.h>


// TODO fix this
#define LEDNAME "asus-wireless::airplane"

// TODO implement index compression
#define NDEVS 64

#define MAX(A,B) ((A)>(B)?(A):(B))

int main(int argc, char *argv[]) {
    int ret = 0;
    if (argc < 2) return 1;

    // TODO maybe parse /proc/bus/input/devices?
    int evfd = open(argv[1], O_RDONLY);
    struct input_event evi;

    int rffd = open("/dev/rfkill", O_RDWR);
    ioctl(rffd, RFKILL_IOCTL_NOINPUT);
    struct rfkill_event evr;

    int afd = open("/sys/class/leds/" LEDNAME "/trigger", O_WRONLY);
    if (afd == -1) return errno;
    // TODO maybe save previous state and restore on exit?
    write(afd, "none", 4);
    close(afd);

    fd_set fdset;
    FD_ZERO(&fdset);
    FD_SET(evfd, &fdset);
    FD_SET(rffd, &fdset);

    int states[NDEVS];
    int savedstates[NDEVS];
    enum rfkill_type types[NDEVS];
    for (size_t i = 0; i < NDEVS; ++i) states[i] = -1;

    int airplane = 0;

    while (1) {
        // TODO maybe switch to poll?
        int sr = select(MAX(evfd, rffd)+1, &fdset, NULL, NULL, NULL);
        if (sr == -1) {
            ret = errno;
            break;
        } else if (sr == 0) continue;
        if (FD_ISSET(rffd, &fdset)) {
            // TODO ignore if generated by me
            if (read(rffd, &evr, sizeof(evr)) != RFKILL_EVENT_SIZE_V1) {
                ret = 2;
                break;
            }
            if (evr.idx >= NDEVS) continue;
            if (evr.op == RFKILL_OP_ADD) {
                // TODO maybe try to deal with device renumbering?
                savedstates[evr.idx] = states[evr.idx] = evr.hard || evr.soft;
                types[evr.idx] = evr.type;
            } else if (evr.op == RFKILL_OP_DEL) {
                states[evr.idx] = -1;
            } else {
                states[evr.idx] = (evr.hard || evr.soft);
            }
        } else FD_SET(rffd, &fdset);
        if (FD_ISSET(evfd, &fdset)) {
            if (read(evfd, &evi, sizeof(evi)) != sizeof(evi)) {
                ret = 3;
                break;
            }
            if (!(evi.value == 1 && evi.code == KEY_RFKILL)) continue;
            if (airplane) {
                airplane = 0;
                for (size_t i = 0; i < NDEVS; ++i) {
                    states[i] = states[i] && (savedstates[i]==1); //in case it was unused before
                    // TODO move this into a function
                    evr.hard = 0;
                    evr.soft = states[i];
                    evr.idx = i;
                    evr.op = RFKILL_OP_CHANGE;
                    evr.type = types[i];
                    write(rffd, &evr, sizeof(evr));
                }
            } else {
                airplane = 1;
                for (size_t i = 0; i < NDEVS; ++i) {
                    savedstates[i] = states[i];
                    // TODO move this into a function
                    states[i] = 1;
                    evr.hard = 0;
                    evr.soft = 1;
                    evr.idx = i;
                    evr.op = RFKILL_OP_CHANGE;
                    evr.type = types[i];
                    write(rffd, &evr, sizeof(evr));
                }
            }
        } else FD_SET(evfd, &fdset);

        int afd = open("/sys/class/leds/" LEDNAME "/brightness", O_WRONLY);
        if (afd == -1) {
            ret = errno;
            break;
        }
        write(afd, airplane?"1":"0", 1);
        close(afd);
    }

    close(rffd);
    close(evfd);
    return ret;
}