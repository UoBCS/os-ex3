#define SUCCESS 0
#define DEVICE_NAME "opsysmem"

static int Major;
static size_t max_msg_len = 4 * 1024 * sizeof(char);
static size_t max_msg_ls_len = 2 * (1024 * 1024) * sizeof(char);

struct node {
	char *msg;
	struct node *next;
};

struct linked_list {
	struct node *head;
	size_t total_sz;
};

static struct linked_list msg_ls;
static char *msg_ptr;

// Linked list definition
// ----------------------
int ls_append(struct linked_list*, char*, size_t);
int ls_remove(struct linked_list*, char**);
int ls_destroy(struct linked_list*);

// Device driver definition
// ------------------------
int init_module(void);
void cleanup_module(void);
static int device_open(struct inode *, struct file *);
static int device_release(struct inode *, struct file *);
static ssize_t device_read(struct file *, char *, size_t, loff_t *);
static ssize_t device_write(struct file *, const char *, size_t, loff_t *);
static long device_ioctl(struct file *file, unsigned int ioctl_num, unsigned long);

static struct file_operations fops = {
	.read = device_read,
	.write = device_write,
	.open = device_open,
	.release = device_release,
	.unlocked_ioctl = device_ioctl
};