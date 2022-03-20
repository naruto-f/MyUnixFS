//
// Created by 123456 on 2021/10/16.
//

#include "Emulated_kernel.h"


Emulated_kernel::Emulated_kernel()
{
    user_list = std::make_shared<std::array<struct USER_FDATA, USER_MAX>>();
    v_table = std::make_shared<std::vector<struct V_ITEM>>(100);
    kinode_table = std::make_shared<std::vector<struct KERNEL_INODE>>(100);
    root = new Dtree_Node;
    cat_map = {{"quit", 0}, {"up", 1}, {"down", 2}};

    /* 后面如果从文件或数据库读取，则扩展为函数init_user_password() */
    user_pass = { {"root", "123456"}, {"naruto", "123456"} };
    //init_user_password()

    init_order_no();

    for(int i = 0; i < 10; ++i)
    {
        idle_vnode.push_back(i);
        idle_kinode.push_back(i);
    }

    idle_vec.push_back(&idle_kinode);
    idle_vec.push_back(&idle_vnode);


}

void Emulated_kernel::login() {
    while(1)
    {
        system("clear");
        std::cout << " _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ _ " << std::endl;
        std::cout << "|****************************************************************|" << std::endl;
        std::cout << "|*********************** Welcome To My FS ***********************|" << std::endl;
        std::cout << "|****************************************************************|" << std::endl;
        std::cout << " ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ ¯ " << std::endl;
        std::cout << std::endl;

        std::string user = "";
        std::string password = "";
        std::cout << "login:";
        std::cin >> user;
        if(user_pass.count(user) == 1)
        {
            std::cout << "Password:";
            std::cin >> password;
            if(user_pass[user] == password)
            {
                init_user_environ(user);
                break;
            }
            else
            {
                std::cout << "您输入的密码有误，请重新输入！" << std::endl;
            }
        }
        else
        {
            std::cout << "您输入的用户名有误，请重新输入！" << std::endl;
        }
        sleep(2);
    }
    system("clear");
}

void Emulated_kernel::start_shell() {
    std::string word = "", order_name = "", arg = "";
    std::vector<std::string> order_arg;
    std::cin.ignore(1);

    while(!exit_flag)
    {
        order_arg.clear();
        order_name = "";
        std::string line_in;

        std::cout << (*user_list)[cur_user].user_name << ":" << (*user_list)[cur_user].dir_name
                  << (((*user_list)[cur_user].user_name == "root") ? "#" : "$") << " ";

        getline(std::cin, line_in);
        std::stringstream line(line_in);
        line >> order_name;
        while(line >> arg)
        {
            order_arg.push_back(arg);
        }

        if(strcmp(order_name.c_str(), "") == 0)
        {
            continue;
        }

        switch (get_order_no(order_name))
        {
            case 0:
                /* mkdir */
                if(order_arg.size() > 1)
                {
                    std::cout << "There can be no spaces between the names!" << std::endl;
                    break;
                }
                mkdir(order_arg[0]);
                break;
            case 1:
                /* ls */
                if(order_arg.size() > 2)
                {
                    err_cout("Too many parameters!");
                    break;
                }

                if(order_arg[0] == "-l")
                {
                    if(order_arg.size() == 1)
                    {
                        list_diritem(order_arg[0]);
                    }
                    else
                    {
                        list_diritem(order_arg[0], order_arg[1]);
                    }
                }
                else
                {
                    err_cout("Parameter error!");
                }
                break;
            case 2:
                /* cd */
                change_dir(order_arg[0]);
                break;
            case 3:
                /* rm */
                remove(order_arg[0]);
                break;
            case 4:
                /* rmdir */
                remove_dir(order_arg[0]);
                break;
            case 5:
                /* touch */
                touch(order_arg[0]);
                break;
            case 6:
                /* cat */
                if(std::strcmp(order_arg[0].c_str(), "") == 0 || cat(order_arg[0]) == -1)
                {
                    err_cout("The file does not exist!");
                }
                else
                {
                    std::cin.ignore(1);
                }
                break;
            case 7:
                /* 格式化: 逻辑格式化参数为-l，底层格式化参数为-b */
                if(order_arg.size() > 1)
                {
                    std::cout << "Only one parameter can be entered!" << std::endl;
                    break;
                }

                if(order_arg[0] == "-l")
                {
                    logic_format();
                }
                else if(order_arg[0] == "-b")
                {
                    bottom_format();
                }
                else
                {
                    err_cout("Invalid parameter ");
                }
                break;
            case 8:
                /* test */
                test_system_call();
                break;
            case 9:
                /* 打印当前内核目录树 */
                print_Dtree();
                break;
            case 10:
                /* 打印当前内核目录树 */
                kernal_exit();
                break;
            case 11:
                /* 显示上一次命令行操作或系统调用出错的原因的编号, 0表示没有任何问题 */
                show_err();
                break;
            case 12:
                /* mv */
                if(order_arg.size() == 2)
                {
                    move(order_arg[0], order_arg[1]);
                }
                else
                {
                    err_cout("Parameter error!");
                }
                break;
            case 13:
                /* cp */
                if(order_arg.size() == 2)
                {
                    copy(order_arg[0], order_arg[1]);
                }
                else
                {
                    err_cout("Parameter error!");
                }
                break;
            default:
                std::cout << "Command " << order_name << " not found!";
                std::cout << std::endl;
                break;
        }
    }
}

void Emulated_kernel::init_order_no() {
    order_no = {{"mkdir", 0}, {"ls", 1}, {"cd", 2}, {"rm", 3}, {"rmdir", 4}, {"touch", 5},
                {"cat", 6}, {"format", 7}, {"test", 8}, {"show_tree", 9}, {"exit", 10}, {"$?", 11},
                {"mv", 12}, {"cp", 13}};
}

const short Emulated_kernel::get_order_no(const std::string& order_name) {
    if(order_no.find(order_name) != order_no.end())
    {
        return order_no[order_name];
    }
    return -1;
}

int Emulated_kernel::mount(Emulated_vfs *vfs) {
    /* 使用父类静态类型接收子类指针或引用实现动态绑定 */
    fs = vfs;

    /* 初始化维护空闲inode和block的数据结构 */
    idle_inode = std::make_shared<std::list<unsigned short>>();
    idle_block = std::make_shared<std::list<unsigned short>>();

    /* 模拟从磁盘读入超级块 */
    struct super_block sb = fs->get_sblk();
    sblk = std::make_shared<struct super_block>(sb);

    /* 将初始超级块中的空闲块号结点号读入链表 */
    for(int i = 0; i < IDLE_NUM_FSKEEP; ++i)
    {
        if(i != 0 && sblk->block_wait[i] == 0)
        {
            break;
        }
        idle_block->push_back(sblk->block_wait[i]);
    }

    for(int i = 0; i < IDLE_NUM_FSKEEP; ++i)
    {
        if(i != 0 && sblk->inode_wait[i] == 0)
        {
            break;
        }
        idle_inode->push_back(sblk->inode_wait[i]);
    }

    /* 不管是第一次挂载还是已经挂载，为根目录分配的内核inode结点号和v_table表项号一定都是0 */
    unsigned short root_loc = get_a_idleloc("v_node");
    unsigned short root_i_loc = get_a_idleloc("ki_node");

    (*v_table)[root_loc].file_type = DIR_FILE;
    (*v_table)[root_loc].is_used = true;
    (*v_table)[root_loc].i_ptr = root_i_loc;

    /* 如果文件系统第一次挂载到操作系统 */
    if(sblk->is_mount == false)
    {
        /* 创建挂载点"/"目录, 先在内核中建立KERNEL_INODE，V_ITEM，到真正有文件系统挂载时才写入磁盘 */
        (*kinode_table)[root_i_loc].type = DIR_FILE;
        (*kinode_table)[root_i_loc].inode_no = 0;
        (*kinode_table)[root_i_loc].ditem_link = 2;        /* 初始时根节点中的.和..目录项都指向他自己的索引结点 */
        (*kinode_table)[root_i_loc].mode = 1;              /* 先暂时都置为1，需要修改*/
        (*kinode_table)[root_i_loc].uid = 0;
        (*kinode_table)[root_i_loc].create_time = time(nullptr);
        (*kinode_table)[root_i_loc].modified_time = (*kinode_table)[root_i_loc].create_time;
        (*kinode_table)[root_i_loc].process_link = 0;
        (*kinode_table)[root_i_loc].v_ptr = root_loc;
        (*kinode_table)[root_i_loc].block_used = 0;
        /* 因为存储到物理文件时要为"/"根目录创建.和..两个目录项，所以长度暂时设为一个盘块的大小, 避免再浪费时间去同步 */
        (*kinode_table)[root_i_loc].length = BLOCK_SIZE;

        /* 将文件系统挂载到当前仅有的”/“目录，为”/“目录分配实际的索引结点和空间, 并将”/“目录的索引结点等信息写回磁盘 */
        distribute_newfile_block((*kinode_table)[0]);

        /* 在目录树上为根节点创建.和..两个子节点 */
        Dtree_Node* dn1 = new Dtree_Node();
        Dtree_Node* dn2 = new Dtree_Node();

        dn1->is_dir = true;
        dn1->dir_item->inode_no = (*kinode_table)[root_i_loc].inode_no;
        std::strcpy(dn1->dir_item->file_name, ".");
        dn1->parent = root;
        root->child->push_back(dn1);

        dn2->is_dir = true;
        dn2->dir_item->inode_no = (*kinode_table)[root_i_loc].inode_no;
        std::strcpy(dn2->dir_item->file_name, "..");
        dn2->parent = root;
        root->child->push_back(dn2);

        struct DIR d;
        memset(&d, '\0', sizeof(struct DIR));
        d.dir[0] = *(dn1->dir_item);
        d.dir[1] = *(dn2->dir_item);

        /* 也可以等关闭文件时再将目录项写回磁盘 */
        struct inode i_node = static_cast<struct inode>((*kinode_table)[0]);
        if(fs->write_by_blockno((*kinode_table)[0].iaddr[0], (char*)&d) == -1 || fs->set_stat((*kinode_table)[0].inode_no, (char*)&i_node) == -1)
        {
            std::cout << "根目录创建失败！" << std::endl;
            exit(1);
        }

        sblk->is_mount = true;
    }
    else
    {
        /* 为根目录分配初始内核索引结点和v_table表项 */
        struct inode root_inode = fs->stat(0);
        init_kinode_byinode(root_inode, root_i_loc);
        (*kinode_table)[root_i_loc].process_link = 0;
        (*kinode_table)[root_i_loc].v_ptr = root_loc;
        (*kinode_table)[root_i_loc].block_used = 0;

        /* 将第0索引结点号对应的文件即"/"根目录的文件盘块调入内核 */
        char buf[MAX_DIRITEM_NUM * DIR_ITEM_SIZE] = { '\0' };
        Dtree_Node* dn = nullptr;
        fs->read_by_inode(buf);
        std::array<struct DIR_ITEM, MAX_DIRITEM_NUM> v = *(std::array<struct DIR_ITEM, MAX_DIRITEM_NUM>*)buf;
        for(int i = 0; i < v.size(); ++i)
        {
            dn = new Dtree_Node;

            if(i == 0)
            {
                dn->is_dir = true;
                dn->dir_item->inode_no = 0;
                std::strcpy(dn->dir_item->file_name, ".");
                dn->parent = root;
                root->child->push_back(dn);

                continue;
            }

            if(i == 1)
            {
                dn->is_dir = true;
                dn->dir_item->inode_no = 0;
                std::strcpy(dn->dir_item->file_name, "..");
                dn->parent = root;
                root->child->push_back(dn);

                continue;
            }

            if(v[i].inode_no == 0)
            {
                break;
            }

            dn->parent = root;
            *(dn->dir_item) = v[i];
            struct inode in = fs->stat(dn->dir_item->inode_no);
            dn->is_dir = in.type == DIR_FILE ? true : false;
            (*root->child).push_back(dn);
        }
    }

    /* 只有根目录结点的父节点是空 */
    root->parent = nullptr;
    root->is_dir = true;
    (root->dir_item)->inode_no = 0;
    std::strcpy((root->dir_item)->file_name, "/");
    cur_dir = root;

    return 0;
}

