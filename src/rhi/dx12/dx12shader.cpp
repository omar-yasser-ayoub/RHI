#include "rhi/dx12/dx12rhi.h"
#include "third_party/dxc/inc/dxcapi.h"
#include <wrl/client.h>

Microsoft::WRL::ComPtr<IDxcBlob> DX12RHI::CompileShader(DxcBuffer source, std::vector<LPCWSTR> args) {
    Microsoft::WRL::ComPtr<IDxcUtils> utils;

    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(utils.ReleaseAndGetAddressOf()));

    if (FAILED(hr)) {
        printf("Failed to create DXC utils: 0x%x\n", hr);
        return {};
    }

    Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler;

    hr = utils->CreateDefaultIncludeHandler(includeHandler.ReleaseAndGetAddressOf());

    if (FAILED(hr)) {
        printf("Failed to create DXC include handler: 0x%x\n", hr);
        return {};
    }

    Microsoft::WRL::ComPtr<IDxcCompiler3> compiler3;

    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&compiler3));

    if (FAILED(hr)) {
        printf("Failed to create DXC compiler: 0x%x\n", hr);
        return {};
    }

    Microsoft::WRL::ComPtr<IDxcResult> result;

    hr = compiler3->Compile(
        &source,
        args.data(),
        static_cast<UINT32>(args.size()),
        includeHandler.Get(),
        IID_PPV_ARGS(&result)
    );

    if (FAILED(hr) || !result) {
        printf("Failed to compile shader: 0x%x\n", hr);
        return {};
    }

    result->GetStatus(&hr);

    if (FAILED(hr)) {
        return {};
    }

    Microsoft::WRL::ComPtr<IDxcBlobUtf8> errorMsgs;

    result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&errorMsgs), nullptr);

    if (errorMsgs && errorMsgs->GetStringLength()) {
        printf("Shader compilation errors:\n%s\n", errorMsgs->GetStringPointer());
    }

    Microsoft::WRL::ComPtr<IDxcBlob> shaderobj;

    hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderobj), nullptr);

    return shaderobj;
}
