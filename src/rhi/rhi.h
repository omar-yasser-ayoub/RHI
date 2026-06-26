#pragma once
#include <string>

struct RHICapabilities {
    std::string deviceName = "N/A";
    bool SupportsMeshShaders = false;
    bool SupportsRayTracing = false;
};

class RHI {
public:
    RHI();
    ~RHI();

    bool Initialize();
    void Shutdown();
    void PrintCapabilities();

    const RHICapabilities& GetCapabilities() const;

private:
    RHICapabilities m_Capabilities;
};
