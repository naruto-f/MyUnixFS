//
// Created by 123456 on 2021/10/16.
//

#include "Emulated_fs.h"


Emulated_fs::Emulated_fs(const char* fs_file_name, const char* name) : fs_name(name)
{
    fd_fs = open(fs_file_name, O_RDWR);
    if(fd_fs < 0)
    {
        std::cout << "文件打开失败，errno = " << errno << std::endl;
        exit(1);
    }

    /* 初始化各控制块位置哈希表 */
    init_ctrblock_loc();

    /* 为各文件系统中的控制数据结构分配内存空间 */
    bblk = std::make_shared<struct Boot_block>();
    sblk = std::make_shared<struct super_block>();
    b_map = std::make_shared<std::array<std::array<uint32_t, BLOCK_SIZE / sizeof(int)>, GDT_ITEM_NUM>>();
    i_map = std::make_shared<std::array<std::array<uint32_t, BLOCK_SIZE / sizeof(int)>, GDT_ITEM_NUM>>();
    inode_list = std::make_shared<std::array<std::array<struct inode, INODE_NO_MAX / GDT_ITEM_NUM>, GDT_ITEM_NUM>>();
    g_table = std::make_shared<struct GDT_table>();

    uint8_t magic_num = 0;
    int ret = read(fd_fs, (char*)&magic_num, sizeof(magic_num));
    if(ret < -1)
    {
        std::cout << "数据读取失败，errno = " << errno << std::endl;
        exit(1);
    }

    /* 如果系统魔数为0xAA，则将此文件中存放的数据结构初始化此类中的各智能指针，否则格式化新文件系统 */
    if(magic_num != 0xA0)
    {
        if(new_fs() == -1)
        {
            std::cout << "文件系统创建失败！" << std::endl;
            /* 试图再次创建或直接退出程序 */
            exit(1);
        }
        std::cout << "新的文件系统初始化成功！" << std::endl;
    }
    else
    {
        if(init_old_fs() == 0)
        {
            std::cout << "读取数据结构时出现了某些问题！" << std::endl;
            /* 试图再次载入 或直接退出程序 */
            exit(1);
        }
        else
        {
            std::cout << "文件系统载入成功！" << std::endl;
        }
    }
}

int Emulated_fs::new_fs() {
    /* 设置新文件系统引导块中的魔数 */
    bblk->magic_num = 0xA0;

    /* 初始化空的文件系统的超级块和快组描述符表数据结构 */
    reset_sblk_and_gdt();

    /* 将所有数据结构写入磁盘 */
    //int res = write_to_dev();

    return 1;
}

void Emulated_fs::init_ctrblock_loc() {
    control_loc["Boot_block"] = 0;
    control_loc["super_block"] = BLOCK_SIZE / 4;
    control_loc["super_block0"] = BLOCK_SIZE;
    control_loc["super_block1"] = BLOCK_SIZE + SIZE_PER_BLOCKGROUP;
    control_loc["super_block2"] = BLOCK_SIZE + SIZE_PER_BLOCKGROUP * 2;
    control_loc["GDT_table"] = BLOCK_SIZE / 2;
    control_loc["b_map0"] = BLOCK_SIZE * 2;
    control_loc["b_map1"] = BLOCK_SIZE * 2 + SIZE_PER_BLOCKGROUP;
    control_loc["b_map2"] = BLOCK_SIZE * 2 + SIZE_PER_BLOCKGROUP * 2;
    control_loc["i_map0"] = BLOCK_SIZE * 3;
    control_loc["i_map1"] = BLOCK_SIZE * 3 + SIZE_PER_BLOCKGROUP;
    control_loc["i_map2"] = BLOCK_SIZE * 3 + SIZE_PER_BLOCKGROUP * 2;
    control_loc["i_list0"] = BLOCK_SIZE * 4;
    control_loc["i_list1"] = BLOCK_SIZE * 4 + SIZE_PER_BLOCKGROUP;
    control_loc["i_list2"] = BLOCK_SIZE * 4 + SIZE_PER_BLOCKGROUP * 2;
    control_loc["datablock_start0"] = BLOCK_SIZE * (4 + BLOCKNUM_INODELIST);
    control_loc["datablock_start1"] = BLOCK_SIZE * (4 + BLOCKNUM_INODELIST) + SIZE_PER_BLOCKGROUP;
    control_loc["datablock_start2"] = BLOCK_SIZE * (4 + BLOCKNUM_INODELIST) + SIZE_PER_BLOCKGROUP * 2;
}

