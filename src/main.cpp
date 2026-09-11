#include <iostream>

#include "rhi/rhi.h"

int main()
{
    RHI rhi;

    if (!rhi.Initialize())
        return 1;

    rhi.Shutdown();

    std::cin.get();
    return 0;
}