unsigned short Emulated_kernel::get_a_idleloc(const std::string name) {
    /* 每次取空闲块号时先检查是否需要更新 */
    update_idle();

    int ret = -1;
    if(name == "v_node")
    {
        ret = idle_vnode.front();
        idle_vnode.pop_front();
    }
    else if(name == "ki_node")
    {
        ret = idle_kinode.front();
        idle_kinode.pop_front();
    }

    return ret;
}

void Emulated_kernel::update_idle() {
    bool flag = true;
    for(auto idle : idle_vec)
    {
        if(idle->size() == 0)
        {
            flag = false;
            break;
        }
    }

    /* 只要有一个结构空闲块为空，更新所有结构，可以有更优更可拓展的实现方法，但我暂时还没想到 */
    if(flag == false)
    {
        /* 更新v结点空闲表 */
        int i = 0, need_size = IDLE_NUM_KENKEEP - idle_vnode.size();
        for(i = 0; i < v_table->size(); ++i)
        {
            if(need_size == 0)
            {
                break;
            }

            if((*v_table)[i].is_used == false)
            {
                idle_vnode.push_back(i);
                --need_size;
            }
        }
        while(need_size)
        {
            idle_vnode.push_back(i++);
            --need_size;
        }

        /* 更新内核inode结点空闲表 */
        need_size = IDLE_NUM_KENKEEP - idle_kinode.size();
        for(i = 0; i < kinode_table->size(); ++i)
        {
            if(need_size == 0)
            {
                break;
            }

            if((*kinode_table)[i].create_time == 0)
            {
                idle_kinode.push_back(i);
                --need_size;
            }
        }
        while(need_size)
        {
            idle_kinode.push_back(i++);
            --need_size;
        }

        /* 如果后期添加更多相同类型的空闲链表，可在此处添加更新过程
         *
         *
         *
         */
    };
}

int Emulated_kernel::mkdir(const std::string dir_name) {
    fd_t dir_fd = init_new_file(DIR_FILE, dir_name);
    if(dir_fd == -1)
    {
        err_cout("Failed to create new dir!");
        return -1;
    }

    my_close(dir_fd);
    return 0;
}

int Emulated_kernel::logic_format() {
    /* 关闭除了fd=0(根目录所使用的文件描述符)外的所有文件描述符 */
    for(int i = 1; i < OPEN_MAX; ++i)
    {
        if((*user_list)[cur_user].fd_list[i].fd != -1)
        {
            my_close(i);
        }
    }

    /* 只删除除root外内核目录树中的所有目录项，不清除具体的物理文件，并将.和..两个目录项添加到root子目录中 */
    del_Dtree(root);
    init_root_dir();

    /* 调用底层文件系统格式化接口，并更新内核中超级块 */
    fs->Logical_formatting();

    /* 将内核内的空闲链表清空并请求更新超级块并更新两个空闲链表 */
    idle_inode->clear();
    idle_block->clear();
    update_sblk(1);
    update_sblk(2);

    /* 将根目录设为当前目录, 并重置根目录索引结点信息 */
    cur_dir = root;
    (*user_list)[cur_user].dir_name = root->dir_item->file_name;
    (*user_list)[cur_user].cur_fd = 0;
    (*user_list)[cur_user].fd_list[0].cur_block = 0;
    bzero((*user_list)[cur_user].fd_list[0].file_buf, '\0');
    (*kinode_table)[0].iaddr[11] = { 0 };
    (*kinode_table)[0].ditem_link = 2;        /* 初始时根节点中的.和..目录项都指向他自己的索引结点 */
    (*kinode_table)[0].block_used = 0;
    (*kinode_table)[0].length = BLOCK_SIZE;
    (*kinode_table)[0].modified_time = time(nullptr);

    struct inode in = static_cast<struct inode>((*kinode_table)[0]);
    fs->set_stat(0, (char*)&in);

    std::cout << "logic format successed!" << std::endl;
    return 0;
}

int Emulated_kernel::bottom_format() {
    /* 将根目录下的除了.和..目录项对应文件外的文件和子目录及其在内核目录树中的结点和存储盘块都删除(清理)，当存储的数据相对较小的时候这种效率比较高 */

    for(int i = 1; i < OPEN_MAX; ++i)
    {
        if((*user_list)[cur_user].fd_list[i].fd != -1)
        {
            my_close(i);
        }
    }

    cur_dir = root;
    int size = root->child->size() - 2;
    while(size)
    {
        Dtree_Node *temp = root->child->back();
        if(remove(temp->dir_item->file_name) == -1)
        {
            std::cout << "bottom format failed!" << std::endl;
            exit(1);
        }
        --size;
    }

    fs->Low_level_formatting();

    /* 将内核内的空闲链表清空并请求更新超级块并更新两个空闲链表 */
    idle_inode->clear();
    idle_block->clear();
    update_sblk(1);
    update_sblk(2);

    /* 将根目录设为当前目录, 并重置根目录索引结点信息 */
    cur_dir = root;
    (*user_list)[cur_user].dir_name = root->dir_item->file_name;
    (*user_list)[cur_user].cur_fd = 0;
    (*user_list)[cur_user].fd_list[0].cur_block = 0;
    bzero((*user_list)[cur_user].fd_list[0].file_buf, '\0');
    (*kinode_table)[0].iaddr[11] = { 0 };
    (*kinode_table)[0].ditem_link = 2;        /* 初始时根节点中的.和..目录项都指向他自己的索引结点 */
    (*kinode_table)[0].block_used = 0;
    (*kinode_table)[0].length = BLOCK_SIZE;
    (*kinode_table)[0].modified_time = time(nullptr);

    struct inode in = static_cast<struct inode>((*kinode_table)[0]);
    fs->set_stat(0, (char*)&in);

    std::cout << "bottom format successed!" << std::endl;
    return 0;
}

void Emulated_kernel::update_sblk(const unsigned char flag) {
    struct super_block sb = fs->get_sblk(flag);
    *sblk = sb;
    sblk->update_time = time(nullptr);

    if(flag == 1)
    {
        for(int i = 0; i < IDLE_NUM_FSKEEP; ++i)
        {
            if(i != 0 && sblk->block_wait[i] == 0)
            {
                break;
            }
            idle_block->push_back(sblk->block_wait[i]);
        }
    }
    else if(flag == 2)
    {
        for(int i = 0; i < IDLE_NUM_FSKEEP; ++i)
        {
            if(i != 0 && sblk->inode_wait[i] == 0)
            {
                break;
            }
            idle_inode->push_back(sblk->inode_wait[i]);
        }
    }
}

int Emulated_kernel::distribute_newfile_block(struct KERNEL_INODE& i_node) {
    /* 文件系统已经没有足够的inode结点或空间去创建一个新文件 */
    if(sblk->idle_inode_num == 0 || sblk->idle_block_num < 1000)
    {
        return -1;
    }

    int inode_no = -1;
    if((inode_no = get_a_idleinode()) != -1)
    {
        i_node.inode_no = inode_no;
    }
    else
    {

        return -1;
    }

    /* 为新创建的文件分配一个物理盘块号 */
    i_node.iaddr[0] = get_a_idleblock();

    return 0;
}

int Emulated_kernel::init_user_environ(const std::string& username) {
    (*user_list)[cur_user].user_name = username;
    (*user_list)[cur_user].dir_name = "/";
    (*user_list)[cur_user].cur_fd = 0;
    (*user_list)[cur_user].user_id = name_to_id(username);
    (*user_list)[cur_user].fd_list[0].fd = 0;
    (*user_list)[cur_user].fd_list[0].ftable = std::make_shared<struct file_table>();
    (*user_list)[cur_user].fd_list[0].ftable->file_status_flag = 1;
    (*user_list)[cur_user].fd_list[0].ftable->offset = 0;
    (*user_list)[cur_user].fd_list[0].ftable->v_ptr = 0;
    ++(*kinode_table)[0].process_link;
    ++user_num;
    return 0;
}