const int Emulated_fs::get_ctrblock_loc(const std::string block_name) {
    for(int i = 0; i < ctrblock_name.size(); ++i)
    {
        if(block_name == ctrblock_name[i])
        {
            return control_loc[block_name];
        }
    }
    std::cout << "不存在或不允许访问此控制块地址" << std::endl;
    return -1;
}

int Emulated_fs::init_old_fs() {
    int i = 0, ret = 1;
    std::string name1 = "b_map", name2 = "i_map", name3 = "i_list";

    ret = ret & init_lseek_read(ctrblock_name[0], bblk.get(), sizeof(*bblk), "引导块");
    ret = ret & init_lseek_read(ctrblock_name[1], sblk.get(), sizeof(*sblk), "超级块");
    ret = ret & init_lseek_read(ctrblock_name[5], g_table.get(), sizeof(*g_table), "块表描述表");

    for(i = 0; i < GDT_ITEM_NUM; ++i)
    {
        ret = ret & init_lseek_read(name1 + std::to_string(i), &(*b_map)[i], sizeof((*b_map)[i]), std::to_string(i) + "号块组的块位表");
        ret = ret & init_lseek_read(name2 + std::to_string(i), &(*i_map)[i], sizeof((*i_map)[i]), std::to_string(i) + "号块组的索引节点位表");
        ret = ret & init_lseek_read(name3 + std::to_string(i), &(*inode_list)[i], sizeof((*inode_list)[i]), std::to_string(i) + "号块组的索引结点表");
    }

    if(ret != 0)
    {
        /* 将初始超级块中的空闲盘块号和索引结点号加载到空闲链表上 */
        for(int i = 0; i < IDLE_NUM_FSKEEP; ++i)
        {
            if(sblk->block_wait[i] == 0)
            {
                break;
            }
            idle_block.push_back(sblk->block_wait[i]);
        }

        for(int i = 0; i < IDLE_NUM_FSKEEP; ++i)
        {
            if(sblk->inode_wait[i] == 0)
            {
                break;
            }
            idle_inode.push_back(sblk->inode_wait[i]);
        }
    }

    return ret;
}

int Emulated_fs::init_lseek_read(const std::string block_name, void *ptr, size_t size, const std::string arg) {
    int offset = lseek(fd_fs, get_ctrblock_loc(block_name), SEEK_SET);
    if(offset == -1)
    {
        std::cout << "设置偏移量失败，errno = " << errno << std::endl;
        return 0;
    }
    int ret = read(fd_fs, (char*)ptr, size);
    if(ret == -1)
    {
        /* 回头加个功能 : 超级块副本只要有至少一块写入文件成功就行 */
        std::cout << arg << "读出文件失败，errno = " << errno << std::endl;
        return 0;
    }
    std::cout << "从文件中读取了" << ret << "byte数据" << std::endl;
    std::cout << "errno = " << errno << std::endl;
    return 1;
}


