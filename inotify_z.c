/*
 *     filename: inotify_z.c
 *    author:      bodhix
 *    date:      2018-01-31
 *    desc:      test the function of inotify in Linux
 */

#include <sys/inotify.h>
#include <assert.h> // assert
#include <errno.h>  // errno
#include <string.h> // strerror
#include <stdlib.h> // exit

#include "clog.h"

/*
   struct inotify_event {
       int      wd;     // Watch descriptor
       uint32_t mask;   // Mask of events
       uint32_t cookie; // Unique cookie associating related events (for rename(2))
       uint32_t len;    // Size of name field
       char     name[]; // Optional null-terminated name
   };
 */

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_SIZE ((EVENT_SIZE + 16) * 16)
#define EXIT_FLAG "exit_flag"

static int exit_flag = 0;

static int log_inotify_event(struct inotify_event *ev)
{
    if (ev == NULL)
    {
        lerror("log_inotify_event: ev is NULL");
        return -1;
    }

    int mask = ev->mask;
    if (ev->len == 0)
    {
        linfo("log_inotify_event: get 0 len of ev, mask %x", mask);
        return -1;
    }

    if (strncmp(ev->name, EXIT_FLAG, strlen(EXIT_FLAG)) == 0)
    {
        linfo("get exit flag %s, begin to set exit_flag to 1", EXIT_FLAG);
        exit_flag = 1;
    }

    if (mask & IN_CREATE)
    {
        if (mask & IN_ISDIR)
        {
            linfo("Directory %s is created", ev->name);
        }
        else
        {
            linfo("File %s is created", ev->name);
        }
    }
    else if (mask & IN_DELETE)
    {
        if (mask & IN_ISDIR)
        {
            linfo("Directory %s is deleted", ev->name);
        }
        else
        {
            linfo("File %s is deleted", ev->name);
        }
    }
    else
    {
        lerror("Unknown mask %x for %s", mask, ev->name);
    }

}

int main(int argc, char const *argv[])
{
    const char *path = argv[1];
    assert(path);

    int ifd = inotify_init();
    if (ifd == -1)
    {
        lerror("inotify_init: %s", strerror(errno));
        goto ERR;
    }

    int mask = IN_CREATE | IN_DELETE;
    linfo("inotify_add_watch: add path %s with mask %x", path, mask);
    int wd = inotify_add_watch(ifd, path, mask);
    if (wd == -1)
    {
        lerror("inotify_add_watch: %s", strerror(errno));
        goto ERR;
    }

    while (exit_flag == 0)
    {
        int shift = 0;
        char buffer[BUFFER_SIZE];
        int len = read(ifd, buffer, BUFFER_SIZE);
        if(len < 0)
        {
            lerror("read: %s", strerror(errno));
            continue;
        }

        while (shift < len)
        {
            struct inotify_event *ev = (struct inotify_event *)(buffer + shift);
            log_inotify_event(ev);

            shift += EVENT_SIZE + ev->len;
        }
    }

    linfo("inotify_rm_watch: rm watch %d with mask %x", wd, mask);
    int res = inotify_rm_watch(ifd, wd);
    if (res == -1)
    {
        lerror("inotify_rm_watch: %s", strerror(errno));
    }

    linfo("unlink file %s", EXIT_FLAG);
    res = unlink(EXIT_FLAG);
    if (res == -1)
    {
        lerror("unlink: %s", strerror(errno));
    }

    close(ifd);
    goto END;

ERR:
    if (ifd)
    {
        linfo("close ifd %d", ifd);
        close(ifd);
    }
    return -1;

END:
    return 0;
}