int Emulated_kernel::get_a_idleblock() {
    /* 文件系统已经没有足够空间去创建一个新文件，虽然每次只分配一个物理盘块号，但还是要预留一些空间以备使用 */
    if(sblk->idle_block_num < 1000)
    {
        return -1;
    }

    /* 目前仅使用10个直接索引块, 间接索引块功能后续再拓展 */
    if(idle_block->size() == 0)
    {
        update_sblk(1);
    }

    int ret = idle_block->front();
    idle_block->pop_front();
    sblk->idle_block_num -= 1;

    /* 将物理盘块内容清空，防止读到脏数据 */
    char nu[BLOCK_SIZE] = { '\0' };
    fs->write_by_blockno(ret, nu);

    return ret;
}

int Emulated_kernel::alloc_file_block(KERNEL_INODE &i_node) {
    /* 如果当前文件的长度大于已经分配的盘块所能容纳的最大空间，再分配空间 */
    if(i_node.length > MAX_FILE_LEN)
    {
        return -1;
    }

    if(i_node.length > (i_node.block_used + 1) * BLOCK_SIZE)
    {
        int needsize = i_node.length / BLOCK_SIZE - i_node.block_used;
        int blockno = -1;
        for(int i = 0; i < needsize; ++i)
        {
            if((blockno = get_a_idleblock()) == -1)
            {
                /* 磁盘空间不足返回-1 */
                return -1;
            }
            i_node.iaddr[++i_node.block_used] = blockno;
        }
    }
    return 0;
}

int Emulated_kernel:: list_diritem(const std::string &arg, const std::string &dir_name) {
    Dtree_Node* dest_file = nullptr;

    /* 如果dir_name不是空string, 那么解析路径名并更新内核目录树并返回路径对应的目录或普通文件在内核目录树中的位置 */
    if(dir_name != "")
    {
        dest_file = parse_path(dir_name);
        if (dest_file == nullptr) {
            /* 可以模仿unix中的$?命令，这里由errno记下上一次命令行操作或系统调用出错的原因的编号 */
            set_errno(1);
            return 1;
        }
    }
    else
    {
        dest_file = cur_dir;
    }

    std::cout.width(10);
    std::cout << std::left << "inode_no" << "\t";
    std::cout.width(10);
    std::cout << std::left << "fmode" << "\t";
    std::cout.width(10);
    std::cout << std::left << "uname" << "\t";
    std::cout.width(10);
    std::cout << std::left << "length" << "\t";
    std::cout.width(30);
    std::cout << std::left << "file name" << "\t";
    std::cout.width(26);
    std::cout << std::left << "modified time" << "\t" << std::endl;

    /* 如果目标文件是普通文件 */
    if(dest_file->is_dir == false)
    {
        struct inode cur_f = fs->stat(dest_file->dir_item->inode_no);

        std::cout.width(10);
        std::cout << std::left << dest_file->dir_item->inode_no << "\t" ;
        std::cout.width(1);
        std::cout << std::left << (dest_file->is_dir ? "d" : "-");
        std::cout.width(9);
        std::cout << std::left << mode_to_string(cur_f.mode) << "\t";
        std::cout.width(10);
        std::cout << std::left << uid_to_name(cur_f.uid) << "\t";
        std::cout.width(10);
        std::cout << std::left << cur_f.length << "\t";
        std::cout.width(30);
        std::cout << std::left << std::string(dest_file->dir_item->file_name) << "\t";
        std::cout.width(26);
        std::cout << std::left << std::asctime(std::localtime(&cur_f.modified_time)) << "\t" << std::endl;
    }
    /* 要ls的文件为目录文件 */
    else
    {
        for(auto iter = dest_file->child->cbegin(); iter != dest_file->child->cend(); ++iter)
        {
            struct inode cur_f = fs->stat((*iter)->dir_item->inode_no);

            std::cout.width(10);
            std::cout << std::left << (*iter)->dir_item->inode_no << "\t" ;
            std::cout.width(1);
            std::cout << std::left << ((*iter)->is_dir ? "d" : "-");
            std::cout.width(9);
            std::cout << std::left << mode_to_string(cur_f.mode) << "\t";
            std::cout.width(10);
            std::cout << std::left << uid_to_name(cur_f.uid) << "\t";
            std::cout.width(10);
            std::cout << std::left << cur_f.length << "\t";
            std::cout.width(30);
            std::cout << std::left << std::string((*iter)->dir_item->file_name) << "\t";
            std::cout.width(26);
            std::cout << std::left << std::asctime(std::localtime(&cur_f.modified_time)) << "\t" << std::endl;
        }
    }

    return 0;
}

std::string Emulated_kernel::mode_to_string(const uint16_t mode) {
    return "rwxrwxrwx";
}

int Emulated_kernel::change_dir(const std::string &dir_name) {
    auto dest_dir = parse_path(dir_name);
    if(dest_dir == nullptr)
    {
        return -1;
    }

    if(dest_dir->is_dir == false)
    {
        std::cout << "cd: " << dest_dir->dir_item->file_name  << ": Not a directory" << std::endl;
        return -1;
    }

    /* 处理cd目录项.和..两种特殊情况 */
    if(dir_name == ".")
    {
        /* .就是本目录，所以什么也不干 */
    }
    else if(dir_name == "..") {
        if (strcmp(cur_dir->dir_item->file_name, "/") == 0) {
            /* 如果是根目录，就什么也不做 */
        } else {
            /* 切换命令行显示的目录名 */
            int size = strlen(cur_dir->dir_item->file_name);
            size = (strcmp(cur_dir->parent->dir_item->file_name, "/") == 0) ? size : size + 1;
            for (int i = 0; i < size; ++i) {
                (*user_list)[cur_user].dir_name.pop_back();
            }

            cur_dir = cur_dir->parent;
        }
    }
    else
    {
        cur_dir = dest_dir;
        std::list<std::string> path_file;
        std::string path = dir_name;
        for(int i = 0; i < path.size(); ++i)
        {
            if(path[i] == '/')
            {
                path[i] = ' ';
            }
        }

        std::stringstream ss(path);
        std::string temp = "";
        while(ss >> temp)
        {
            path_file.push_back(temp);
        }

        if(dir_name[0] == '/')
        {
            (*user_list)[cur_user].dir_name = "/";
        }

        while(!path_file.empty())
        {
            auto name = path_file.front();
            path_file.pop_front();
            if(strcmp(name.c_str(), ".") == 0)
            {
                continue;
            }
            else if(strcmp(name.c_str(), "..") == 0)
            {
                if ((*user_list)[cur_user].dir_name == "/") {
                    /* 如果是根目录，就什么也不做 */
                    continue;
                }

                while((*user_list)[cur_user].dir_name.back() != '/')
                {
                    (*user_list)[cur_user].dir_name.pop_back();
                }
                if((*user_list)[cur_user].dir_name.size() != 1)
                {
                    (*user_list)[cur_user].dir_name.pop_back();
                }
            }
            else
            {
                if((*user_list)[cur_user].dir_name == "/")
                {
                    (*user_list)[cur_user].dir_name += name;
                }
                else
                {
                    (*user_list)[cur_user].dir_name += "/";
                    (*user_list)[cur_user].dir_name += name;
                }
            }
        }
    }

    return 0;
}

const fd_t Emulated_kernel::get_idle_fd() {
    int i = 0;
    for(i = 0; i < OPEN_MAX; ++i)
    {
        if((*user_list)[cur_user].fd_list[i].fd == -1)
        {
            return i;
        }
    }
    return -1;
}

const std::string Emulated_kernel::uid_to_name(user_t uid) {
    return "naruto";
}

user_t Emulated_kernel::name_to_id(const std::string &username) {
    return 1;
}

offset_t Emulated_kernel::my_lseek(fd_t fd, offset_t off, mode_t mode) {
    if((*user_list)[cur_user].fd_list[fd].fd == -1)
    {
        err_cout("Cannot lseek unopened file!");
        return -1;
    }

    unsigned short vptr = (*user_list)[cur_user].fd_list[fd].ftable->v_ptr;
    unsigned short iptr = (*v_table)[vptr].i_ptr;

    if(mode == SEEK_SET)
    {
        if(off >= MAX_FILE_LEN)
        {
            err_cout("offset exceeds the maximum file length!");
            return -1;
        }

        (*user_list)[cur_user].fd_list[fd].ftable->offset = off;
    }
    else if(mode == SEEK_CUR)
    {
        if((*user_list)[cur_user].fd_list[fd].ftable->offset + off >= MAX_FILE_LEN)
        {
            err_cout("offset exceeds the maximum file length!");
            return -1;
        }

        (*user_list)[cur_user].fd_list[fd].ftable->offset += off;
    }

    /* 如果当前偏移超过文件长度，则为文件分配物理盘块并增加文件长度(此时这个文件就是空洞文件) */
    if(((*user_list)[cur_user].fd_list[fd].ftable->offset) > ((*kinode_table)[vptr].length - 1))
    {
        if(((*user_list)[cur_user].fd_list[fd].ftable->offset) >= (((*kinode_table)[vptr].block_used + 1) * BLOCK_SIZE))
        {
            int need_block = (((*user_list)[cur_user].fd_list[fd].ftable->offset) - ((*kinode_table)[vptr].block_used + 1) * BLOCK_SIZE) / BLOCK_SIZE + 1;
            for(int i = 0; i < need_block; ++i)
            {
                int block_no = 0;
                if((block_no = get_a_idleblock()) == -1)
                {
                    err_cout("Failed to allocate physical disk block!");
                    return -1;
                }
                (*kinode_table)[vptr].iaddr[++(*kinode_table)[vptr].block_used] = block_no;
            }
        }
        (*kinode_table)[vptr].length = (*user_list)[cur_user].fd_list[fd].ftable->offset;
        (*kinode_table)[vptr].modified_time = time(nullptr);

        /* 如果文件长度更改，将文件信息(即inode结点)写回磁盘 */
        struct inode new_inode = static_cast<struct inode>((*kinode_table)[vptr]);
        fs->set_stat((*kinode_table)[vptr].inode_no, (char*)&new_inode);
    }

    return (*user_list)[cur_user].fd_list[fd].ftable->offset;
}