super_block Emulated_fs::get_sblk(unsigned char flag) {
    if(flag != 0)
    {
        update_idle(flag);

        int i = 0;
        /* 将idle_inode和idle_block中更新的盘块号读入超级块的对应数组 */
        if(flag == 1)
        {
            int i = 0;
            (*sblk).block_wait = { 0 };
            for(auto iter = idle_block.cbegin(); iter != idle_block.cend(); ++iter)
            {
                (*sblk).block_wait[i++] = *iter;
            }
        }
        else if(flag == 2)
        {
            int i = 0;
            (*sblk).inode_wait = { 0 };
            for(auto iter = idle_inode.cbegin(); iter != idle_inode.cend(); ++iter)
            {
                (*sblk).inode_wait[i++] = *iter;
            }
        }
    }

    /* 更新每个块组的备份超级块 */
    if(update_backup_sblk() == -1)
    {
        std::cout << "sblk backup filed!" << std::endl;
    }
    return (*sblk);
}


int Emulated_fs::Logical_formatting() {
    /* 直接把文件系统调入内存的数据结构块位表和inode位表都置为0，并将超级块和块组描述符表重置，在定时写回磁盘或关闭文件系统时会检查这些数据结构与磁盘中的数据一不一致 */
    *b_map = { 0 };
    *i_map = { 0 };

    /* 将不删除的根目录的初始索引结点0和初始盘块号0按位置1 */
    set_idle_bit("b_map", 0, 1);
    set_idle_bit("i_map", 0, 1);

    /* 将文件系统对象的空闲链表和超级块中的空闲数组清空，等待内核请求更新 */
    idle_inode.clear();
    idle_block.clear();

    sblk->idle_block_num = BLOCK_NO_MAX - 1;
    sblk->idle_inode_num = INODE_NO_MAX - 1;
    sblk->is_mount = true;
    sblk->update_time = time(nullptr);

    return 0;
}

void Emulated_fs::reset_sblk_and_gdt() {
    /* 重置内存中的超级块数据结构 */
    int i = 0;
    /* 初始化并在第一个4kB物理块的第二个1kB中写入源超级块,由于初始时没有任何存储文件和索引结点，所以可以及时分配的物理盘块号和索引结点编号就设为前100个 */
    for(i = 0; i < IDLE_NUM_FSKEEP; ++i)
    {
        idle_inode.push_back(i);
        idle_block.push_back(i);
    }

    i = 0;
    for(auto iter = idle_block.cbegin(); iter != idle_block.cend(); ++iter)
    {
        (*sblk).block_wait[i++] = *iter;
    }
    i = 0;
    for(auto iter = idle_inode.cbegin(); iter != idle_inode.cend(); ++iter)
    {
        (*sblk).inode_wait[i++] = *iter;
    }

    sblk->fs_size = FS_TOTAL_SIZE;
    sblk->block_size = BLOCK_SIZE;
    sblk->idle_block_num = BLOCK_NO_MAX;
    sblk->idle_inode_num = INODE_NO_MAX;
    sblk->update_time = time(nullptr);
    sblk->fs_status = FS_OK;
    strncpy(sblk->fs_typename, fs_name, strlen(fs_name));
    sblk->is_origin = true;

    for(i = 0; i < GDT_ITEM_NUM; ++i)
    {
        sblk->duplication_list[i] = get_ctrblock_loc("super_block" + std::to_string(i));
    }

    /* 重置内存中的块表描述符表数据结构 */
    for(i = 0; i < GDT_ITEM_NUM; ++i)
    {
        /* 初始化每个块组描述符表项的属性 */
        g_table->gdt_table[i].id = i;
        g_table->gdt_table[i].totle_blockno[0] = 1 + i * BLOCKNUM_PER_BLOCK;
        g_table->gdt_table[i].totle_blockno[1] = (i + 1) * BLOCKNUM_PER_BLOCK;
        g_table->gdt_table[i].data_blockno[0] = 1 + 3 + BLOCKNUM_INODELIST + i * BLOCKNUM_PER_BLOCK;
        g_table->gdt_table[i].data_blockno[1] = (i + 1) * BLOCKNUM_PER_BLOCK;
        g_table->gdt_table[i].idle_inode_num = INODE_NO_MAX / 3;
        g_table->gdt_table[i].idle_block_num = BLOCK_NO_MAX / 3;
        for(int j = 0; j < 100; ++j)
        {
            g_table->gdt_table[i].idle_inode[j] = j;
            g_table->gdt_table[i].idle_block[j] = j;
        }
    }
}

