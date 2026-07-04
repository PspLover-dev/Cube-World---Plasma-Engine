#include "cube/CubeShader.hpp"

namespace cube {

CubeShader::CubeShader() = default;

CubeShader::~CubeShader() = default;

bool CubeShader::load() {
    // Voxel rendering uses OpenGLEngine fixed pipeline; shader slot reserved for parity.
    loaded_ = true;
    return true;
}

void CubeShader::bind() const {}

void CubeShader::unbind() const {}

} // namespace cube
