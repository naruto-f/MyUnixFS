/**
  * Created by 123456 on 2021/10/16.
  * Introduction : 模拟在使用文件系统时内核的动作, 这里主要用于维护于打开文件相关的三个表项: ①文件描述符表 ②文件表 ③v节点表
  *                其中每个进程有自己的文件描述符表和文件表，而对于一个给定的物理文件只有一个v节点表项
  * Tips : 目前版本的内核只能挂载一个文件系统。
  */

#ifndef MY_UNIX_FS_EMULATED_KERNEL_H
#define MY_UNIX_FS_EMULATED_KERNEL_H


#include <vector>
#include <string>
#include <cstring>
#include <set>
#include <sstream>
#include <list>
#include <stack>
#include <map>
#include <queue>
#include <algorithm>

#include "global_header.h"
#include "Emulated_fs.h"
#include "Dtree_node.h"

/* 在内核中的inode结构，比文件中的多了进程引用计数和指向v结点的指针 */
struct KERNEL_INODE : public inode
{
    unsigned char process_link = 0;                  /* 有多少个进程引用了这个打开文件 */
    unsigned short v_ptr = 0;                        /* 在系统v_table数组中的位置，从0开始，也可以使用链表 */
    unsigned char block_used = 0;                    /* 当前已经使用到的盘块号 */
    Locker locker;                                   /* 经过封装的一个简单的互斥锁 */
};

/* 文件的v节点表项，注意对于所有的进程给定的物理文件在内核中只有一个v结点表项，其中的信息是打开文件的时候从磁盘中读出 */
struct V_ITEM
{
    bool is_used = false;             /* 标志V_ITEM数组中的这个结构是否被分配 */
    ftype_t file_type;                /* 文件类型，目前仅支持普通文件(标志为0)和目录文件(标志为1) */

    /*
     *  占位，此处应存放对文件进行各种操作的函数指针
     *
     */

    unsigned short i_ptr;             /* 在系统KERNEL_INODE数组中的位置，从0开始， */
};

/* 文件表项，每一个进程的一个打开文件描述符对应一个文件表项 */
struct file_table
{
    flag_t file_status_flag = -1;                            /* 文件状态标志, 可以为读、写、添写、同步、非阻塞等 */
    offset_t offset = 0;                                         /* 当前进程在此文件上的操作偏移量 */
    unsigned short v_ptr;                                    /* 该文件v节点表项的下标 */
};

/* 文件描述符表中的一个表项 */
struct Fd_list_item
{
    fd_t fd = -1;
    std::shared_ptr<struct file_table> ftable = std::make_shared<struct file_table>();
    char file_buf[BLOCK_SIZE] = { '\0' };          /* 此打开文件的数据缓冲区, 如果文件长度太长可以从磁盘不断提取数据并覆盖缓冲区 */
    fd_t last_fd = -1;                              /* 存放上一个fd的值，在当前只能单目录操作时就是当前工作目录或父目录的fd */
    int cur_block = -1;                             /* 现在缓冲区上操作的是iaddr中的第几个盘块，从0开始 */
};

/* 用户文件表，存放对打开文件的控制信息，每一个用户(我猜想可能类似于一个进程，但不确定)有自己的这个结构 */
struct USER_FDATA
{
    user_t user_id = 0;                                                          /* 已在文件系统注册的用户的id */
    std::string user_name;                                                       /* 已在文件系统注册的用户的用户名 */
    std::array<struct Fd_list_item, OPEN_MAX> fd_list;                           /* 该用户所属的文件描述符表 */
    fd_t cur_fd;                                                                 /* 当前工作的目录或文件的文件描述符号, 或者说是创建此文件描述符时的cur_fd */
    std::string dir_name = "/";                                                  /* 该用户当前或上一次换出前所在的目录名，如果当前用户在使用系统，那么命令提示符上显示这个string */
    char user_buf[MAX_FILE_LEN] = { '\0' };                                      /* 存放用户辅助数据的数据缓冲区 */
    unsigned int buf_offset = 0;                                                 /* 在当前用户缓冲区中的操作偏移 */
};


class Emulated_kernel {
public:
    Emulated_kernel();

    ~Emulated_kernel();

/***                                内核暴露接口定义区                                                       ***/

    /* 模拟挂载行为, 将文件系统挂载到根目录下, 成功返回0，不成功返回-1 */
    int mount(Emulated_vfs* vfs);

    /* 登录界面并校验用户名密码 */
    void login();

    /* 进入命令行用户交互界面 */
    void start_shell();

    /* 获取errno */
    err_t get_errno()
    {
        return err_no;
    }
private:
/***                                内核本来就有的维护的变量或结构                                             ***/