int Emulated_fs::Low_level_formatting() {
    /* 调用逻辑格式化接口 */
    Logical_formatting();

    return 0;
}

int Emulated_fs::write_to_dev() {
    int i = 0;
    /* 第一个4kB物理块的第一个1kB存放引导块结构体 */
    off_t offset = lseek(fd_fs, 0, SEEK_SET);
    if(offset == -1)
    {
        std::cout << "设置偏移量失败，errno = " << errno << std::endl;
        return -1;
    }

    int ret = write(fd_fs, (char*)bblk.get(), sizeof(*bblk));
    if(ret == -1)
    {
        std::cout << "数据写入失败，errno = " << errno << std::endl;
        return -1;
    }
    else
    {
        std::cout << "写入" + std::to_string(ret) + "byte数据，sizeof(*bblk) = " + std::to_string(sizeof(*bblk)) <<
                  ", errno = " << errno << std::endl;
    }

    /* 第一个4kB物理块的第2个1kB存放引导块结构体 */
    offset = lseek(fd_fs, 1024, SEEK_SET);
    if(offset == -1)
    {
        std::cout << "设置偏移量失败，errno = " << errno << std::endl;
        return -1;
    }

    ret = write(fd_fs, (char*)sblk.get(), sizeof(*sblk));
    if(ret == -1)
    {
        std::cout << "数据写入失败，errno = " << errno << std::endl;
        return -1;
    }
    else
    {
        std::cout << "写入" + std::to_string(ret) + "byte数据，sizeof(*sblk) = " + std::to_string(sizeof(*sblk)) <<
                  ", errno = " << errno << std::endl;
    }

    /* 先初始化各快组的块位图，inode位图，inode结点表，数据块其实不用初始化，因为用来创建此文件系统的文件被要求格式化为0字符 */
    std::string name1 = "b_map", name2 = "i_map", name3 = "i_list", name4 = "super_block";
    for(i = 0; i < GDT_ITEM_NUM; ++i)
    {
        /* 每一个循环初始化一个块组的块位图，inode位图，inode结点表 */
        offset = lseek(fd_fs, get_ctrblock_loc(name1 + std::to_string(i)), SEEK_SET);
        if(offset == -1)
        {
            std::cout << "设置偏移量失败，errno = " << errno << std::endl;
            return -1;
        }
        ret = write(fd_fs, (char*)&b_map.get()[i], sizeof((*b_map)[i]));
        if(ret == -1)
        {
            std::cout << "块位图结构写入文件失败，errno = " << errno << std::endl;
            return -1;
        }
        else
        {
            std::cout << "写入" + std::to_string(ret) + "byte数据，sizeof((*b_map)[i]) = " + std::to_string(sizeof((*b_map)[i])) <<
                      ", errno = " << errno << std::endl;
        }

        offset = lseek(fd_fs, get_ctrblock_loc(name2 + std::to_string(i)), SEEK_SET);
        if(offset == -1)
        {
            std::cout << "设置偏移量失败，errno = " << errno << std::endl;
            return -1;
        }
        ret = write(fd_fs, (char*)&i_map.get()[i], sizeof((*i_map)[i]));
        if(ret == -1)
        {
            std::cout << "i结点位图结构写入文件失败，errno = " << errno << std::endl;
            return -1;
        }
        else
        {
            std::cout << "写入" + std::to_string(ret) + "byte数据，sizeof((*i_map)[i]) = " + std::to_string(sizeof((*i_map)[i])) <<
                      ", errno = " << errno << std::endl;
        }

        offset = lseek(fd_fs, get_ctrblock_loc(name3 + std::to_string(i)), SEEK_SET);
        if(offset == -1)
        {
            std::cout << "设置偏移量失败，errno = " << errno << std::endl;
            return -1;
        }
        ret = write(fd_fs, (char*)&(*inode_list)[i], sizeof((*inode_list)[i]));
        if(ret == -1)
        {
            std::cout << "数据写入失败，errno = " << errno << std::endl;
            return -1;
        }
        else {
            std::cout << "写入" + std::to_string(offset) + "bytes到第" << i << "块组, while sizeof((*inode_list)[i]) = " << sizeof((*inode_list)[i]) << std::endl;
        }

    }

    if(update_backup_sblk() == -1)
    {
        /* 超级块没有一份成功的备份 */
        std::cout << "超级块备份失败，errno = " << errno << std::endl;
    }

    /* 将块组描述符表写入文件系统第一个物理块的后两个KB中 */
    offset = lseek(fd_fs, get_ctrblock_loc("GDT_table"), SEEK_SET);
    if(offset == -1)
    {
        std::cout << "设置偏移量失败，errno = " << errno << std::endl;
        return -1;
    }
    ret = write(fd_fs, (char*)g_table.get(), sizeof(*g_table));
    if(ret == -1)
    {
        /* 回头加个功能 : 超级块副本只要有至少一块写入文件成功就行 */
        std::cout << "快组描述符写入文件失败，errno = " << errno << std::endl;
        return -1;
    }
    else
    {
        std::cout << "写入" + std::to_string(ret) + "byte数据，sizeof((*gdt_table)[i]) = " + std::to_string(sizeof(*g_table)) <<
                  ", errno = " << errno << std::endl;
    }
    return 0;
}

