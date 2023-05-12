#include "types.h"
#include "riscv.h"
#include "defs.h"
#include "param.h"
#include "stat.h"
#include "spinlock.h"
#include "proc.h"
#include "fs.h"
#include "sleeplock.h"
#include "file.h"
#include "fcntl.h"

uint64 sys_symlink(void)
{
    char name[DIRSIZ], path[MAXPATH], linkpath[MAXPATH];
    struct inode *ip, *dp;

    int linkpath_length = argstr(1, linkpath, MAXPATH);

    if(argstr(0, path, MAXPATH) < 0 || linkpath_length < 0) {
        return -1;
    }

    begin_op();

    if ((dp = nameiparent(path, name)) == 0) {
        end_op();
        return -1;
    }

    if ((ip = ialloc(dp->dev, T_SYMLINK)) == 0) {
        iput(dp);
        end_op();
        return -1;
    }

    ilock(ip);
    ip->nlink++;
    iupdate(ip);
    dp->nlink++;
    iupdate(dp);
    iput(dp);

    int result = writei(ip, 0, (uint64)linkpath, 0, linkpath_length);

    iput(ip);
    end_op();
    return result < linkpath_length;
}

uint64 sys_readlink(void)
{
    char path[MAXPATH];

    uint64 buf;
    argaddr(1, &buf);

    struct inode *ip;

    begin_op();

    if (argstr(0, path, MAXPATH) < 0) {
        return -1;
    }

    if ((ip = namei(path)) == 0) {
        end_op();
        return -1;
    }

    if (ip->type != T_SYMLINK) {
        iput(ip);
        end_op();
        return -1;
    }

    ilock(ip);

    int result = readi(ip, 1, buf, 0, ip->size);

    iput(ip);

    end_op();
    return result <= 0;
}
