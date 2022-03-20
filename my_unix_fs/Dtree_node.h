//
// Created by 123456 on 2021/10/22.
// Introduction : 用于缓存已访问的目录项的目录树的结点
//

#ifndef MY_UNIX_FS_DTREE_NODE_H
#define MY_UNIX_FS_DTREE_NODE_H

#include "global_header.h"

class Dtree_Node
{
public:
    Dtree_Node() { }

    /* 拷贝构造函数 */
    Dtree_Node(const Dtree_Node &dn)
    {
        strcpy(dir_item->file_name, dn.dir_item->file_name);
        dir_item->inode_no = dn.dir_item->inode_no;
        is_dir = dn.is_dir;

        if(dn.is_dir == true)
        {
            for(auto d : *dn.child)
            {
                Dtree_Node* temp = new Dtree_Node(*d);
                temp->parent = this;
                child->push_back(temp);
            }
        }
    }

    ~Dtree_Node() { }

    Dtree_Node* parent = nullptr;                                                                                /* 指向父目录在目录树中的结点 */
    std::shared_ptr<std::list<Dtree_Node*>> child = std::make_shared<std::list<Dtree_Node*>>();        /* 指向子目录的指针数组，非目录结点不能使用这个指针 */
    std::shared_ptr<struct DIR_ITEM> dir_item = std::make_shared<struct DIR_ITEM>();                   /* 实际目录项 */
    bool is_dir = false;                                                                                       /* 是不是目录项 */
};

#endif //MY_UNIX_FS_DTREE_NODE_H