Emulated_fs::~Emulated_fs()
{
    /* 因为内存中的各控制块数据结构可能与磁盘上存储的对应部分有区别，所以退出前将它们写入磁盘 */
    int res = write_to_dev();
    if(res == -1)
    {
        std::cout << "将控制块写回磁盘时出现问题！" << std::endl;
    }
    close(fd_fs);
}

const long Emulated_fs::blockno_to_offset(unsigned short bno) {
    /* 这个数据盘块所在的块组号 */
    int gdt_no = bno / (BLOCK_NO_MAX / 3);

    /* 在其所在块组的偏移盘块数 */
    int gdt_offset = bno % (BLOCK_NO_MAX / 3);

    return get_ctrblock_loc("datablock_start" + std::to_string(gdt_no)) + gdt_offset * BLOCK_SIZE;
}

int Emulated_fs::update_idle(unsigned char flag) {
    /* flag = 1 更新盘块号, flag = 2更新索引结点号 */
    int i = 0;
    unsigned short no = 0;
    unsigned char need_size = IDLE_NUM_FSKEEP;
    if(flag == 1)
    {
        while(!idle_block.empty())
        {
            no = idle_block.front();
            idle_block.pop_front();

            /* 同步内核和文件系统关于inode结点和数据块的使用信息的差异 */
            set_idle_bit("b_map", no, 1);
        }

        /* 补充空闲链表 */
        if(supplement_idle_list("b_map", IDLE_NUM_FSKEEP) == -1)
        {
            return -1;
        }
    }
    if(flag == 2)
    {
        while(!idle_inode.empty())
        {
            no = idle_inode.front();
            idle_inode.pop_front();

            /* 同步内核和文件系统关于inode结点和数据块的使用信息的差异 */
            set_idle_bit("i_map", no, 1);
        }

        /* 补充空闲链表 */
        if(supplement_idle_list("i_map", IDLE_NUM_FSKEEP) == -1)
        {
            return -1;
        }
    }
    return 0;
}