fd_t Emulated_kernel::open(const std::string &filename, mode_t mode) {
    auto cur_loc = parse_path(filename);
    if(cur_loc == nullptr)
    {
        err_cout("The file does not exist!");
        return -1;
    }

    /* 查找这个文件是否已经被当前用户打开 */
    if(is_file_open(cur_loc->dir_item->inode_no) != -1)
    {
        err_cout("This file has been opened!");
        return is_file_open(cur_loc->dir_item->inode_no);
    }

    fd_t fd = get_idle_fd();
    if(fd == -1)
    {
        err_cout("The number of open files has reached the limit!");
        return -1;
    }

    /* 先分配必要的内核中数据结构 */
    bool is_dir = cur_loc->is_dir;

    unsigned short v_ptr = get_a_idleloc("v_node");
    unsigned short i_ptr = get_a_idleloc("ki_node");

    struct inode i_node = fs->stat(cur_loc->dir_item->inode_no);
    /* 简单判断一下，如果读出的索引节点的创建时间为0，则读取有误，不能打开文件 */
    if(i_node.create_time == 0)
    {
        err_cout("Index node read error!");
        return -1;
    }

    init_kinode_byinode(i_node, i_ptr);
    (*kinode_table)[i_ptr].v_ptr = v_ptr;
    ++(*kinode_table)[i_ptr].process_link;

    /* 计算物理文件已使用的盘块号 */
    int i = 0;
    for(i = 0; i < 10; ++i)
    {
        if((*kinode_table)[i_ptr].iaddr[i] == 0)
        {
            break;
        }
    }
    (*kinode_table)[i_ptr].block_used = i - 1;


    (*v_table)[v_ptr].is_used = true;
    (*v_table)[v_ptr].file_type = is_dir ? DIR_FILE : NORMAL_FILE;
    (*v_table)[v_ptr].i_ptr = i_ptr;

    (*user_list)[cur_user].fd_list[fd].fd = fd;
    (*user_list)[cur_user].fd_list[fd].cur_block = 0;
    (*user_list)[cur_user].fd_list[fd].ftable = std::make_shared<struct file_table>();
    (*user_list)[cur_user].fd_list[fd].ftable->file_status_flag = 1;
    (*user_list)[cur_user].fd_list[fd].ftable->offset = (mode == RDWR) ? 0 : (*kinode_table)[i_ptr].length;
    (*user_list)[cur_user].fd_list[fd].ftable->v_ptr = v_ptr;

    /* 如果是普通文件，先将该文件的第一个磁盘块中的内容读入其文件描述符对应的缓存区, 等待其他操作 */
    if(!is_dir)
    {
        /* 普通文件的目录项已经在其父目录打开时被挂入内核目录树，所以不需要再有关于目录树的其他操作 */
        int res = 0;
        memset((*user_list)[cur_user].fd_list[fd].file_buf, '\0', BLOCK_SIZE);
        res = fs->read_by_blockno((*kinode_table)[i_ptr].iaddr[0], (*user_list)[cur_user].fd_list[fd].file_buf);
        if(res == -1)
        {
            err_cout("Failed to read data of block0!");
        }
        (*user_list)[cur_user].fd_list[fd].cur_block = 0;
    }

    (*user_list)[cur_user].fd_list[fd].last_fd = (*user_list)[cur_user].cur_fd;
    return fd;
}

int Emulated_kernel::find_filename(const std::string& filename, Dtree_Node* dir) {
    for(auto dnode = dir->child->cbegin(); dnode != dir->child->cend(); ++dnode)
    {
        if(strcmp((*dnode)->dir_item->file_name, filename.c_str()) == 0)
        {
            return 0;
        }
    }

    return -1;
}

fd_t Emulated_kernel::is_file_open(unsigned short inode_no) {
    int i = 0, i_ptr = 0;
    for(i = 0; i < OPEN_MAX; ++i)
    {
        if((*user_list)[cur_user].fd_list[i].fd != -1)
        {
            i_ptr = (*v_table)[(*user_list)[cur_user].fd_list[i].ftable->v_ptr].i_ptr;
            if((*kinode_table)[i_ptr].inode_no == inode_no)
            {
                return (*user_list)[cur_user].fd_list[i].fd;
            }
        }
    }
    return -1;
}

void Emulated_kernel::err_cout(const std::string tips) {
    std::cout << "error: " << tips << std::endl;
}

/* 创建并打开文件，如果文件已存在，则打开文件，将文件内容清空，长度清0 */
fd_t Emulated_kernel::creat(const std::string &path_name, mode_t mode) {
    fd_t file_fd = init_new_file(NORMAL_FILE, path_name, mode);
    if(file_fd == -1)
    {
        err_cout("Failed to create new file");
        return -1;
    }
    return file_fd;
}

int Emulated_kernel::init_new_file(const ftype_t& file_type, const std::string& file_name, mode_t mode) {
    /* 文件名长度不能超过限制 */
    if(file_name.size() > FILE_NAME_MAX)
    {
        err_cout("sorry, the directory name is too long to create failed!");
        return -1;
    }

    /* 解析出父目录的名称和其在目录树中的位置 */
    std::string parent_name = file_name;
    std::string real_filename = "";
    while(parent_name.back() != '/' && !parent_name.empty())
    {
        real_filename.push_back(parent_name.back());
        parent_name.pop_back();
    }
    std::reverse(real_filename.begin(), real_filename.end());

    Dtree_Node* parent_dnode = nullptr;
    if(parent_name.empty())
    {
        parent_dnode = cur_dir;
    }
    else
    {
        parent_dnode = parse_path(parent_name);
        if(parent_dnode == nullptr)
        {
            return -1;
        }
    }

    /* 同目录下不能有同名文件 */
    if(find_filename(real_filename, parent_dnode) != -1)
    {
        err_cout("There can be no files with the same name in the same directory!");
        return -1;
    }

    /* 如果没有可用的文件描述符，不能创建 */
    fd_t fd = get_idle_fd();
    if(fd == -1)
    {
        err_cout("The number of open files has reached the limit!");
        return -1;
    }

    unsigned short v_ptr = get_a_idleloc("v_node");
    unsigned short i_ptr = get_a_idleloc("ki_node");

    if(distribute_newfile_block((*kinode_table)[i_ptr]) == -1)
    {
        std::cout << "sorry, there is not enough space in the file system to create a new file !" << std::endl;
        return -1;
    }

    (*v_table)[v_ptr].is_used = true;
    (*v_table)[v_ptr].file_type = file_type;
    (*v_table)[v_ptr].i_ptr = i_ptr;

    /* 父目录的一个子目录项和其自身的.目录项指向它自身 */
    (*kinode_table)[i_ptr].ditem_link = (file_type == DIR_FILE) ? 2 : 1;
    (*kinode_table)[i_ptr].type = file_type;
    (*kinode_table)[i_ptr].uid = (*user_list)[cur_user].user_id;
    (*kinode_table)[i_ptr].mode = (mode == 0) ? 1 : mode;
    (*kinode_table)[i_ptr].length = ((file_type == DIR_FILE) ? BLOCK_SIZE : 0);
    (*kinode_table)[i_ptr].modified_time = (*kinode_table)[i_ptr].create_time = time(nullptr);
    (*kinode_table)[i_ptr].v_ptr = v_ptr;
    ++(*kinode_table)[i_ptr].process_link;
    (*kinode_table)[i_ptr].block_used = 0;

    /* 将父目录增加一个目录项并将父目录的ditem_link加1 */
    struct inode cur_f = fs->stat(parent_dnode->dir_item->inode_no);
    ++cur_f.ditem_link;
    cur_f.modified_time = time(nullptr);
    fs->set_stat(parent_dnode->dir_item->inode_no, (char*)&cur_f);

    /* 更新内核目录树 */
    Dtree_Node* dn = new Dtree_Node();
    dn->is_dir = ((file_type == DIR_FILE) ? true : false);
    dn->parent = parent_dnode;
    (*dn->dir_item).inode_no = (*kinode_table)[i_ptr].inode_no;
    std::strcpy((*dn->dir_item).file_name, real_filename.c_str());
    parent_dnode->child->push_back(dn);

    /* 如果是新创建的目录文件，则创建.和..两个目录树结点，并挂在新创建的目录结点的子节点数组上 */
    if(file_type == DIR_FILE)
    {
        Dtree_Node* dn1 = new Dtree_Node();
        Dtree_Node* dn2 = new Dtree_Node();

        dn1->is_dir = true;
        dn1->dir_item->inode_no = (*kinode_table)[i_ptr].inode_no;
        std::strncpy(dn1->dir_item->file_name, ".", strlen("."));
        dn1->parent = dn;
        dn->child->push_back(dn1);

        dn2->is_dir = true;
        dn2->dir_item->inode_no = parent_dnode->dir_item->inode_no;
        std::strncpy(dn2->dir_item->file_name, "..", strlen(".."));
        dn2->parent = dn;
        dn->child->push_back(dn2);
    }

    /* 配置文件描述符相关信息 */
    (*user_list)[cur_user].fd_list[fd].fd = fd;
    (*user_list)[cur_user].fd_list[fd].cur_block = 0;
    (*user_list)[cur_user].fd_list[fd].last_fd = (*user_list)[cur_user].cur_fd;
    (*user_list)[cur_user].fd_list[fd].ftable = std::make_shared<struct file_table>();
    (*user_list)[cur_user].fd_list[fd].ftable->file_status_flag = 1;
    (*user_list)[cur_user].fd_list[fd].ftable->offset = 0;
    (*user_list)[cur_user].fd_list[fd].ftable->v_ptr = v_ptr;

    return fd;
}

