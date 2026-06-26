#pragma once
#include <string>

struct RHICapabilities {
    std::string deviceName = "N/A";
    bool SupportsMeshShaders = false;
    bool SupportsRayTracing = false;
};

class IRHI
{
public:
    virtual ~IRHI() = default;

    virtual bool Initialize() = 0;
    virtual void Shutdown() = 0;
    virtual void PrintCapabilities() = 0;

    const RHICapabilities& GetCapabilities() const
    {
        return m_Capabilities;
    }

protected:
    RHICapabilities m_Capabilities;
};