void Emulated_fs::set_idle_bit(std::string struct_name, unsigned short loc, unsigned char set_value)
{
    /* 因为在内存中管理b_map和i_map的是int的数组，所以要定位在数组中的索引和在int数中的偏移位 */
    unsigned char group_num = loc / (BLOCK_NO_MAX / 3);                    /* 位于哪个快组的快位表或i位表 */
    unsigned short group_offset = loc % (BLOCK_NO_MAX / 3);
    unsigned short int_no = group_offset / 32;
    unsigned short int_offset = group_offset % 32;
    if(struct_name == "b_map")
    {
        if(set_value == 0)
        {
            (*b_map)[group_num][int_no] &= (UINT32_MAX ^ (uint32_t)(0x1 << int_offset));
            ++sblk->idle_block_num;
        }
        else if(set_value == 1)
        {
            (*b_map)[group_num][int_no] |= (uint32_t)(0x1 << int_offset);
            --sblk->idle_block_num;
        }
    }
    else if(struct_name == "i_map")
    {
        if(set_value == 0)
        {
            (*i_map)[group_num][int_no] &= (UINT32_MAX ^ (uint32_t)(0x1 << int_offset));
            ++sblk->idle_inode_num;
        }
        else if(set_value == 1)
        {
            (*i_map)[group_num][int_no] |= (uint32_t)(0x1 << int_offset);
            --sblk->idle_inode_num;
        }
    }
}

int Emulated_fs::supplement_idle_list(const std::string struct_name, unsigned short num) {
    if(struct_name == "b_map")
    {
        for(int i = 0; i < GDT_ITEM_NUM; ++i)
        {
            for(int j = 0; j < BLOCK_SIZE / sizeof(uint32_t); ++j)
            {
                for(int z = 0; z < 32; ++z)
                {
                    if(num == 0)
                    {
                        return 0;
                    }

                    if(((*b_map)[i][j] & (uint32_t)(0x1 << z)) == 0)
                    {
                        /* 找到一个空闲块位置, 将其加入超级块空闲链表 */
                        --num;
                        idle_block.push_back((i * BLOCK_SIZE * 8) + (j * 32) + z);
                    }
                }
            }
        }
    }
    else if(struct_name == "i_map")
    {
        for(int i = 0; i < GDT_ITEM_NUM; ++i)
        {
            for(int j = 0; j < BLOCK_SIZE / sizeof(uint32_t); ++j)
            {
                for(int z = 0; z < 32; ++z)
                {
                    if(num == 0)
                    {
                        return 0;
                    }

                    if(((*i_map)[i][j] & (uint32_t)(0x1 << z)) == 0)
                    {
                        /* 找到一个空闲块位置, 将其加入超级块空闲链表 */
                        --num;
                        idle_inode.push_back((i * BLOCK_SIZE * 8) + (j * 32) + z);
                    }
                }
            }
        }
    }
    return -1;
}

int Emulated_fs::update_backup_sblk()
{
    /* 至少要有一块成功备份 */
    int success_backup = 0;

    struct super_block sblk_duplication = *sblk;
    sblk_duplication.is_origin = false;
    for(int i = 0; i < GDT_ITEM_NUM; ++i)
    {
        sblk_duplication.duplication_list[i] = 0;
    }

    for(int i = 0; i < GDT_ITEM_NUM; ++i)
    {
        off_t offset = lseek(fd_fs, sblk->duplication_list[i], SEEK_SET);
        if(offset == -1)
        {
            if(i == GDT_ITEM_NUM - 1 && success_backup == 0)
            {
                return -1;
            }
        }

        int ret = write(fd_fs, (char*)&sblk_duplication, sizeof(*sblk));
        if(ret == -1)
        {
            if(i == GDT_ITEM_NUM - 1 && success_backup == 0)
            {
                return -1;
            }
        }

        if(ret == sizeof(*sblk))
        {
            ++success_backup;
        }

    }
    return 0;
}