void Emulated_kernel::my_close(const fd_t fd)
{
    if((*user_list)[cur_user].fd_list[fd].fd == -1)
    {
        err_cout("fd" + std::to_string(fd) + "do not open!");
        return;
    }

    unsigned short vptr = (*user_list)[cur_user].fd_list[fd].ftable->v_ptr;
    unsigned short iptr = (*v_table)[vptr].i_ptr;

    /* 将内核inode结构写回磁盘(这里指的是我们在文件系统对象中维护的inode_list缓冲区) */
    struct inode i_node = static_cast<struct inode>((*kinode_table)[iptr]);
    fs->set_stat((*kinode_table)[iptr].inode_no, (char*)&i_node);

    /* 将使用的kinode和v_table项重置, 其实只要将关键项重置就行，例如我们是根据v_table项的is_used属性判断有没有被使用 */
    (*v_table)[vptr].is_used = false;
    (*v_table)[vptr].i_ptr = 0;

    (*kinode_table)[iptr].process_link = 0;
    (*kinode_table)[iptr].v_ptr = 0;
    (*kinode_table)[iptr].create_time = 0;
    (*kinode_table)[iptr].inode_no = 0;

    /* 清理文件描述符表项 */
    (*user_list)[cur_user].fd_list[fd].fd = -1;
    (*user_list)[cur_user].fd_list[fd].ftable = nullptr;
    memset((*user_list)[cur_user].fd_list[fd].file_buf, '\0', BLOCK_SIZE);

    if(fd == (*user_list)[cur_user].cur_fd)
    {
        (*user_list)[cur_user].cur_fd = (*user_list)[cur_user].fd_list[fd].last_fd;
    }
    (*user_list)[cur_user].fd_list[fd].last_fd = 0;
}

int Emulated_kernel::cat(const std::string &file_name) {
    auto dest_file = parse_path(file_name);
    if(dest_file == nullptr)
    {
        return -1;
    }

    cat_flag = true;

    /* 先将这个文件的所有数据读入当前用户缓冲区user_buf */
    struct inode cur_f = fs->stat(dest_file->dir_item->inode_no);

    /* 记录user_buf操作偏移 */
    unsigned int buf_off = 0;
    char buf[BLOCK_SIZE + 1] = { '\0' };
    memset((*user_list)[cur_user].user_buf, '\0', MAX_FILE_LEN);

    int i = 0;
    for(i = 0; i < 10; ++i)
    {
        bzero(buf, BLOCK_SIZE + 1);
        if(cur_f.iaddr[i] == 0)
        {
            break;
        }

        fs->read_by_blockno(cur_f.iaddr[i], buf);
        memcpy((*user_list)[cur_user].user_buf + buf_off, buf, BLOCK_SIZE);
        buf_off += BLOCK_SIZE;
    }

    memset((*user_list)[cur_user].user_buf + cur_f.length, '\0', BLOCK_SIZE);

    /* 进入查看界面, 显示文件内容的为29 * 120，最后一行为操作行 */
    int row = 45, col = 145;
    char op[20] = { '\0' };
    unsigned int cur_page = 0;
    unsigned int last_page = 0;
    if(cur_f.length == 0)
    {
        last_page = 0;
    }
    else
    {
        last_page = (cur_f.length % (row * col) == 0) ? cur_f.length / (row * col) - 1 : cur_f.length / (row * col);
    }

    while(cat_flag)
    {
        system("clear");

        /* 每一页读取row * col个字节的数据到屏幕, 先将第一页的内容打印在屏幕上 */
        print_page(cur_page);
        bzero(op, 20);

        std::cout << "operation: ";
        //std::cin.ignore(1);
        std::cin >> op;

        if(strcmp(op, "") == 0)
        {
            continue;
        }

        if(cat_map.find(op) == cat_map.end())
        {
            err_cout(" Command not found! ");
            continue;
        }

        switch (cat_map[op]) {
            case 0:
            {
                /* 退出 */
                cat_flag = false;
                system("clear");
                break;
            }
            case 1:
            {
                /* 向上翻页 */
                if(cur_page != 0)
                {
                    cur_page -= 1;
                }
                break;
            }
            case 2:
            {
                /* 向下翻页 */
                if (cur_page != last_page)
                {
                    cur_page += 1;
                }
                break;
            }
            default:
                /* 什么也不干 */
                break;
        }
    }

    return 0;
}

void Emulated_kernel::init_kinode_byinode(const inode &i_node, int i_ptr) {
    (*kinode_table)[i_ptr].type = i_node.type;
    (*kinode_table)[i_ptr].inode_no = i_node.inode_no;
    (*kinode_table)[i_ptr].mode = i_node.mode;
    (*kinode_table)[i_ptr].length = i_node.length;
    (*kinode_table)[i_ptr].uid = i_node.uid;
    (*kinode_table)[i_ptr].create_time = i_node.create_time;
    (*kinode_table)[i_ptr].modified_time = i_node.modified_time;
    (*kinode_table)[i_ptr].ditem_link = i_node.ditem_link;

    for(int i = 0; i < 11; ++i)
    {
        (*kinode_table)[i_ptr].iaddr[i] = i_node.iaddr[i];
    }
}

Emulated_kernel::~Emulated_kernel() {
    /* 将内核中超级块写回磁盘，并同步剩余资源控制信息 */
    syn_sblk();

    /* 清理所有未关闭文件描述符并析构内核目录树 */
    for(int i = 0; i < OPEN_MAX; ++i)
    {
        if((*user_list)[cur_user].fd_list[i].fd != -1)
        {
            my_close(i);
        }
    }

    del_Dtree(root);
    delete root;
}

int Emulated_kernel::my_read(fd_t fd, void *buf, const unsigned int size) {
    if((*user_list)[cur_user].fd_list[fd].fd == -1)
    {
        err_cout("fd" + std::to_string(fd) + "do not open!");
        return -1;
    }

    /* buf的操作偏移 */
    unsigned short buf_off = 0;

    unsigned short vptr = (*user_list)[cur_user].fd_list[fd].ftable->v_ptr;
    unsigned short iptr = (*v_table)[vptr].i_ptr;

    unsigned int read_size = ((*user_list)[cur_user].fd_list[fd].ftable->offset + size <= (*kinode_table)[iptr].length) ? size : (*kinode_table)[iptr].length - (*user_list)[cur_user].fd_list[fd].ftable->offset;
    unsigned short start_block = ((*user_list)[cur_user].fd_list[fd].ftable->offset) / BLOCK_SIZE;
    unsigned short end_block = ((*user_list)[cur_user].fd_list[fd].ftable->offset + read_size - 1) / BLOCK_SIZE;

    /* 要写的起始偏移和最终偏移在其所在块中的位置 */
    unsigned short start_off = (*user_list)[cur_user].fd_list[fd].ftable->offset % BLOCK_SIZE;
    unsigned short end_off = ((*user_list)[cur_user].fd_list[fd].ftable->offset + read_size - 1) % BLOCK_SIZE;

    for(int i = start_block; i <= end_block; ++i)
    {
        /* 如果当前需要的数据所在的块已在缓冲区，可以直接使用 */
        if(i != (*user_list)[cur_user].fd_list[fd].cur_block)
        {
            memset((*user_list)[cur_user].fd_list[fd].file_buf, '\0', BLOCK_SIZE);
            fs->read_by_blockno((*kinode_table)[iptr].iaddr[i], (*user_list)[cur_user].fd_list[fd].file_buf);
            (*user_list)[cur_user].fd_list[fd].cur_block = i;
        }

        if(i == start_block)
        {
            /* 只需操作一个物理盘块 */
            if(start_block == end_block)
            {
                std::strncpy((char*)buf + buf_off, (*user_list)[cur_user].fd_list[fd].file_buf + start_off, read_size);
                buf_off += read_size;
            }
            else
            {
                std::strncpy((char*)buf + buf_off, (*user_list)[cur_user].fd_list[fd].file_buf + start_off, BLOCK_SIZE - start_off);
                buf_off += (BLOCK_SIZE - start_off);
            }
        }
        else if(i == end_block)
        {
            std::strncpy((char*)buf + buf_off, (*user_list)[cur_user].fd_list[fd].file_buf, end_off + 1);
            buf_off += (end_off + 1);
        }
        else
        {
            std::strncpy((char*)buf + buf_off, (*user_list)[cur_user].fd_list[fd].file_buf,  BLOCK_SIZE);
            buf_off += BLOCK_SIZE;
        }
    }

    (*user_list)[cur_user].fd_list[fd].ftable->offset += read_size;

    return read_size;
}

