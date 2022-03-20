//
// Created by 123456 on 2021/10/17.
// Introduction : 存放全局的类型替换和常量等信息
//

#ifndef MY_UNIX_FS_GLOBAL_HEADER_H
#define MY_UNIX_FS_GLOBAL_HEADER_H

#include <time.h>
#include <fcntl.h>
#include <unistd.h>
#include <memory>
#include <array>
#include <vector>
#include <list>
#include <iostream>
#include <ctime>

#include "locker.h"

/***                                         设计的文件系统中的常量定义区，操作系统不需要知道这些细节                                              ***/
#define FS_TOTAL_SIZE 409772032         /* 固定的文件系统大小，单位为字节(byte)，为400168KB */
#define INODE_LINK_MAX 512              /* 定义了可以指向一个索引结点的目录项的最大数量 */
#define GDT_ITEM_NUM 3                  /* 因为设计的这个文件系统只有三个块组 */
#define INODE_NO_MAX 98304              /* 最大索引结点数量 */
#define BLOCK_NO_MAX 98304              /* 最大存放数据盘块的数量 */
#define BLOCK_SIZE 4096                 /* 此文件系统中的一个物理块的大小 */
#define BLOCKNUM_INODELIST 576          /* 每个块组分配给存储索引结点表的物理块数 */
#define INODE_SIZE 72                   /* 一个inode结点的固定大小 */
#define SIZE_PER_BLOCKGROUP (FS_TOTAL_SIZE - BLOCK_SIZE) / GDT_ITEM_NUM        /* 每个块组的总大小 */
#define BLOCKNUM_PER_BLOCK SIZE_PER_BLOCKGROUP / BLOCK_SIZE                    /* 每个块组共有多少个物理块(不只是只存放数据的块) */
#define FILE_NAME_MAX 30                                                       /* 可定义的最大文件名长度 */
#define DIR_ITEM_SIZE 32                                                       /* 此文件系统的目录项的固定长度 */
#define IDLE_NUM_FSKEEP 100                      /* 文件系统维护的各种结构的空闲表中元素的最大个数 */
//#define MAX_FILE_LEN 8429568                   /* 该文件系统能创建的最大文件长度, 采用一级间接索引，大概8M左右 */
#define MAX_FILE_LEN 40960                       /* 只使用10个直接索引块时文件系统呢个创建的最大文件长度 */
#define MAX_DIRITEM_NUM 512                      /* 一个目录下最多能创建的文件数 */



/***                                         内核常量定义区                                             ***/
#define USER_MAX 30                         /* 最多只能有30个用户同时登录文件系统 */
#define OPEN_MAX 1024                       /* 每个进程可打开的最大文件描述符数量 */
#define IDLE_NUM_KENKEEP 10                 /* 内核维护的各种结构的空闲表中元素的最大个数 */



/***                                         数据类型定义区                                             ***/
typedef short fd_t;                      /* 定义文件描述符类型，因为在此系统中允许的最大文件描述数量很少，所以使用unsigned char */
typedef unsigned char flag_t;            /* 用于表示需要数量较少的标志信息的数据类型 */
typedef unsigned char ftype_t;           /* 表示文件类型的数据类型 */
typedef long offset_t;                   /* 表示文件的偏移量的数据类型，注意文件的偏移量可以为负 */
typedef unsigned char user_t;            /* 用作登录文件系统的用户编号和数量的数据类型 */
typedef unsigned char err_t;             /* 用于表示errno编号的数据类型 */


/***                                         文件类型(ftype_t)定义区                                             ***/
#define NORMAL_FILE 0                   /* 普通文件 */
#define DIR_FILE 1                      /* 目录文件 */
//#define CDEVICE_FILE 2                /* 字符设备文件 */
//#define BDEVICE_FILE 3                /* 块设备文件 */
//#define LINK_FILE 4                   /* 链接文件 */


/***                                         文件系统状态常量定义区                                             ***/
#define FS_OK 0                         /* 文件系统运行正常 */
#define FS_WRONG 1                      /* 文件系统出现了某些问题，可能不能正常运行 */



/***                                         errno常量定义区                                             ***/





/***                                         超级块结构定义区                                             ***/
/* 超级块数据结构，当启动操作系统时超级块被调入内存 */
struct super_block
{
    std::array<unsigned short, IDLE_NUM_FSKEEP> block_wait = {0};     /* 可以及时分配的物理盘块号链表 */
    std::array<unsigned short, IDLE_NUM_FSKEEP> inode_wait = {0};     /* 可以及时分配的索引结点链表 */
    unsigned long fs_size;               /* 文件系统的总大小, 此类数据均为以字节为单位 */
    unsigned short block_size;           /* 此文件系统中物理块的大小 */
    unsigned int idle_block_num;         /* 文件系统中空闲盘块数量 */
    unsigned int idle_inode_num;       /* 文件系统中的空闲inode结点数量 */
    time_t update_time;                  /* 超级块最近一次的更新时间 */
    flag_t fs_status;                    /* 文件系统状态 */
    char fs_typename[20] = { '\0' };     /* 文件系统类型名 */
    bool is_origin;                      /* 是不是超级块本体，如果是，则可以使用超级块副本所在盘块号数组 */
    unsigned int duplication_list[GDT_ITEM_NUM];    /* 超级块所有副本所在文件系统中的偏移 */
    char fs_root[30] = "/";         /* 文件系统的根目录 */
    char fs_dev[20] = { '\0' };          /* 文件系统依赖的底层磁盘设备，挂载时由操作系统分配 */
    bool is_mount = false;               /* 标志是否曾经挂载到内核(操作系统上) */
    //int crc_code;                      /* crc循环冗余校验码，后面添加这个功能 */
};



///* 时间(戳)解析函数 */
//const char* t_prase(const time_t& t)
//{
//   return std::asctime(std::localtime(&t));
//}


/***                                         目录项结构定义区                                            ***/
/* 每一个目录项中只有一个文件名和一个索引结点编号 */
struct DIR_ITEM
{
    char file_name[FILE_NAME_MAX] = { '\0' };
    unsigned short inode_no = 0;
};

/* 由多个目录项组成的数组，长度刚好可以填满一个物理块，当要分配某个空物理块存放目录项时，使用这个结构体格式化那个物理块 */
struct DIR
{
    struct DIR_ITEM dir[BLOCK_SIZE / DIR_ITEM_SIZE];
};


/***                                         stat系统调用返回的file_stat结构体                                            ***/
struct file_stat
{
    unsigned short inode_no;                           /* 索引结点编号 */
    uint16_t mode = 1;                                 /* 文件操作权限 */
    user_t uid = 2;                                    /* 文件属主 */
    unsigned short length;                             /* 文件长度 */
    time_t modified_time;                              /* 文件最后修改时间 */
    bool successed_get = false;                        /* 是否成功获取文件信息, 如果是false，则在获取状态时出现某些问题 */
};


/***                                         lseek操作模式宏定义区                                            ***/
#define SEEK_SET 0                  /* 相对于此打开文件的开头的偏移 */
#define SEEK_CUR 1                  /* 相对于此打开文件当前打开位置的偏移 */


/***                                         文件open打开方式定义区                                            ***/
#define RDWR 0                    /* 以读写方式打开，当前操作偏移为0 */
#define APPEND 1                  /* 以追加方式打开，当前操作偏移为文件长度 */



#endif //MY_UNIX_FS_GLOBAL_HEADER_H
