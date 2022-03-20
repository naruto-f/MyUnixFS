//
// Created by 123456 on 2021/10/16.
// Introduction : 模拟的文件系统对象
//

#ifndef MY_UNIX_FS_EMULATED_FS_H
#define MY_UNIX_FS_EMULATED_FS_H

#include <iostream>
#include <stdint.h>
#include <unistd.h>
#include <unordered_map>
#include "global_header.h"
#include "VFS.h"
#include <memory.h>


/* 存放在该文件系统引导块中的数据，与实际引导块中的内容不同，位置在文件系统的第一个物理盘块的第一个KB */
struct Boot_block {
    /* 文件系统魔数，存放在文件系统的前8个比特位，用于识别加载的文件是否已被格式化为一个文件系统，只有当其二进制位为10101010时判断该文件已被格式化为文件系统 */
    uint8_t magic_num;
    /*
     *  待添加数据
     */
};

/* 块组描述符表项，即一个块组的描述信息 */
struct GDT_item
{
    unsigned char id;                    /* 此文件系统的第几个块(柱面)组 */
    unsigned short totle_blockno[2];     /* 此块组的所有物理块的块号范围 */
    unsigned short data_blockno[2];      /* 此块组的只用于存储数据的物理块的块号范围 */
    unsigned short idle_inode_num;       /* 此块组中空闲inode结点数量 */
    unsigned short idle_block_num;       /* 此块组中空闲物理盘块数量 */
    unsigned short idle_inode[100];      /* 可用的inode结点号 */
    unsigned short idle_block[100];      /* 可用的block盘快号 */
};

/* 块组描述符表 */
struct GDT_table
{
    struct GDT_item gdt_table[GDT_ITEM_NUM];
};

/* inode结构，此结构体72B */
struct inode
{
    unsigned int iaddr[11] = {0};                    /* 存放盘块号，采用一级间接索引，10个直接地址位，1个间接索引位 */
    unsigned int inode_no = 0;                           /* 索引结点编号 */
    ftype_t type = NORMAL_FILE;                        /* 文件类型 */
    unsigned char ditem_link;                          /* 表示指向该索引结点的目录项数量 */

    /* 文件操作权限，有一个系统默认的权限，后面要设置正确*/
    uint16_t mode = 1;

    user_t uid = 2;                              /* 文件属主 */
    unsigned short length = 0;                   /* 文件长度 */
    //gid_t gid = 0;                             /* 文件属主所在用户组，目前设置所有用户都在0组 */
    time_t modified_time = 0;                    /* 文件最后修改时间 */
    time_t create_time = 0;                      /* 文件创建时间 */
};



class Emulated_fs : public Emulated_vfs{
public:
    /* 默认构造函数，也可以传入其他已创建过此模拟文件系统的文件或空文件(全为字符0) */
    Emulated_fs(const char* fs_file_name = "/mnt/c/Users/123456/Desktop/linux网络编程/my_unix_fs/myfs.fs", const char* name = "my_fs");

    ~Emulated_fs();

    /* 系统调用stat: 根据索引结点编号获取文件信息, 可以返回stat结构体也可以直接返回索引结点的副本 */
    const inode stat(unsigned short inode_no);

    /* 重新设置inode结点，这里用于文件关闭时从内核读回inode最新数据 */
    int set_stat(unsigned short inode_no, const char *buf);

    /* 将指定的索引结点对应的文件调入内存, 如果i_node为默认值，则将根目录的内容调入内存 */
    int read_by_inode(char *buf, struct inode* i_node);

    /* 接收一个索引结点编号，删除其代表的物理文件，释放其占用的资源 */
    int rm_file(const unsigned short inode_no);

    /* 通过盘块号将数据读入内核缓冲区 */
    int read_by_blockno(unsigned short blockno, char *buf);

    /* 通过盘块号将数据写回磁盘 */
    int write_by_blockno(unsigned short blockno, const char *buf);

    /* 将根目录文件信息写回磁盘, 成功返回写入的数据数，不成功返回-1 */
    int writeroot_to_dev(struct inode* i_node);

    /* 向内核返回超级块数据的副本, flag=0说明什么都不用干直接返回超级块副本, flag=1说明空闲块号用完，flag=2说明空闲inode号用完 */
    super_block get_sblk(unsigned char flag = 0);

    /* 退出系统时与内核中超级块同步信息 */
    void syn_sblk(const struct super_block &k_sblk);