int Emulated_kernel::my_write(fd_t fd, void *buf, const unsigned int size) {
    if((*user_list)[cur_user].fd_list[fd].fd == -1)
    {
        err_cout("fd" + std::to_string(fd) + "do not open!");
        return -1;
    }

    /* buf的操作偏移 */
    unsigned short buf_off = 0;

    unsigned short vptr = (*user_list)[cur_user].fd_list[fd].ftable->v_ptr;
    unsigned short iptr = (*v_table)[vptr].i_ptr;

    unsigned int write_size = ((*user_list)[cur_user].fd_list[fd].ftable->offset + size > MAX_FILE_LEN) ? (MAX_FILE_LEN - (*user_list)[cur_user].fd_list[fd].ftable->offset) : size;
    unsigned short start_block = ((*user_list)[cur_user].fd_list[fd].ftable->offset) / BLOCK_SIZE;
    unsigned short end_block = ((*user_list)[cur_user].fd_list[fd].ftable->offset + write_size - 1) / BLOCK_SIZE;

    /* 要写的起始偏移和最终偏移在其所在块中的位置 */
    unsigned short start_off = (*user_list)[cur_user].fd_list[fd].ftable->offset % BLOCK_SIZE;
    unsigned short end_off = ((*user_list)[cur_user].fd_list[fd].ftable->offset + write_size - 1) % BLOCK_SIZE;

    /* 对索引结点加锁 */
    (*kinode_table)[iptr].locker.lock();

    if(end_block > (*kinode_table)[iptr].block_used)
    {
        int need_size = end_block - (*kinode_table)[iptr].block_used;
        for(int i = 0; i < need_size; ++i)
        {
            int block_no = 0;
            if(block_no = get_a_idleblock() == -1)
            {
                return -1;
            }
            (*kinode_table)[iptr].iaddr[++(*kinode_table)[iptr].block_used] = block_no;
        }
    }

    if((*kinode_table)[iptr].length < (*user_list)[cur_user].fd_list[fd].ftable->offset + write_size)
    {
        (*kinode_table)[iptr].length = (*user_list)[cur_user].fd_list[fd].ftable->offset + write_size;
    }
    (*kinode_table)[iptr].modified_time = time(nullptr);

    for(int i = start_block; i <= end_block; ++i)
    {
        /* 如果当前需要的数据所在的块已在缓冲区，可以直接使用 */
        if(i != (*user_list)[cur_user].fd_list[fd].cur_block)
        {
            memset((*user_list)[cur_user].fd_list[fd].file_buf, '\0', BLOCK_SIZE);
            fs->read_by_blockno((*kinode_table)[iptr].iaddr[i], (*user_list)[cur_user].fd_list[fd].file_buf);
            (*user_list)[cur_user].fd_list[fd].cur_block = i;
        }

        if(i == start_block)
        {
            /* 只需操作一个物理盘块 */
            if(start_block == end_block)
            {
                std::strncpy((*user_list)[cur_user].fd_list[fd].file_buf + start_off, (char*)buf + buf_off, write_size);
                buf_off += write_size;
            }
            else
            {
                std::strncpy((*user_list)[cur_user].fd_list[fd].file_buf + start_off, (char*)buf + buf_off, BLOCK_SIZE - start_off);
                buf_off += (BLOCK_SIZE - start_off);
            }
        }
        else if(i == end_block)
        {
            std::strncpy((*user_list)[cur_user].fd_list[fd].file_buf, (char*)buf + buf_off, end_off + 1);
            buf_off += (end_off + 1);
        }
        else
        {
            std::strncpy((*user_list)[cur_user].fd_list[fd].file_buf, (char*)buf + buf_off, BLOCK_SIZE);
            buf_off += BLOCK_SIZE;
        }

        /* 将调入fd缓冲区的盘块写回磁盘 */
        if(fs->write_by_blockno((*kinode_table)[iptr].iaddr[i], (*user_list)[cur_user].fd_list[fd].file_buf) == -1)
        {
            err_cout("Data block write back failed!");
        }
    }

    (*user_list)[cur_user].fd_list[fd].ftable->offset += write_size;
    struct inode new_inode = static_cast<struct inode>((*kinode_table)[iptr]);
    fs->set_stat((*kinode_table)[iptr].inode_no, (char*)&new_inode);

    /* 对索引结点解锁 */
    (*kinode_table)[iptr].locker.unlock();

    return write_size;
}

int Emulated_kernel::remove_dir(const std::string &dir_name) {
    auto dest_dir = parse_path(dir_name);
    if(dest_dir == nullptr)
    {
        err_cout("No such file or directory!");
        return -1;
    }

    if(dest_dir->child->size() != 2)
    {
        err_cout("rmdir can only delete empty directories!");
        return -1;
    }

    struct DIR rm_dir;
    memset(&rm_dir, '\0', sizeof(struct DIR));

    struct inode rm_inode = fs->stat(dest_dir->dir_item->inode_no);
    if(rm_inode.iaddr[0] == 0)
    {
        /* 第0个盘块一定是根目录占用 */
         err_cout("Incorrect plate number!");
         return -1;
    }
    unsigned short block0_no = rm_inode.iaddr[0];
    fs->read_by_blockno(block0_no, (char*)&rm_dir);

    /* 只有前两个.和..目录项 */
    fs->rm_file(rm_inode.inode_no);
    /* 先在当前目录所对应的目录树结点中删除这个空目录的目录项，到写回磁盘时当前目录的目录项会自动更新 */
    for(auto iter = dest_dir->child->begin(); iter != dest_dir->child->end(); ++iter)
    {
        delete(*iter);
    }
    dest_dir->child->clear();
    dest_dir->parent->child->remove(dest_dir);
    delete dest_dir;

    return 0;
}

int Emulated_kernel::remove(const std::string &file_name) {
    auto dest_node = parse_path(file_name);
    if(dest_node == nullptr)
    {
        err_cout("The file does not exist!");
        return -1;
    }

    /* 普通文件，直接将其从父目录的目录项中删除即可 */
    if(dest_node->is_dir == false)
    {
        fs->rm_file(dest_node->dir_item->inode_no);

        dest_node->parent->child->remove(dest_node);
        delete dest_node;
    }
    /* 目录文件，递归删除 */
    else
    {
        if(del_dir(dest_node) == -1)
        {
            err_cout("Failed to delete catalog file!");
            return -1;
        }
    }

    return 0;
}

void Emulated_kernel::print_page(unsigned short page_no) {
    std::string out = "";

    /* 当前读user_buf的偏移 */
    unsigned int buf_off = page_no * (45 * 145);

    for(int i = 0; i < 45; ++i)
    {
        out.clear();

        for(int j = 0; j < 145; ++j)
        {
            if((*user_list)[cur_user].user_buf[buf_off] == '\0')
            {
                out += " ";
            }
            else
            {
                out += (*user_list)[cur_user].user_buf[buf_off];
            }
            ++buf_off;
        }
        std::cout << out << std::endl;
    }
}

void Emulated_kernel::test_system_call() {
    std::unordered_map<std::string, int> name_to_no = {{"read", 0}, {"write", 1}, {"open", 2}, {"close", 3}, {"lseek", 4}, {"print_fdlist", 5}, {"quit", 6}, {"cat", 7}};

    std::string order_name = " ", arg = "";
    std::vector<std::string> order_arg;
    bool flag = true;
    while(flag)
    {
        order_arg.clear();
        order_name.clear();
        char read_buf[200] = { '\0' };
        std::string line_in;

        std::cout << "Please enter the system call and parameters to be tested: >";
        getline(std::cin, line_in);

        std::stringstream line(line_in);
        line >> order_name;
        while(line >> arg)
        {
            order_arg.push_back(arg);
        }

        if(strcmp(order_name.c_str(), "") == 0)
        {
            continue;
        }

        if(name_to_no.find(order_name) == name_to_no.end())
        {
            err_cout(" Command  not found! ");
            continue;
        }

        switch (name_to_no[order_name])
        {
            case 0:
                if(order_arg.size() != 2)
                {
                    err_cout("Invalid parameter!");
                }
                else
                {
                    my_read(atoi(order_arg[0].c_str()), read_buf, atoi(order_arg[1].c_str()));
                    std::cout << read_buf << std::endl;
                }
                break;
            case 1:
                if(order_arg[1] == "0")
                {
                    char test[] = "helloworld";
                    /* 测试write, 连续写入4096个字符0 */
                    for(int i = 0; i < 1000; ++i)
                    {
                        my_write(atoi(order_arg[0].c_str()), test, strlen(test));
                    }
                }
                else
                {
                    my_write(atoi(order_arg[0].c_str()), (char*)order_arg[1].c_str(), strlen(order_arg[1].c_str()));
                }
                break;
            case 2:
                if(order_arg.size() == 2)
                {
                    open(order_arg[0], atoi(order_arg[1].c_str()));
                }
                else
                {
                    open(order_arg[0]);
                }
                break;
            case 3:
                for(int i = 0; i < order_arg[0].size(); ++i)
                {
                    if(order_arg[0][i] < '0' || order_arg[0][i] > '9')
                    {
                        err_cout("Invalid parameter!");
                    }
                }
                my_close(atoi(order_arg[0].c_str()));
                break;
            case 4:
                if(order_arg.size() != 3)
                {
                    err_cout("Invalid parameter!");
                }
                else
                {
                    my_lseek(atoi(order_arg[0].c_str()), atoi(order_arg[1].c_str()), atoi(order_arg[2].c_str()));
                }
                break;
            case 5:
                print_fdlist();
                break;
            case 6:
                flag = false;
                break;
            case 7:
                if(std::strcmp(order_arg[0].c_str(), "") == 0 || cat(order_arg[0]) == -1)
                {
                    err_cout("The file does not exist!");
                }
                else
                {
                    std::cin.ignore(1);
                }
                break;
            default:
                break;
        }
    }
    std::cout << std::endl;
}

void Emulated_kernel::print_fdlist() {
    unsigned short vptr = 0;
    unsigned short iptr = 0;

    std::cout.width(4);
    std::cout << std::left << "fd";
    std::cout << " ";
    std::cout.width(5);
    std::cout << std::left << "inode_no";
    std::cout << " ";

    std::cout.width(20);
    std::cout << std::right << "filename";
    std::cout << std::endl;

    for(int i = 0; i < OPEN_MAX; ++i)
    {
        if((*user_list)[cur_user].fd_list[i].fd != -1)
        {
            vptr = (*user_list)[cur_user].fd_list[i].ftable->v_ptr;
            iptr = (*v_table)[vptr].i_ptr;

            std::cout.width(4);
            std::cout << std::left <<(*user_list)[cur_user].fd_list[i].fd;
            std::cout << " ";
            std::cout.width(5);
            std::cout << std::right << (*kinode_table)[iptr].inode_no;
            std::cout << " ";

            /* 如果一个文件已经打开，那么他的目录项一定已在内核目录树中 */
            std::string filename = find_dnode_by_inodeno((*kinode_table)[iptr].inode_no)->dir_item->file_name;
            std::cout.width(20);
            std::cout << std::right << filename;
            std::cout << std::endl;
        }
    }
}

Dtree_Node *Emulated_kernel::find_dnode_by_inodeno(unsigned short inode_no) {
    auto cur_node = root;

    std::stack<Dtree_Node*> st;
    st.push(root);

    while(!st.empty())
    {
        cur_node = st.top();
        st.pop();

        for(auto iter = (*cur_node->child).rbegin(); iter != (*cur_node->child).rend(); ++iter)
        {
            st.push(*iter);
        }
        if((*cur_node->dir_item).inode_no == inode_no)
        {
            return cur_node;
        }
    }

    return nullptr;
}