int Emulated_fs::writeroot_to_dev(struct inode *i_node) {
    /* 将初始根目录写入磁盘，按照我们的设计，其一定占用索引结点号为0，数据盘块号为0 */
    (*inode_list)[0][0] = (*i_node);
    struct DIR root;
    memset(&root, '\0', sizeof(root));

    /* 根目录的目录项.和..的索引结点编号都是其自身的索引结点编号 */
    strncpy(root.dir[0].file_name, ".", strlen("."));
    root.dir[0].inode_no = (*i_node).inode_no;
    strncpy(root.dir[1].file_name, "..", strlen(".."));
    root.dir[1].inode_no = (*i_node).inode_no;

    off_t offset = lseek(fd_fs, blockno_to_offset(0), SEEK_SET);
    if(offset == -1)
    {
        return -1;
    }

    int ret = write(fd_fs, (char*)&root, sizeof(root));
    if(ret == -1)
    {
        return -1;
    }

    (*inode_list)[0][0].modified_time = time(nullptr);
    (*inode_list)[0][0].ditem_link = 2;
    return 0;
}

int Emulated_fs::read_by_inode(char *buf, struct inode *i_node = nullptr) {
    if(i_node == nullptr)
    {
        return read_rootdir(buf);
    }
    else
    {
        return read_file(buf, i_node);
    }

    return 0;
}

int Emulated_fs::read_file(char *buf, struct inode *i_node) {
    off_t offset = 0;
    int ret = 0;
    int buf_offset = 0;        /* 往buf里读数据的偏移 */
    for(int i = 0; i < 10; ++i)
    {
        if((*i_node).iaddr[i] == 0)
        {
            if((*i_node).inode_no == 0 && i == 0)
            {
                offset = lseek(fd_fs, blockno_to_offset((*i_node).iaddr[i]), SEEK_SET);
                if(offset == -1)
                {
                    return -1;
                }

                int ret = read(fd_fs, buf, BLOCK_SIZE);
                if(ret == -1)
                {
                    return -1;
                }

                buf_offset += BLOCK_SIZE;
            }
            else
            {
                break;
            }
        }
        else
        {
            offset = lseek(fd_fs, blockno_to_offset((*i_node).iaddr[i]), SEEK_SET);
            if(offset == -1)
            {
                return -1;
            }

            int ret = read(fd_fs, buf + buf_offset, BLOCK_SIZE);
            if(ret == -1)
            {
                return -1;
            }

            buf_offset += BLOCK_SIZE;
        }
    }
    return 0;
}

int Emulated_fs::read_rootdir(char *buf) {
    if(read_file(buf, &(*inode_list)[0][0]) == -1)
    {
        return -1;
    }
    return 0;
}

const struct inode Emulated_fs::stat(unsigned short inode_no) {
    unsigned short group_num = inode_no / (INODE_NO_MAX / GDT_ITEM_NUM);
    unsigned short group_off = inode_no % (INODE_NO_MAX / GDT_ITEM_NUM);

//    返回stat结构体
//    struct file_stat fstat;
//
//    fstat.inode_no = (*inode_list)[group_num][group_off].inode_no;
//    fstat.mode = (*inode_list)[group_num][group_off].mode;
//    fstat.uid = (*inode_list)[group_num][group_off].uid;
//    fstat.length = (*inode_list)[group_num][group_off].length;
//    fstat.modified_time = (*inode_list)[group_num][group_off].modified_time;
//    fstat.successed_get = true;

    return (*inode_list)[group_num][group_off];
}

int Emulated_fs::read_by_blockno(unsigned short blockno, char *buf) {
    int off = blockno_to_offset(blockno);
    memset(buf, '\0', BLOCK_SIZE);

    int offset = lseek(fd_fs, off, SEEK_SET);
    if(offset == -1)
    {
        std::cout << "设置偏移量失败，errno = " << errno << std::endl;
        return -1;
    }

    int ret = read(fd_fs, buf, BLOCK_SIZE);
    if(ret == -1)
    {
        std::cout << "读数据失败，errno = " << errno << std::endl;
        return -1;
    }

    return 0;
}

