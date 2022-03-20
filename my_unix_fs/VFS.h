//
// Created by 123456 on 2021/10/19.
// Introduction : 模拟内核与实际文件系统的中间层虚拟文件系统，为上层(内核或操作系统管理员或程序员)提供
//                统一的接口，使上层不必关心文件系统底层实现。
//

#ifndef MY_UNIX_FS_VFS_H
#define MY_UNIX_FS_VFS_H


/* 这里的实现方式是普通的纯虚基类，只定义能挂载到操作系统的文件系统必须实现的纯虚接口 */
class Emulated_vfs
{
public:
    Emulated_vfs() { }

    /* 系统调用stat: 根据索引结点编号获取文件信息 */
    virtual const struct inode stat(unsigned short inode_no) = 0;

    /* 重新设置inode结点，这里用于文件关闭时从内核读回inode最新数据 */
    virtual int set_stat(unsigned short inode_no, const char *buf) = 0;

    /* 通过盘块号将数据读入缓冲区 */
    virtual int read_by_blockno(unsigned short blockno, char *buf) = 0;

    /* 接收一个索引结点编号，删除一个物理文件，释放其占用的资源 */
    virtual int rm_file(const unsigned short inode_no) = 0;

    /* 通过盘块号将数据写回磁盘 */
    virtual int write_by_blockno(unsigned short blockno, const char *buf) = 0;

    /* 将指定的索引结点对应的文件调入内存 */
    virtual int read_by_inode(char *buf, struct inode* i_node = nullptr) = 0;

    /* 将目录信息写回磁盘, 成功返回写入的数据数，不成功返回-1 */
    virtual int writeroot_to_dev(struct inode* i_node) = 0;

    /* 将文件系统的超级块读入内核中超级块, 默认是直接传输超级块副本, flag!=0则说明内核超级块的空闲链表用完, 需要更新空闲链表 */
    virtual super_block get_sblk(unsigned char flag = 0) = 0;

    /* 退出系统时与内核中超级块同步信息 */
    virtual void syn_sblk(const struct super_block &k_sblk) = 0;

    /* 实现文件的逻辑格式化，即只将将块位图和inode位图和其他与盘块描述信息有关的结构重置 */
    virtual int Logical_formatting() = 0;

    /* 实现文件的低级格式化，即将所有控制块重置和将所有有数据的物理盘块都清空 */
    virtual int Low_level_formatting() = 0;
};


#endif //MY_UNIX_FS_VFS_H