int Emulated_kernel::touch(const std::string &file_name) {
    fd_t fd = creat(file_name);
    if(fd == -1)
    {
        return -1;
    }

    my_close(fd);
    return 0;
}

void Emulated_kernel::kernal_exit() {
    exit_flag = true;
    std::cout << "Now shutting down the system, wait a moment!" << std::endl;
}

int Emulated_kernel::del_dir(Dtree_Node *dir_node) {
    /* 使用两次深度优先遍历删除目录及其中的所有文件 */
    if(strcmp((*dir_node).dir_item->file_name, "/") == 0 || dir_node == nullptr)
    {
        /* 根目录不能删除 */
        err_cout("Failed to delete catalog file ");
        return -1;
    }

    Dtree_Node* old_dir = cur_dir;
    Dtree_Node* cur_node = dir_node;

    /* 先将要删除目录下的所有子孙目录都打开，然后关闭，以将该目录下的所有文件挂在内核目录树上 */
    std::queue<Dtree_Node*> st;
    st.push(dir_node);

    /* 第一次广度优先遍历将该目录下的所有文件挂在内核目录树上 */
    while(!st.empty())
    {
        cur_node = st.front();
        if(cur_node->is_dir == true)
        {
            cur_dir = cur_node->parent;
            parse_path((*cur_node).dir_item->file_name);
            for(auto iter = (*cur_node->child).rbegin(); iter != (*cur_node->child).rend(); ++iter)
            {
                if((*iter)->is_dir == true && (*iter)->child->size() >= 2)
                {
                    st.push(*iter);
                }
            }
        }
        st.pop();
    }

    /* 第二次深度优先遍历递归删除文件 */
    auto dir_parent = dir_node->parent;
    auto it = dir_parent->child->begin();
    for(it; it != dir_parent->child->end(); ++it)
    {
        if(*it == dir_node)
        {
            break;
        }
    }
    dfs_del_file(dir_node);
    dir_parent->child->erase(it);

    cur_dir = old_dir;
    return 0;
}

void Emulated_kernel::dfs_del_file(Dtree_Node *cur_root) {
    if(cur_root == nullptr)
    {
        return;
    }

    std::list<Dtree_Node*>::reverse_iterator iter = (*cur_root->child).rbegin();

    if(cur_root->is_dir)
    {
        for(iter = (*cur_root->child).rbegin(); iter != (*cur_root->child).rend(); ++iter)
        {
            dfs_del_file(*iter);
        }

        cur_root->child->clear();
    }

    if(strcmp(cur_root->dir_item->file_name, ".") != 0 && strcmp(cur_root->dir_item->file_name, "..") != 0)
    {
        fs->rm_file(cur_root->dir_item->inode_no);
    }

    /* 因为是从迭代器rbgin()开始往左删，所以当前删除的目录项一定是父目录的子目录项的最后一个 */
    cur_root->parent = nullptr;
    delete cur_root;
}

void Emulated_kernel::del_Dtree(Dtree_Node* cur_root) {
    if(cur_root == nullptr)
    {
        return;
    }

    if(cur_root->is_dir && strcmp(cur_root->dir_item->file_name, ".") != 0 && strcmp(cur_root->dir_item->file_name, "..") != 0)
    {
        for(auto iter = (*cur_root->child).begin(); iter != (*cur_root->child).end(); ++iter)
        {
            del_Dtree(*iter);
        }

        struct inode cur_f = fs->stat(cur_root->dir_item->inode_no);

        /* 将内核目录树结点的目录项写回磁盘 */
        struct DIR dir_close;
        std::vector<struct DIR_ITEM> ditem_vec;

        Dtree_Node* node = cur_root;
        for(auto dnode = (*node->child).begin(); dnode != (*node->child).end(); ++dnode)
        {
            ditem_vec.push_back(*((*dnode)->dir_item));
        }

        int need = 0;
        int block_num = ditem_vec.size() % (BLOCK_SIZE / DIR_ITEM_SIZE) == 0 ? ditem_vec.size() / (BLOCK_SIZE / DIR_ITEM_SIZE) : ditem_vec.size() / (BLOCK_SIZE / DIR_ITEM_SIZE) + 1;
        int block_used = 0;
        for(int i = 0; i < 10 ; ++i)
        {
            if(cur_f.iaddr[i] == 0)
            {
                if(!(cur_f.inode_no == 0 && i == 0))
                {
                    break;
                }
            }

            ++block_used;
        }

        if((need = (block_num - block_used)) > 0)
        {
            int i = 0;
            for(i = 0; i < need; ++i)
            {
                int block_no = get_a_idleblock();
                if(block_no == -1)
                {
                    /* 只将已分配的数据块所能写入的数据写回磁盘 */
                    break;
                }
                cur_f.iaddr[block_used + i] = block_no;
            }
            block_num = block_used + i;
        }

        block_num = (block_num * BLOCK_SIZE > MAX_FILE_LEN) ? MAX_FILE_LEN / BLOCK_SIZE : block_num;

        for(int i = 0; i < block_num; ++i)
        {
            memset(&dir_close, '\0', sizeof(struct DIR));
            for(int j = 0; j < ((i == block_num - 1) ? (ditem_vec.size() % (BLOCK_SIZE / DIR_ITEM_SIZE)) : (BLOCK_SIZE / DIR_ITEM_SIZE)); ++j)
            {
                dir_close.dir[j] = ditem_vec[i * (BLOCK_SIZE / DIR_ITEM_SIZE) + j];
            }

            /* 根据盘块号将数据写回磁盘 */
            fs->write_by_blockno(cur_f.iaddr[i], (char*)&dir_close);
        }
        cur_f.length = block_num * BLOCK_SIZE;
        cur_f.modified_time = time(nullptr);
        fs->set_stat(node->dir_item->inode_no, (char*)&cur_f);

        cur_root->child->clear();
    }

    if(cur_root != root)
    {
        delete cur_root;
    }
}

void Emulated_kernel::print_Dtree() {
    /* 使用队列的广度优先遍历 */
    std::queue<Dtree_Node*> qu;
    qu.push(root);

    Dtree_Node* cur_node = nullptr;
    while(!qu.empty())
    {
        int size = qu.size();
        for(int i = 0; i < size; ++i)
        {
            cur_node = qu.front();
            qu.pop();
            if(strcmp(cur_node->dir_item->file_name, ".") != 0 && strcmp(cur_node->dir_item->file_name, "..") != 0)
            {
                std::cout << cur_node->dir_item->file_name << (cur_node->is_dir ? "          " : " ");
            }

            for(auto child = cur_node->child->cbegin(); child != cur_node->child->cend(); ++child)
            {
                qu.push(*child);
            }

            if(i == size - 1)
            {
                std::cout << std::endl;
            }
        }
    }
}

void Emulated_kernel::init_root_dir() {
    root->child->clear();

    Dtree_Node* dn1 = new Dtree_Node;
    Dtree_Node* dn2 = new Dtree_Node;

    dn1->is_dir = true;
    dn1->dir_item->inode_no = 0;
    std::strcpy(dn1->dir_item->file_name, ".");
    dn1->parent = root;
    root->child->push_back(dn1);

    dn2->is_dir = true;
    dn2->dir_item->inode_no = 0;
    std::strcpy(dn2->dir_item->file_name, "..");
    dn2->parent = root;
    root->child->push_back(dn2);

    (*kinode_table)[0].iaddr[11] = { 0 };
    (*kinode_table)[0].block_used = 0;
    (*user_list)[cur_user].fd_list[0].cur_block = 0;
    bzero((*user_list)[cur_user].fd_list[0].file_buf, '\0');
}

void Emulated_kernel::syn_sblk()
{
    fs->syn_sblk(*sblk);
}

void Emulated_kernel::show_err() {
    std::cout << (int)err_no << std::endl;
}

Dtree_Node *Emulated_kernel::parse_path(const std::string &path_name) {
    Dtree_Node* res_node = nullptr;
    std::list<std::string> path_file;

    std::string path = path_name;
    for(int i = 0; i < path.size(); ++i)
    {
        if(path[i] == '/')
        {
            path[i] = ' ';
        }
    }

    std::stringstream ss(path);
    std::string temp = "";
    while(ss >> temp)
    {
        path_file.push_back(temp);
    }

    /* 特殊情况: 要ls根目录 */
    if(path_file.empty())
    {
        return root;
    }

    /* 绝对路径, 从目录树中root开始找 */
    if(path_name[0] == '/')
    {
        res_node = root;
    }
    /* 相对路径, 从目录树中cur_dir开始找 */
    else
    {
        res_node = cur_dir;
    }

    while(!path_file.empty())
    {
        auto cur_filename = path_file.front();
        path_file.pop_front();

        auto dnode = (*res_node->child).cbegin();
        for(dnode = (*res_node->child).cbegin(); dnode != (*res_node->child).cend(); ++dnode)
        {
            if(strcmp((*dnode)->dir_item->file_name, cur_filename.c_str()) == 0)
            {
                break;
            }
        }

        if((dnode == (*res_node->child).cend()) || ((*dnode)->is_dir == false && !path_file.empty()))
        {
            /* 路径错误 */
            std::cout << "cd: 1: No such file or directory" << std::endl;
            return nullptr;
        }

        if(strcmp((*dnode)->dir_item->file_name, ".") == 0)
        {
            continue;
        }
        else if(strcmp((*dnode)->dir_item->file_name, "..") == 0)
        {
            if(strcmp(res_node->dir_item->file_name, "/") != 0)
            {
                res_node = res_node->parent;
            }
            continue;
        }

        if((*dnode)->is_dir && (*(*dnode)->child).size() == 0)
        {
            struct inode cur_f = fs->stat((*dnode)->dir_item->inode_no);

            /* 需要从磁盘中调入多少次数据(本系统一次一块4kb)到该文件描述符对应的缓冲区 */
            int call_data_times = cur_f.length / BLOCK_SIZE;
            if(cur_f.length % BLOCK_SIZE != 0)
            {
                ++call_data_times;
            }

            /* 如果子目录项的数目大于0则说明打开过，又关闭了，内核目录树中已存在它的所有子目录项 */
            Dtree_Node* dir_item = nullptr;
            for(int i = 0; i < call_data_times; ++i)
            {
                memset((*user_list)[cur_user].fd_list[0].file_buf, '\0', BLOCK_SIZE);
                if(fs->read_by_blockno(cur_f.iaddr[i], (*user_list)[cur_user].fd_list[0].file_buf) == -1)
                {
                    err_cout("Failed to read dir item!");
                    return nullptr;
                }
                struct DIR d = *(struct DIR*)(*user_list)[cur_user].fd_list[0].file_buf;
                for(int j = 0; j < BLOCK_SIZE / DIR_ITEM_SIZE; ++j)
                {
                    if(d.dir[j].inode_no == 0)
                    {
                        if(!(i == 0 && j == 1 && ((*dnode)->parent == root)))
                        {
                            break;
                        }
                    }

                    /* 将目录项依次挂入内核目录树数据结构 */
                    dir_item = new Dtree_Node();
                    dir_item->parent = (*dnode);
                    dir_item->dir_item = std::make_shared<struct DIR_ITEM>(d.dir[j]);

                    struct inode i = fs->stat(d.dir[j].inode_no);
                    dir_item->is_dir = i.type == DIR_FILE ? true : false;
                    (*dnode)->child->push_back(dir_item);
                }
            }
        }
        res_node = (*dnode);
    }

    return res_node;
}