    std::shared_ptr<std::array<struct USER_FDATA, USER_MAX>> user_list;      /* 用于当允许多用户同时登陆系统时存放当前登陆系统的用户表 */
    std::shared_ptr<std::vector<struct V_ITEM>> v_table;                     /* 存放所有打开文件在内核内的唯一一份信息，包括内核中索引结点 */
    std::shared_ptr<std::vector<struct KERNEL_INODE>> kinode_table;          /* 存放每个打开文件在内核中的唯一内核态索引结点结构 */
    std::unordered_map<std::string, std::string> user_pass;                  /* 存放用户名和密码的映射，在内核初始化时由内核读入或创建 */
    std::unordered_map<std::string, unsigned short> order_no;                /* shell支持的命令的名称和其编号的映射关系 */
    std::list<unsigned short> idle_vnode;                                    /* 内核维护的v结点空闲位置表链表 */
    std::list<unsigned short> idle_kinode;                                   /* 内核维护的内核inode结点空闲位置链表 */
    std::vector<std::list<unsigned short>*> idle_vec;                        /* 存放所有空闲链表结构的首地址 */
    user_t cur_user = 0;                                                     /* 用于描述当前操作该文件系统的用户在用户文件表中的编号，从0开始 */
    user_t user_num = 0;                                                     /* 当前同时登陆文件系统的用户数 */
    err_t err_no = 0;                                                        /* 接收上一次模拟的系统调用的错误码，若为0则表示没有错误  */
    Dtree_Node* root = nullptr;                                              /* 内核维护的目录树的根结点 */
    Dtree_Node* cur_dir = nullptr;                                           /* 当前所在工作目录在目录树中的位置 */
    bool exit_flag = false;                                                  /* 用于控制是否退出系统 */
    bool cat_flag = false;                                                   /* 用于控制是否退出cat界面 */
    std::unordered_map<std::string, int> cat_map;    /* cat命令映射*/


/***                                实际文件系统挂载后生成或调入内核的变量和结构                              ***/

    /* 纯虚基类虚拟文件系统的指针，指向实际的文件系统 */
    Emulated_vfs* fs;

    /* 当文件系统挂载时将超级块副本调入内存由内核管理，文件中的超级块和内核控制的超级块同一时间数据可能不一致, 如果后面内核可以允许多个文件系统挂载，可将此结构扩展为可变长数组 */
    std::shared_ptr<struct super_block> sblk;

    /* 维护从超级块中读出的空闲数据块号和索引结点号的链表 */
    std::shared_ptr<std::list<unsigned short>> idle_inode;
    std::shared_ptr<std::list<unsigned short>> idle_block;


/***                                shell命令函数定义区                                                       ***/

    /* cd命令: 切换工作目录，包括open要打开目录文件的类似操作 */
    int change_dir(const std::string& dir_name);

    /* ls命令: 显示目录下的文件，可以带参数-a, -i, -l, 默认是列出当前目录的所有目录项 */
    int list_diritem(const std::string& arg, const std::string& dir_name = "");

    /* mv命令: ①修改文件(夹)名称 ②移动文件或文件夹到另一个目录 */
    int move(const std::string& source, const std::string& dest);

    /* cp命令: 拷贝普通文件或一个目录文件的所有文件到另一个目录下 */
    int copy(const std::string& source, const std::string& dest);

    /* rm命令: 删除一个文件，如欲删除一个目录则要带-r参数 */
    int remove(const std::string& file_name);

    /* rmdir命令: 删除一个空的文件夹，如果非空则需要使用rm命令, 这里删除的空目录文件是没有open的 */
    int remove_dir(const std::string& dir_name);

    /* vi命令: 将文件在文本编辑器 */
    int vi(const std::string& file_name);

    /* touch命令: 创建一个新的普通文件 */
    int touch(const std::string& file_name);

    /* 将普通文本文件的所有内容打印在标准输出设备上, 只能看不能修改
     * 内部命令行操作 : ①q 退出 ②w 向上翻页 ③s 向下翻页
     */
    int cat(const std::string& file_name);

    /* 实现文件系统逻辑格式化 */
    int logic_format();

    /* 实现文件系统底层格式化 */
    int bottom_format();

    /* 在当前目录下创建一个目录文件，注意：在本文件系统中目录文件的大小都是块大小的整数倍, 返回0表示成功，-1表示失败 */
    int mkdir(const std::string dir_name);

    /* exit命令，退出当前用户，跳转到登录页面 */
    void kernal_exit();

    /* 系统调用测试区 */
    void test_system_call();

    /* 模仿模仿unix中的$?命令，这里由errno记下上一次命令行操作或系统调用出错的原因的编号, 0表示没有任何问题 */
    void show_err();

/***                                系统调用函数区                                                       ***/

    /* 打开普通文件，默认以读写方式打开 */
    fd_t open(const std::string& filename, mode_t mode = RDWR);

    /* 关闭打开文件，注意目录文件关闭的方式与普通文件不同 */
    void my_close(const fd_t fd);

    /* 创建新文件 */
    fd_t creat(const std::string& username, mode_t mode = 1);

    /* 改变打开文件的操作偏移 */
    offset_t my_lseek(fd_t fd, offset_t off, mode_t mode);

