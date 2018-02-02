/*
 *    filename:  inotify_z.c
 *    author:    bodhix
 *    date:      2018-01-31
 *    desc:      test the function of inotify in Linux
 */

#include <sys/inotify.h>
#include <assert.h> // assert
#include <errno.h>  // errno
#include <string.h> // strerror
#include <stdlib.h> // exit
#include <getopt.h>

#include "clog.h"

#define EVENT_SIZE (sizeof(struct inotify_event))
#define BUFFER_SIZE ((EVENT_SIZE + 16) * 16)
#define EXIT_FLAG "exit_flag"

static int exit_flag = 0;

/*
   struct inotify_event {
       int      wd;     // Watch descriptor
       uint32_t mask;   // Mask of events
       uint32_t cookie; // Unique cookie associating related events (for rename(2))
       uint32_t len;    // Size of name field
       char     name[]; // Optional null-terminated name
   };
 */

/*
   IN_ACCESS         File was accessed (read) (*).
   IN_ATTRIB         Metadata changed, e.g., permissions, timestamps, extended attributes, link count (since Linux 2.6.25), UID, GID, etc. (*).
   IN_CLOSE_WRITE    File opened for writing was closed (*).
   IN_CLOSE_NOWRITE  File not opened for writing was closed (*).
   IN_CREATE         File/directory created in watched directory (*).
   IN_DELETE         File/directory deleted from watched directory (*).
   IN_DELETE_SELF    Watched file/directory was itself deleted.
   IN_MODIFY         File was modified (*).
   IN_MOVE_SELF      Watched file/directory was itself moved.
   IN_MOVED_FROM     Generated for the directory containing the old filename when a file is renamed (*).
   IN_MOVED_TO       Generated for the directory containing the new filename when a file is renamed (*).
   IN_OPEN           File was opened (*).
   IN_ALL_EVENTS     macro is defined as a bit mask of all of the above events
   IN_MOVE           which equates to IN_MOVED_FROM|IN_MOVED_TO
   IN_CLOSE          which equates to IN_CLOSE_WRITE|IN_CLOSE_NOWRITE
   IN_DONT_FOLLOW    Don't dereference pathname if it is a symbolic link. (since Linux 2.6.15)
   IN_EXCL_UNLINK    By default, when watching events on the children of a directory, events are generated for children even after they have been  unlinked  from  the  directory.
                     This  can  result  in  large numbers of uninteresting events for some applications (e.g., if watching /tmp, in which many applications create temporary files
                     whose names are immediately unlinked).  Specifying IN_EXCL_UNLINK changes the default behavior, so that events are not generated for children after they have
                     been unlinked from the watched directory. (since Linux 2.6.36)
   IN_MASK_ADD       Add (OR) events to watch mask for this pathname if it already exists (instead of replacing mask).
   IN_ONESHOT        Monitor pathname for one event, then remove from watch list.
   IN_ONLYDIR        Only watch pathname if it is a directory. (since Linux 2.6.15)
 */

struct inotify_mask_s
{
    int access;
    int close_write;
    int close_nowrite;
    int create;
    int delete;
    int delete_self;
    int modify;
    int move_self;
    int moved_from;
    int moved_to;
    int open;
    int all_events;
    int move;  // IN_MOVED_FROM | IN_MOVED_TO
    int close; // IN_CLOSE_WRITE | IN_CLOSE_NOWRITE
    int dont_follow;
    int excl_unlink;
    int mask_add;
    int oneshot;
    int onlydir;
} mask_s;

struct option long_options[] =
{
    {"path", 1, NULL, 'p'},
    {"create", 0, NULL, 'c'},
    {"delete", 0, NULL, 'd'},
    {"modify", 0, NULL, 'm'},
    {NULL, 0, NULL, 0}
};

struct inotify_opt {
    int mask;
    char *path;
};

static const char *optstring = "p:cdm";

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
    else if (mask & IN_MODIFY)
    {
        if (mask & IN_ISDIR)
        {
            linfo("Directory %s is modified", ev->name);
        }
        else
        {
            linfo("File %s is modified", ev->name);
        }
    }
    else
    {
        lerror("Unknown mask %x for %s", mask, ev->name);
    }
}

static void init_options(struct inotify_opt *opt)
{
    assert(opt);

    opt->mask = 0;
    opt->path = NULL;
}

static void parse_options(int argc, char *argv[], struct inotify_opt *opt)
{
    assert(argc);
    assert(argv);
    assert(opt);

    int c = 0;
    while (1)
    {
        c = getopt_long(argc, argv, optstring, long_options, NULL);

        if (c == -1)
        {
            break;
        }

        switch (c)
        {
            case 'p':
                opt->path = optarg;
                break;
            case 'c':
                opt->mask = opt->mask | IN_CREATE;
                break;
            case 'd':
                opt->mask = opt->mask | IN_DELETE;
                break;
            case 'm':
                opt->mask = opt->mask | IN_MODIFY;
                break;
            default:
                lerror("Unknown option %c", c);
                break;
        }
    }

    linfo("parse_options finish");
}

static void check_options(struct inotify_opt *opt)
{
    assert(opt);

    if (opt->path == NULL)
    {
        lerror("You should specify a path to monitored");
        exit(-1);
    }
    else if (opt->mask == 0)
    {
        lerror("You should specify inotify mask");
        exit(-1);
    }

    linfo("check_options: path = %s; mask = %x", opt->path, opt->mask);
}

int main(int argc, char *argv[])
{
    struct inotify_opt *opt;
    init_options(opt);
    parse_options(argc, argv, opt);
    check_options(opt);

    int ifd = inotify_init();
    if (ifd == -1)
    {
        lerror("inotify_init: %s", strerror(errno));
        goto ERR;
    }

    linfo("inotify_add_watch: add path = %s with mask = %x", opt->path, opt->mask);
    int wd = inotify_add_watch(ifd, opt->path, opt->mask);
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

    linfo("inotify_rm_watch: rm watch %d with mask = %x", wd, opt->mask);
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
