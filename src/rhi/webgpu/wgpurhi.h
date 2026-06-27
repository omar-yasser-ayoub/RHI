#pragma once
#include "../irhi.h"
#include <webgpu/webgpu_cpp.h>

class WebGPURHI : public IRHI
{
public:
    bool Initialize() override;
    void Shutdown() override;

private:
    wgpu::Instance m_Instance;
    wgpu::Adapter  m_Adapter;
    wgpu::Device   m_Device;
    wgpu::Queue    m_Queue;
};