    /* 读打开文件, 失败返回-1，成功返回读入的字节数 */
    int my_read(fd_t fd, void* buf, const unsigned int size);

    /* 写打开文件, 失败返回-1，成功返回写入文件的字节数 */
    int my_write(fd_t fd, void* buf, const unsigned int size);

/***                                内核工具函数定义区                                                       ***/

    /* 设置errno, 这个操作只能由操作系统完成 */
    void set_errno(flag_t new_errno)
    {
        /* 多进程访问时可能需要加锁，后面有需要时再加加锁操作 */
        err_no = new_errno;
    }

    /* 初始化shell命令名-编号映射表 */
    void init_order_no();

    /* 获取shell支持的命令对应的编号, 如果不存在该命令返回-1 */
    const short get_order_no(const std::string& order_name);

    /*
     * 从空闲v结点空闲或空闲内核inode结点表中获得一个空闲位置, 错误返回-1
     * 当前可选参数只有两个: ①"v_node" ②"ki_node"
     */
    unsigned short get_a_idleloc(const std::string struct_name);

    /* 如果有一个空闲链表用完，更新所有结构空闲链表, 后期再添加此类结构可拓展程序 */
    void update_idle();

    /* 请求文件系统读取磁盘相关信息更新内核中的超级块 */
    void update_sblk(const unsigned char flag = 0);

    /* 为一个创建的文件分配索引结点和物理盘块号
     * 盘块分配规则: 为新创建的文件分配1个盘块共4kb的空间，如果当关闭文件时文件大小超出40kb则使用1级间接索引分配。
     */
    int distribute_newfile_block(struct KERNEL_INODE& i_node);

    /* 当初始分配的物理盘块空间不足，从内核超级块block_wait链表中分配一块 */
    int get_a_idleblock();

    /* 从内核超级块inode_wait链表中分配一个inode结点号 */
    int get_a_idleinode();

    /* 为文件分配需要的盘块号
     * 返回值 : ①0，说明分配成功 ②1，说明当前文件长度超出系统最大长度限制,不予分配内存 ③-1，在分配的过程中磁盘空间不足
     */
    int alloc_file_block(struct KERNEL_INODE& i_node);

    /* 当用户登录成功，初始化用户环境 */
    int init_user_environ(const std::string& username);

    /* 用户名和用户id的映射 */
    user_t name_to_id(const std::string& username);

    /* 用户名和用户id的映射 */
    const std::string uid_to_name(const user_t uid);

    /* 将文件的权限码转换为一个9个字符的字符串 */
    std::string mode_to_string(const uint16_t mode);

    /* 获取一个空闲的文件描述符, 失败返回-1 */
    const fd_t get_idle_fd();

    /* 在目录树的dir目录下查找filename是否存在，若不存在返回-1, 存在返回其索引结点编号 */
    int find_filename(const std::string& filename, Dtree_Node* dir);

    /* 查找指定索引结点编号代表的文件是否已经被当前用户打开 */
    fd_t is_file_open(unsigned short inode_no);

    /* 错误提示函数 */
    void err_cout(const std::string tips);

    /* 创建目录文件和普通文件的公共操作 */
    int init_new_file(const ftype_t& file_type, const std::string& file_name, mode_t mode = 0);

    /* 使用磁盘中索引结点数据初始化内核中索引结点 */
    void init_kinode_byinode(const struct inode& i_node, int i_ptr);

    /* 输出第几页数据在屏幕上 */
    void print_page(unsigned short page_no);

    /* 打印当前正在使用的文件描述符列表信息 */
    void print_fdlist();

    /* 使用深度优先搜索根据索引结点编号在内核目录树中查找对应的Dtree_node */
    Dtree_Node* find_dnode_by_inodeno(unsigned short inode_no);

    /* 递归删除目录文件的所有内容, 注意被删除目录的父目录文件必须打开过(其父目录的所有目录项都已挂入内核目录树) */
    int del_dir(Dtree_Node* dir_node);

    /* dfs深度优先遍历删除文件, flag代表是不要删除的最高层目录项 */
    void dfs_del_file(Dtree_Node* cur_root);

    /* 析构内核目录树, 调用完这个函数后目录树还剩根目录没有析构 */
    void del_Dtree(Dtree_Node* cur_root);

    /* 打印当前内核目录树结构 */
    void print_Dtree();

    /* 设置根目录初始状态: 添加两个.和..目录项 */
    void init_root_dir();

    /* 内核退出时与文件系统对象同步超级块信息 */
    void syn_sblk();

    /* 解析路径名, 并返回在目录树中的位置 */
    Dtree_Node* parse_path(const std::string &path_name);

    /* 如果拷贝一个目录下的所有文件到另一个目录下是，初始化要被拷贝的子目录的内核中目录树 */
    Dtree_Node* init_copy_dir(Dtree_Node* new_tr, Dtree_Node* old_tr);
};


#endif //MY_UNIX_FS_EMULATED_KERNEL_H
