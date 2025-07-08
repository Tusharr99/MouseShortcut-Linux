#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <linux/netlink.h>
#include <time.h>
#include <errno.h>
#include <sys/wait.h>

#define NETLINK_USER 29
#define MAX_PAYLOAD 64
#define HOLD_DURATION_S 2 // 2 seconds for copy/paste hold
#define MIDDLE_HOLD_DURATION_S 3 // 3 seconds for side buttons screenshot hold

int main(int argc, char *argv[]) {
    if (argc != 3) {
        fprintf(stderr, "Usage: %s <volume_up_button> <volume_down_button>\n", argv[0]);
        return 1;
    }

    int volume_up_button = atoi(argv[1]); // 276
    int volume_down_button = atoi(argv[2]); // 275
    printf("Using button codes: volume_up=%d, volume_down=%d\n", volume_up_button, volume_down_button);

    // Create Netlink socket
    struct sockaddr_nl src_addr, dest_addr;
    struct nlmsghdr *nlh = NULL;
    struct msghdr msg;
    struct iovec iov;
    int sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
    if (sock_fd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    memset(&src_addr, 0, sizeof(src_addr));
    src_addr.nl_family = AF_NETLINK;
    src_addr.nl_pid = getpid();
    src_addr.nl_groups = 0;

    if (bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr)) < 0) {
        perror("Bind failed");
        close(sock_fd);
        return 1;
    }

    memset(&dest_addr, 0, sizeof(dest_addr));
    dest_addr.nl_family = AF_NETLINK;
    dest_addr.nl_pid = 0;
    dest_addr.nl_groups = 0;

    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
    if (!nlh) {
        perror("Memory allocation failed");
        close(sock_fd);
        return 1;
    }
    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
    nlh->nlmsg_pid = getpid();
    nlh->nlmsg_flags = 0;
    strcpy(NLMSG_DATA(nlh), "REGISTER");

    iov.iov_base = (void *)nlh;
    iov.iov_len = nlh->nlmsg_len;
    memset(&msg, 0, sizeof(msg));
    msg.msg_name = (void *)&dest_addr;
    msg.msg_namelen = sizeof(dest_addr);
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;

    printf("Sending Netlink message: len=%d, pid=%d, flags=%d, data=%s\n",
           nlh->nlmsg_len, nlh->nlmsg_pid, nlh->nlmsg_flags, (char *)NLMSG_DATA(nlh));

    if (sendmsg(sock_fd, &msg, 0) < 0) {
        perror("Send registration failed");
        free(nlh);
        close(sock_fd);
        return 1;
    }
    printf("Sent registration message to kernel\n");

    // Button states and timers
    int up_pressed = 0, down_pressed = 0; // 276, 275
    struct timespec both_side_press_time = {0, 0}; // For 275+276 hold
    struct timespec up_press_time = {0, 0}, down_press_time = {0, 0};
    int copy_triggered = 0, paste_triggered = 0, screenshot_triggered = 0;
    int both_side_buttons_pressed = 0; // Track simultaneous 275+276 hold
    static int screenshot_counter = 0;

    while (1) {
        struct timeval tv;
        tv.tv_sec = 0;
        tv.tv_usec = 100000; // 100ms timeout
        setsockopt(sock_fd, SOL_SOCKET, SO_RCVTIMEO, (const char *)&tv, sizeof(tv));

        ssize_t recv_len = recvmsg(sock_fd, &msg, 0);
        struct timespec current_time;
        clock_gettime(CLOCK_MONOTONIC, &current_time);

        if (recv_len < 0) {
            // Check hold durations for copy/paste (only if not both pressed)
            if (up_pressed && !copy_triggered && !both_side_buttons_pressed) {
                double elapsed = (current_time.tv_sec - up_press_time.tv_sec) +
                                 (current_time.tv_nsec - up_press_time.tv_nsec) / 1e9;
                if (elapsed >= HOLD_DURATION_S) {
                    printf("Triggered shortcut: Copy text\n");
                    int status = system("sudo -u tushar xdotool key ctrl+c");
                    if (WIFEXITED(status)) {
                        int exit_code = WEXITSTATUS(status);
                        if (exit_code != 0) {
                            fprintf(stderr, "Copy command failed (exit code %d)\n", exit_code);
                        }
                    } else {
                        fprintf(stderr, "Copy command terminated abnormally\n");
                    }
                    copy_triggered = 1;
                }
            }
            if (down_pressed && !paste_triggered && !both_side_buttons_pressed) {
                double elapsed = (current_time.tv_sec - down_press_time.tv_sec) +
                                 (current_time.tv_nsec - down_press_time.tv_nsec) / 1e9;
                if (elapsed >= HOLD_DURATION_S) {
                    printf("Triggered shortcut: Paste text\n");
                    int status = system("sudo -u tushar xdotool key ctrl+v");
                    if (WIFEXITED(status)) {
                        int exit_code = WEXITSTATUS(status);
                        if (exit_code != 0) {
                            fprintf(stderr, "Paste command failed (exit code %d)\n", exit_code);
                        }
                    } else {
                        fprintf(stderr, "Paste command terminated abnormally\n");
                    }
                    paste_triggered = 1;
                }
            }
            // Check hold duration for screenshot (275+276)
            if (both_side_buttons_pressed && up_pressed && down_pressed && !screenshot_triggered) {
                double elapsed = (current_time.tv_sec - both_side_press_time.tv_sec) +
                                 (current_time.tv_nsec - both_side_press_time.tv_nsec) / 1e9;
                if (elapsed >= MIDDLE_HOLD_DURATION_S) {
                    printf("Triggered shortcut: Screenshot taken\n");
                    char command[256];
                    snprintf(command, sizeof(command),
                             "DISPLAY=:0 scrot /home/tushar/screenshot_$(date +%%Y%%m%%d_%%H%%M%%S_%%3N)_%03d.png",
                             screenshot_counter++);
                    int status = system(command);
                    if (WIFEXITED(status)) {
                        int exit_code = WEXITSTATUS(status);
                        if (exit_code != 0) {
                            fprintf(stderr, "Screenshot command failed (exit code %d)\n", exit_code);
                        }
                    } else {
                        fprintf(stderr, "Screenshot command terminated abnormally\n");
                    }
                    screenshot_triggered = 1;
                    both_side_buttons_pressed = 0;
                    up_pressed = 0;
                    down_pressed = 0;
                }
            }
            fprintf(stderr, "Receive timeout, continuing...\n");
            continue;
        }

        char *message = NLMSG_DATA(nlh);
        printf("Received message: %s\n", message);

        int button_code;
        char state[16];
        if (sscanf(message, "Button %d %s", &button_code, state) != 2) {
            fprintf(stderr, "Invalid message format: %s\n", message);
            continue;
        }

        int is_pressed = (strcmp(state, "pressed") == 0);
        if (!is_pressed) {
            printf("Button %d released\n", button_code);
        }

        // Update button states
        if (button_code == volume_up_button) {
            up_pressed = is_pressed;
            if (is_pressed) {
                clock_gettime(CLOCK_MONOTONIC, &up_press_time);
                copy_triggered = 0;
            }
        } else if (button_code == volume_down_button) {
            down_pressed = is_pressed;
            if (is_pressed) {
                clock_gettime(CLOCK_MONOTONIC, &down_press_time);
                paste_triggered = 0;
            }
        }

        // Volume control
        if (button_code == volume_up_button && is_pressed) {
            printf("Triggered shortcut: Increase volume\n");
            int status = system("sudo -u tushar amixer set Master 5%+ unmute && "
                               "sudo -u tushar amixer set PCM 5%+ unmute && "
                               "sudo -u tushar amixer set 'Master Mono' 5%+ unmute && "
                               "sudo -u tushar amixer set Surround 5%+ unmute && "
                               "sudo -u tushar amixer set Center 5%+ unmute && "
                               "sudo -u tushar amixer set LFE 5%+ unmute");
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if (exit_code != 0) {
                    fprintf(stderr, "Volume up command failed (exit code %d)\n", exit_code);
                }
            } else {
                fprintf(stderr, "Volume up command terminated abnormally\n");
            }
        } else if (button_code == volume_down_button && is_pressed) {
            printf("Triggered shortcut: Decrease volume\n");
            int status = system("sudo -u tushar amixer set Master 5%- unmute && "
                               "sudo -u tushar amixer set PCM 5%- unmute && "
                               "sudo -u tushar amixer set 'Master Mono' 5%- unmute && "
                               "sudo -u tushar amixer set Surround 5%- unmute && "
                               "sudo -u tushar amixer set Center 5%- unmute && "
                               "sudo -u tushar amixer set LFE 5%- unmute");
            if (WIFEXITED(status)) {
                int exit_code = WEXITSTATUS(status);
                if (exit_code != 0) {
                    fprintf(stderr, "Volume down command failed (exit code %d)\n", exit_code);
                }
            } else {
                fprintf(stderr, "Volume down command terminated abnormally\n");
            }
        }

        // Check for simultaneous 275+276 hold
        if (up_pressed && down_pressed && !both_side_buttons_pressed) {
            printf("Both side buttons pressed, starting timer\n");
            both_side_buttons_pressed = 1;
            screenshot_triggered = 0;
            clock_gettime(CLOCK_MONOTONIC, &both_side_press_time);
        } else if (!up_pressed || !down_pressed) {
            both_side_buttons_pressed = 0;
            screenshot_triggered = 0;
        }
    }

    free(nlh);
    close(sock_fd);
    return 0;
}