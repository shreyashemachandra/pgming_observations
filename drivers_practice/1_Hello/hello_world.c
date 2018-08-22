#include<linux/module.h>
#include<linux/kernel.h>

static int __init driver_init(void){
	printk(KERN_ALERT "Hello Driver\n");
	return 0;
}

static void __exit driver_exit(void){
	printk(KERN_ALERT "Bye Bye Driver\n");
}

module_init(driver_init);
module_exit(driver_exit);

MODULE_LICENSE("GPL");
MODULE_AUTHOR("Shreyas");
