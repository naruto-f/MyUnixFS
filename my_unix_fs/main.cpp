#include "Emulated_kernel.h"


int main(int argc, char* argv[])
{
    /* 考虑到要先析构内核对象再析构文件系统对象，所以先构造fs，再构造内核 */
    Emulated_fs v_fs;
    Emulated_kernel v_kernel;

    v_kernel.mount(&v_fs);

    v_kernel.login();

    v_kernel.start_shell();

    return 0;
}

