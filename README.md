# The Task
Write a device driver for a character device which implements a simple
way of message passing. The kernel maintains a list of
messages. To limit memory usage, we impose a limit of 4KB for each
message, and also impose a limit of the size of all messages, which is
initially 2MB. 

Your device driver should perform the following operations:

* Creating the device, which has to be _/dev/opsysmem_,   creates an empty list of messages. 
* Removing the device deallocates all messages and removes the list   of messages. 
* Reading from the device returns one message, and removes this message from the kernel list. If the list of messages is empty, the reader returns _-EAGAIN_.
* Writing to the device stores the message in kernel space and adds it to the list if the message is below the maximum size, and the limit of the size of all messages wouldn't be surpassed with this message. If the message is too big, _-EINVAL_ is returned, and if the limit of the size of all messages was surpassed, _-EAGAIN_ is returned.
* You should also provide an ioctl which sets a new maximum size of all messages. This operation should succeed only if the new maximum is bigger than the old maximum, or if the new maximum is bigger than the size of all messages currently held. The ioctl should return 0 on success and _-EINVAL_ on failure.

You need to ensure that your code deals with multiple attempts at
reading and writing at the same time. Your code should minimise the
time spent in critical sections.
The reader should obtain the message in the same order as they were written.

## Testing

Do not modify or add files in the repository that start with _test_, since we use those files for scripts for testing.