int Emulated_fs::write_by_blockno(unsigned short blockno, const char *buf) {
    int off = blockno_to_offset(blockno);

    int offset = lseek(fd_fs, off, SEEK_SET);
    if(offset == -1)
    {
        std::cout << "设置偏移量失败，errno = " << errno << std::endl;
        return -1;
    }
    int ret = write(fd_fs, buf, BLOCK_SIZE);
    if(ret == -1)
    {
        std::cout << "数据写入失败，errno = " << errno << std::endl;
        return -1;
    }

    return 0;
}

int Emulated_fs::set_stat(unsigned short inode_no, const char *buf) {
    if(inode_no >= INODE_NO_MAX)
    {
        std::cout << "Index node number is invalid!" << std::endl;
        return -1;
    }

    unsigned short group_num = inode_no / (INODE_NO_MAX / 3);
    unsigned short group_off = inode_no % (INODE_NO_MAX / 3);

    (*inode_list)[group_num][group_off] = *(struct inode*)buf;

    return 0;
}

int Emulated_fs::rm_file(const unsigned short inode_no) {
    /* 释放占用的物理盘块和索引结点，更新相关数据结构 */
    unsigned short group_num = inode_no / (INODE_NO_MAX / GDT_ITEM_NUM);
    unsigned short group_off = inode_no % (INODE_NO_MAX / GDT_ITEM_NUM);

    unsigned int file_off = 0;
    int offset = 0, ret = 0;
    for(int i = 0; i < 11; ++i)
    {
        if((*inode_list)[group_num][group_off].iaddr[i] == 0)
        {
            break;
        }

        /* 清除物理盘块中的数据 */
        char nu[BLOCK_SIZE] = { '\0' };
        write_by_blockno((*inode_list)[group_num][group_off].iaddr[i], nu);

        /* 将b_map中的对应位置为0 */
        set_idle_bit("b_map", (*inode_list)[group_num][group_off].iaddr[i], 0);
    }

    /* 重置索引结点信息 */
    struct inode i;
    (*inode_list)[group_num][group_off] = i;
    set_idle_bit("i_map", inode_no, 0);

    return 0;
}

void Emulated_fs::syn_sblk(const super_block &k_sblk) {
    /* 据上次更新空闲信息后消耗的数据块数和索引结点个数 */
    int used_block = sblk->idle_block_num - k_sblk.idle_block_num;
    int used_inode = sblk->idle_inode_num - k_sblk.idle_inode_num;

    /* 将两个空闲链表pop_front已经使用的数量次 */
    int i = 0;
    for(i = 0; i < used_block; ++i)
    {
        set_idle_bit("b_map", idle_block.front(), 1);
        idle_block.pop_front();
    }

    for(i = 0; i < used_inode; ++i)
    {
        set_idle_bit("i_map", idle_inode.front(), 1);
        idle_inode.pop_front();
    }

    /* 将空闲链表中剩余的部分写回超级块中 */
    sblk->block_wait = { 0 };
    sblk->inode_wait = { 0 };

    i = 0;
    for(auto iter = idle_block.cbegin(); iter != idle_block.cend(); ++iter)
    {
        sblk->block_wait[i] = *iter;
        ++i;
    }

    i = 0;
    for(auto iter = idle_inode.cbegin(); iter != idle_inode.cend(); ++iter)
    {
        sblk->inode_wait[i] = *iter;
        ++i;
    }

    /* 在set_idle_bit函数中已将当前系统剩余资源数量更新至最新，所以不用再减去已使用的数量 */
    if(k_sblk.is_mount == true)
    {
        sblk->is_mount = true;
    }
    sblk->update_time = time(nullptr);
}


