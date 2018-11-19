#include <linux/module.h>
#include <linux/kernel.h>
#include <linux/sched.h>
#include <linux/pid.h>
#include <linux/kallsyms.h>
#include <linux/fs.h>
#include <linux/miscdevice.h>
#include <linux/uaccess.h>
#include <linux/mmu_context.h>
#include <linux/workqueue.h>
#include <linux/mm_types.h>
#include <linux/sched/mm.h>
#include <linux/sched/task.h>

#include "suprotect.h"

int (*do_mprotect_pkey)(unsigned long start, size_t len,
                        unsigned long prot, int pkey) = NULL;

struct suprotect_work {
    struct work_struct work;

    struct mm_struct *mm;
    unsigned long start;
    size_t len;
    unsigned long prot;
    int ret_value;
};

void suprotect_work(struct work_struct *work)
{
    struct suprotect_work *suprotect_work = container_of(work,
                                                         struct suprotect_work,
                                                         work);

    use_mm(suprotect_work->mm);
    suprotect_work->ret_value = do_mprotect_pkey(suprotect_work->start,
                                                 suprotect_work->len,
                                                 suprotect_work->prot, -1);
    unuse_mm(suprotect_work->mm);
}

static long suprotect_ioctl(struct file *file, unsigned int cmd,
                            unsigned long arg)
{
    long ret;
    struct suprotect_request params;
    struct pid *pid_struct = NULL;
    struct task_struct *task = NULL;
    struct mm_struct *mm = NULL;
    struct suprotect_work work;

    if (cmd != SUPROTECT_IOCTL_MPROTECT)
        return -ENOTTY;

    if (copy_from_user(&params, (struct suprotect_request *) arg,
                       sizeof(params)))
        return -EFAULT;

    /* Find the task by the pid */
    pid_struct = find_get_pid(params.pid);
    if (!pid_struct)
        return -ESRCH;

    task = get_pid_task(pid_struct, PIDTYPE_PID);
    if (!task) {
        ret = -ESRCH;
        goto out;
    }

    /* Get the mm of the task */
    mm = get_task_mm(task);
    if (!mm) {
        ret = -ESRCH;
        goto out;
    }

    /* Initialize work */
    INIT_WORK(&work.work, suprotect_work);

    work.mm = mm;
    work.start = (unsigned long) params.addr;
    work.len = params.len;
    work.prot = params.prot;

    /* Queue the work */
    (void) schedule_work(&work.work);

    /* Wait for completion of the work */
    flush_work(&work.work);
    ret = work.ret_value;

out:
    if (mm) mmput(mm);
    if (task) put_task_struct(task);
    if (pid_struct) put_pid(pid_struct);

    return ret;
}

static const struct file_operations suprotect_fops = {
    .owner = THIS_MODULE,
    .unlocked_ioctl = suprotect_ioctl,
};

static struct miscdevice suprotect_dev = {
    .minor = MISC_DYNAMIC_MINOR,
    .name = SUPROTECT_DEV_NAME,
    .fops = &suprotect_fops,
};

static int suprotect_init(void)
{
    int err;

    do_mprotect_pkey = (void *) kallsyms_lookup_name("do_mprotect_pkey");
    if (!do_mprotect_pkey) {
        printk(KERN_ERR "Could not find do_mprotect_pkey.\n");
        return -ENOSYS;
    }

    err = misc_register(&suprotect_dev);
    if (err < 0) {
        printk(KERN_ERR "Could not register misc device, error: %d.\n", err);
        return err;
    }

    return 0;
}

static void suprotect_cleanup(void)
{
    misc_deregister(&suprotect_dev);
}

MODULE_LICENSE("GPL");
module_init(suprotect_init);
module_exit(suprotect_cleanup);