    /* 实现文件的逻辑格式化*/
    int Logical_formatting();

    /* 实现文件的低级格式化*/
    int Low_level_formatting();
private:
/***                                   私有成员变量/常量定义区                                                   ***/

    int fd_fs;                /* 打开模拟文件系统文件返回的文件描述符 */
    const char* fs_name;         /* 这个文件系统的名称 */

    /* 初始化以下结构体的智能指针相当于给文件系统中对应的结构体在内存中设置的一个缓存区，可以隔一段时间如果不一致则覆盖物理块上的结构体 */
    std::shared_ptr<struct GDT_table> g_table;                                                                   /* 块组描述符表 */
    std::shared_ptr<struct Boot_block> bblk;                                                                       /* 引导块 */
    std::shared_ptr<struct super_block> sblk;                                                                      /* 超级块 */
    std::shared_ptr<std::array<std::array<uint32_t , BLOCK_SIZE / sizeof(int)>, GDT_ITEM_NUM>> b_map;                    /* 块位图*/
    std::shared_ptr<std::array<std::array<uint32_t , BLOCK_SIZE / sizeof(int)>, GDT_ITEM_NUM>> i_map;                    /* inode结点位图 */
    std::shared_ptr<std::array<std::array<struct inode, INODE_NO_MAX / GDT_ITEM_NUM>, GDT_ITEM_NUM>> inode_list;   /* 索引结点位图 */

    /* 文件系统中的各控制数据块的写入位置, 由于设计的此文件系统的结构和大小固定，所以可以确定每个控制块在文件中的存放位置 */
    std::unordered_map<std::string, unsigned int> control_loc;

    /* 需要在control_loc中初始化位置的标准名称, 能且只能获取这些控制块的位置 */
    std::vector<std::string> ctrblock_name = { "Boot_block", "super_block", "super_block0", "super_block1", "super_block2", "GDT_table", "b_map0",
                                      "b_map1", "b_map2", "i_map0", "i_map1", "i_map2", "i_list0", "i_list1", "i_list2", "datablock_start0", "datablock_start1", "datablock_start2"};

    /* 维护向超级块中传输的空闲数据块号和索引结点号的链表 */
    std::list<unsigned short> idle_inode;
    std::list<unsigned short> idle_block;


/***                                   私有成员函数定义区                                                   ***/

    /* 初始化固定的controlblock_loc，用空间换时间，后面使用的时间复杂度都为O(1) */
    void init_ctrblock_loc();

    /*
     * 如果文件已被格式化为此文件系统格式，则将控制结构写入内存
     * 返回值 : ①0，说明在读取文件中的控制块结构时出现了某些问题。
     *         ②1，说明读取一切正常。
     */
    int init_old_fs();

    /* 为一个没创建过文件系统的新文件格式化为一个虚拟文件系统 */
    int new_fs();

    /* 初始化或将关闭文件系统时减少代码量的抽象出来的改变文件偏移和读的操作对 */
    int init_lseek_read(const std::string block_name, void* ptr, size_t size, const std::string arg);

    /* 为外界使用控制块位置提供的公共接口, 成功返回控制块在文件中的偏移，不存在或不允许访问返回-1 */
    const int get_ctrblock_loc(const std::string block_name);

    /* 重置调入内存中的超级块和快表描述符为初始状态 */
    void reset_sblk_and_gdt();

    /* 将所有在内存中的控制块都写回磁盘 */
    int write_to_dev();

    /* 数据盘块号到在文件中的偏移的转换 */
    const long blockno_to_offset(unsigned short bno);

    /* 更新超级块中的空闲盘块号或inode结点号, 如果系统内没有更多空闲号则返回-1和错误信息 */
    int update_idle(unsigned char flag);

    /* 置位块位表和inode位表中的某个位为1或0 */
    void set_idle_bit(const std::string struct_name, unsigned short loc, unsigned char set_value);

    /* 补充超级块中的空闲链表, 没有任何空闲位置时返回-1 */
    int supplement_idle_list(const std::string struct_name, unsigned short num);

    /* 更新所有备份超级块，失败返回-1 */
    int update_backup_sblk();

    /* 将根目录的内容读到buf上 */
    int read_rootdir(char *buf);

    /* 根据inode结点将文件内容读到buf上 */
    int read_file(char *buf, struct inode *i_node);
};


#endif //MY_UNIX_FS_EMULATED_FS_H
