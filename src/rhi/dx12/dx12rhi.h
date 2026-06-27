#pragma once

#include "../IRHI.h"

#include <d3d12.h>
#include <dxgi1_6.h>
#include <wrl/client.h>

class DX12RHI : public IRHI
{
public:
    bool Initialize() override;
    void Shutdown() override;

private:
    Microsoft::WRL::ComPtr<IDXGIFactory6> m_Factory;
    Microsoft::WRL::ComPtr<IDXGIAdapter1> m_Adapter;
    Microsoft::WRL::ComPtr<ID3D12Device> m_Device;
};
