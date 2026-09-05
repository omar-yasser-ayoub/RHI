#include "rhi/resources/shader.h"
#include <vector>
namespace rhi {
    struct BindGroupDesc {

    };

    struct BindGroupLayoutDesc {
        std::vector<ShaderReflection> shaderModules;
    };

}
