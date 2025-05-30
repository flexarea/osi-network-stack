/*
 * cs431vde.c
 */

#include <unistd.h>
#include <stdlib.h>
#include <stdint.h>
#include <stdio.h>
#include <errno.h>
#include <arpa/inet.h>

/* These functions manage the fact that VDE expects the first 2 octets written
 * to be the length of the frame, in octets, in big-endian format.  Therefore,
 * send_ethernet_frame adds those and receive_ethernet_frame removes them. */

ssize_t
receive_ethernet_frame(int fd, void *buf)
{
    uint16_t nbo_len, len;

    read(fd, &nbo_len, 2);
    len = ntohs(nbo_len);
    read(fd, buf, len);

    return len;
}

void
send_ethernet_frame(int fd, void *frame, uint16_t len)
{
    uint16_t nbo_len;

    nbo_len = htons(len);
    write(fd, &nbo_len, 2);
    write(fd, frame, len);
}

/* This function takes the place of dpipe(1), shipped with vde, which is
 * unpleasantly restrictive: it requires the process use stdout and stdin to
 * send and receive packets, respectively, and thus doesn't permit a process
 * to connect to multiple switches (necessary for implementing a router) and
 * prevents the use of printf.
 *
 * Ultimately, we need to fork and exec a process in which to run vde_plug,
 * which is what actually communicates with the vde_switch instance.  Since we
 * want bidirectional communication (send frames to the switch, receive frames
 * from the switch), we need two pipes: to_plug_fds will be used to send
 * frames from this process to the switch (via the vde_plug process);
 * from_plug_fds will be used by this process to receive frames from the
 * switch (again, via the vde_plug process).
 *
 * - to_plug_fds[0]: used by the vde_plug process to receive frames for the
 *   switch
 * - to_plug_fds[1]: used by this process to send frames to the vde_plug
 *   process, and thence to the switch
 * - from_plug_fds[0]: used by this process to receive frames from the
 *   vde_plug process, which it received from the switch
 * - from_plug_fds[1]: used by the vde_plug process to send frames to this
 *   process
 *
 * A return value of 0 indicates success.  A non-zero return value indicates
 * failure, and errno will be set accordingly.
 *
 * Upon successful completion, frames can be read from fds[0] and written to
 * fds[1].
 *
 * The second parameter is the command to execute.  When using a local switch,
 * this will be "vde_plug"; when using a remote switch, this will be an ssh
 * command that in turn calls vde_plug.
 */

int
connect_to_vde_switch(int fds[2], char *cmd[])
{
    int to_plug_fds[2];
    int from_plug_fds[2];
    int err;

	pid_t child_pid;

    /* create pipes */
	if(pipe(to_plug_fds) < 0) {
        err = errno;
        perror("pipe");
        errno = err;
		return -1;
	}
	if(pipe(from_plug_fds) < 0) {
        err = errno;
        close(to_plug_fds[0]);
        close(to_plug_fds[1]);
        perror("pipe");
        errno = err;
		return -1;
	}

    /* fork & exec vde_plug */
    /* Note that we brazenly assume all the close(2) and dup2(2) calls
     * succeed.  Naughty, naughty. */
	child_pid = fork();
	if(child_pid == -1) {
        err = errno;
        close(to_plug_fds[0]);
        close(to_plug_fds[1]);
        close(from_plug_fds[0]);
        close(from_plug_fds[1]);
        errno = err;
        return -1;
    }
    if(child_pid == 0) {
        close(to_plug_fds[1]);
        close(from_plug_fds[0]);

        dup2(to_plug_fds[0], 0);
        dup2(from_plug_fds[1], 1);

        close(to_plug_fds[0]);
        close(from_plug_fds[1]);

        //if(execlp("vde_plug", "vde_plug", NULL) < 0) {
        //if(execlp("ssh", "ssh", "weathertop.cs.middlebury.edu", "/home/pjohnson/cs431/bin/vde_plug", NULL) < 0) {
        if(execvp(cmd[0], cmd) < 0) {
            perror("execlp");
            exit(1);
        }
    }

    /* record the pipe file descriptors that matter to this process */
    fds[0] = from_plug_fds[0];
    fds[1] = to_plug_fds[1];

    /* clean up the others */
    close(from_plug_fds[1]);
    close(to_plug_fds[0]);

    return 0;
}