int Emulated_kernel::move(const std::string &source, const std::string &dest)
{
    Dtree_Node* src_file = parse_path(source);
    if(src_file == nullptr)
    {
        return -1;
    }

    std::string src_fname = source, dest_fname = dest, new_name = "";

    while(src_fname.back() != '/' && !src_fname.empty())
    {
        src_fname.pop_back();
    }
    if(src_fname.back() == '/')
    {
        src_fname.pop_back();
    }

    while(dest_fname.back() != '/' && !dest_fname.empty())
    {
        new_name.push_back(dest_fname.back());
        dest_fname.pop_back();
    }
    if(dest_fname.back() == '/')
    {
        dest_fname.pop_back();
    }
    std::reverse(new_name.begin(), new_name.end());

    Dtree_Node* src_f = parse_path(src_fname);
    Dtree_Node* dest_f = parse_path(dest);
    /* 移动或重命名操作 */
    if(dest_f && src_f)
    {
        if(dest_f != src_f)
        {
            if(src_file->is_dir)
            {
                for(auto iter : (*src_file->child))
                {
                    if (strcmp(iter->dir_item->file_name, "..") == 0)
                    {
                        iter->dir_item->inode_no = dest_f->dir_item->inode_no;
                        break;
                    }
                }
            }
            src_f->child->remove(src_file);
            dest_f->child->push_back(src_file);
            src_file->parent = dest_f;
        }

        /* move的过程中将名称也修改 */
        if(src_file != nullptr && new_name.size() != 0)
        {
            if(find_filename(new_name, dest_f) != -1)
            {
                err_cout("There are duplicate names, modification failed!");
                return -1;
            }

            std::strcpy(src_file->dir_item->file_name, new_name.c_str());
        }
        /* 只是简单的将一个普通文件或目录移到另一个目录下 */
        else if(src_file != nullptr && dest.back() == '/')
        {
            if(dest_f->is_dir == false)
            {
                err_cout("Unsupported operation!");
                return -1;
            }
        }
    }
    else
    {
        /* 源路径或目标路径错误 */
    }

    return 0;
}

int Emulated_kernel::copy(const std::string &source, const std::string &dest) {
    Dtree_Node* src_file = parse_path(source);
    if(src_file == nullptr)
    {
        return -1;
    }

    std::string src_fname = source, dest_fname = dest, new_name = "";

    while(src_fname.back() != '/' && !src_fname.empty())
    {
        src_fname.pop_back();
    }
    if(src_fname.back() == '/')
    {
        src_fname.pop_back();
    }

    while(dest_fname.back() != '/' && !dest_fname.empty())
    {
        new_name.push_back(dest_fname.back());
        dest_fname.pop_back();
    }
    if(dest_fname.back() == '/')
    {
        dest_fname.pop_back();
    }
    std::reverse(new_name.begin(), new_name.end());

    Dtree_Node* src_f = parse_path(src_fname);
    Dtree_Node* dest_f = parse_path(dest_fname);

    if(dest_f && src_f)
    {
        Dtree_Node* dup_src = nullptr;
        if(dest_f != src_f)
        {
            /* 将一个目录下的所有文件拷贝到dest目录下 */
            if(src_file->is_dir)
            {
                for(auto iter : (*src_file->child))
                {
                    if (strcmp(iter->dir_item->file_name, "..") == 0 || strcmp(iter->dir_item->file_name, ".") == 0)
                    {
                        continue;
                    }

                    dup_src = init_copy_dir(dup_src, iter);
                    dup_src->parent = dest_f;
                    dest_f->child->push_back(dup_src);
                    if(iter->is_dir == true)
                    {
                        for(auto it : *dup_src->child)
                        {
                            if (strcmp(it->dir_item->file_name, "..") == 0)
                            {
                                it->dir_item->inode_no = dest_f->dir_item->inode_no;
                                break;
                            }
                        }
                    }
                }
            }
            else
            {
                /* 普通文件拷贝 */
                dup_src = init_copy_dir(dup_src, src_file);
                dup_src->parent = dest_f;
                dest_f->child->push_back(dup_src);
            }
        }
        else
        {
            if(src_file->is_dir == false)
            {
                /* 将普通文件拷贝到自己的目录下，什么都不干 */
                return 0;
            }
        }

        /* move的过程中将名称也修改 */
        if(new_name.size() != 0)
        {
            if(find_filename(new_name, dest_f) != -1)
            {
                err_cout("There are duplicate names, copy failed!");
                return -1;
            }

            std::strcpy(dup_src->dir_item->file_name, new_name.c_str());
        }
    }
    else
    {
        /* 源路径或目标路径错误 */
    }

    return 0;
}

Dtree_Node* Emulated_kernel::init_copy_dir(Dtree_Node *new_tr, Dtree_Node *old_tr) {
    if(old_tr == nullptr)
    {
        err_cout("the old_tr is not a Dtree node!");
        return nullptr;
    }

    /* 先将要被复制的目录下的所有子孙目录都打开，然后关闭，以将该目录下的所有文件挂在内核目录树上 */
    std::queue<Dtree_Node*> st;
    Dtree_Node* cur_node = nullptr;
    Dtree_Node* old_dir = cur_dir;
    st.push(old_tr);

    while(!st.empty())
    {
        cur_node = st.front();
        if(cur_node->is_dir == true)
        {
            cur_dir = cur_node->parent;
            parse_path((*cur_node).dir_item->file_name);
            for(auto iter = (*cur_node->child).rbegin(); iter != (*cur_node->child).rend(); ++iter)
            {
                if((*iter)->is_dir == true && (*iter)->child->size() >= 2)
                {
                    st.push(*iter);
                }
            }
        }
        st.pop();
    }

    new_tr = new Dtree_Node(*old_tr);
    Dtree_Node* cur_tr = new_tr;
    st.push(cur_tr);

    while(!st.empty())
    {
        cur_node = st.front();
        st.pop();

        /* 为新文件申请一个inode结点号 */
        int inode_no = get_a_idleinode();
        if(inode_no == -1)
        {
            return nullptr;
        }
        struct inode in;
        struct inode cur_f = fs->stat(cur_node->dir_item->inode_no);
        in.inode_no = inode_no;
        in.type = cur_node->is_dir ? DIR_FILE : NORMAL_FILE;
        in.length = cur_f.length;

        /* 如果当前结点不是目录结点，就将原文件中的所有数据拷贝到自己的物理块中 */
        if(cur_node->is_dir != true)
        {
            /* 从原文件的物理结点中获取盘块号 */
            for(int i = 0; i < 10; ++i)
            {
                if(cur_f.iaddr[i] == 0)
                {
                    break;
                }

                int block_no = get_a_idleblock();
                if(block_no == -1)
                {
                    return nullptr;
                }
                else
                {
                    memset((*user_list)[cur_user].fd_list[0].file_buf, '\0', BLOCK_SIZE);
                    fs->read_by_blockno(cur_f.iaddr[i], (*user_list)[cur_user].fd_list[0].file_buf);
                    fs->write_by_blockno(block_no, (*user_list)[cur_user].fd_list[0].file_buf);
                    in.iaddr[i] = block_no;
                }
            }
        }
        else
        {
            int block_need = in.length % BLOCK_SIZE == 0 ? in.length / BLOCK_SIZE : in.length / BLOCK_SIZE + 1;
            int i = 0;
            while(block_need)
            {
                int block_no = get_a_idleblock();
                if(block_no == -1)
                {
                    return nullptr;
                }
                in.iaddr[++i] = block_no;
                --block_need;
            }

            for(auto iter : *(cur_node->child))
            {
                if(strcmp((*iter).dir_item->file_name, ".") == 0)
                {
                    iter->dir_item->inode_no = inode_no;
                    continue;
                }
                else if(strcmp((*iter).dir_item->file_name, "..") == 0)
                {
                    if(cur_node != new_tr)
                    {
                        iter->dir_item->inode_no = iter->parent->dir_item->inode_no;
                    }
                    continue;
                }
                else
                {
                    st.push(iter);
                }
            }
        }

        cur_node->dir_item->inode_no = inode_no;
        in.modified_time = in.create_time = time(nullptr);
        fs->set_stat(inode_no, (char*)&in);
    }

    cur_dir = old_dir;
    return new_tr;
}

int Emulated_kernel::get_a_idleinode() {
    if(idle_inode->size() == 0)
    {
        update_sblk(2);
    }

    /* 如果更新之后的inode空闲结点还是0，则是所有的索引结点都用完了 */
    if(idle_inode->size() == 0)
    {
        err_cout("All inode numbers have been used up!");
        return -1;
    }
    else
    {
        int inode_no = idle_inode->front();
        idle_inode->pop_front();
        sblk->idle_inode_num -= 1;
        return inode_no;
    }
}







