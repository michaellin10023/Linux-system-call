#include<linux/kernel.h>
#include<linux/init.h>
#include<linux/sched.h>
#include<linux/syscalls.h>
#include<linux/string.h>
#include<linux/list.h>
#include<linux/kprobes.h>
#include<linux/kallsyms.h>
#include<linux/module.h>
#include<linux/types.h>
#include<linux/errno.h>
#include<linux/slab.h>
#include<asm/uaccess.h>

#define SYMBOL_LEN 20

static LIST_HEAD(global_list);
static struct dp_dev *dp_ptr;
static int dp_id = 0;
struct dumpmode_t {
	unsigned int mode;
};

struct dp_dev{
  struct kprobe kp;
  int dp_id;
  char symbol_name[20];
  pid_t pid;
  pid_t tgid;
  int mode;
  struct list_head next;
};

// void insert(struct dp_dev *ptr){

//   list_add(&ptr->next, &global_list);
// }

int remove(int dumpid){

  struct list_head *list_ptr = NULL;
  struct task_struct *task = current;
  struct dp_dev *dummy_ptr = NULL;
  pid_t pid;
  pid = task->pid;

  list_for_each(list_ptr, &global_list){
    dummy_ptr = container_of(list_ptr,struct dp_dev,next);
    if(dummy_ptr->dp_id == dumpid && dummy_ptr->pid == pid){
      printk("found dump stack! removing\n");
      unregister_kprobe(&dummy_ptr->kp);
      list_del(list_ptr);
      kfree(dummy_ptr);
      break;
    }
    else{
      printk("dump stack not found\n");
      return EINVAL;
    }
  }
  return 0;
}

static int handler_pre(struct kprobe *probe, struct pt_regs *regs){

  struct dp_dev *mydev;
  struct task_struct *task = current;
  pid_t tgid, pid;
  tgid = task->tgid;
  pid = task->pid;

  mydev = container_of(probe, struct dp_dev,kp);
//   printk("prehandler mydev pid: %ld\n", (long)pid);
//   printk("prehandler mydev tgid: %ld\n", (long)tgid);

  // enable for any processes
  if(mydev->mode > 1){
	printk("line %d\n",__LINE__);
    dump_stack();
  }
  // enable if the owner's dump mode is 0
  else if(mydev->mode == 0 && mydev->pid == pid){
	printk("line %d\n",__LINE__);
    dump_stack();
  }
  // enable if process is same as owner and dump mode is 1 
  else if(mydev->mode == 1 && (mydev->pid == pid || mydev->tgid == tgid)){
	printk("line %d\n",__LINE__);
    dump_stack();
  }
  return 0;
}

SYSCALL_DEFINE2(insdump,const char __user *, symbolname, struct dumpmode_t __user *, dumpmode){

  unsigned int addr;
  char *symbol_name;
  struct dumpmode_t data;
  int ret;
  struct task_struct *task = current;
  pid_t pid, tgid;
//   printk(KERN_INFO "Inside insdump syscall:\n");

#ifdef CONFIG_DYNAMIC_DUMP_STACK

  
  pid = task->pid;
  tgid = task->tgid;
  dp_ptr = kmalloc(sizeof(struct dp_dev),GFP_KERNEL);
  symbol_name = kmalloc(sizeof(char)*SYMBOL_LEN, GFP_KERNEL);
  if(!symbol_name){
    printk("can't allocate memory for symbol name\n");
  }
  strncpy_from_user((char *)symbol_name,symbolname, SYMBOL_LEN);
  addr =  kallsyms_lookup_name(symbol_name);
  if(addr == 0){
    printk("can't find symbol\n");
    return EINVAL;
  }
  ret = copy_from_user(&data,dumpmode,sizeof(data));
  if(ret < 0){
    printk("can't get data from user\n");
  }
  printk("dump stack mode: %d\n", data.mode);
  printk("pid: %ld\n", (long)pid);
  printk("tgid: %ld\n", (long)tgid);
  dp_ptr->pid = pid;
  dp_ptr->tgid = tgid;
  dp_ptr->mode = data.mode;
  sprintf(dp_ptr->symbol_name,symbol_name);
  memset(&dp_ptr->kp,0,sizeof(struct kprobe));
  dp_ptr->kp.pre_handler = (kprobe_pre_handler_t)handler_pre;
  dp_ptr->kp.addr = (kprobe_opcode_t *)addr;
  dp_ptr->dp_id = dp_id++;
  ret = register_kprobe(&dp_ptr->kp);
  if(ret < 0){
    printk("register kprobe failed at 0x%x\n", addr);
    return ret;
  }else{
	printk("kprobe planted at 0x%x\n",addr);
  }
  list_add(&dp_ptr->next,&global_list);
  printk("kprobe inserted\n");
//   insert(dp_ptr);
  return dp_ptr->dp_id;

#else

  return 0;

#endif

}

SYSCALL_DEFINE1(rmdump,unsigned int, dumpid){

  int ret;
//   printk(KERN_INFO "Inside rmdump syscall:\n");

#ifdef CONFIG_DYNAMIC_DUMP_STACK

  ret = remove(dumpid);
  if(ret != 0) return EINVAL;
  else return 1;

#else

  return 0;

#endif

}

