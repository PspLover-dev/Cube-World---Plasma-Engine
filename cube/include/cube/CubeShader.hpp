#pragma once

namespace cube {

class CubeShader {
public:
    CubeShader();
    ~CubeShader();

    bool load();
    void bind() const;
    void unbind() const;

    unsigned program() const { return program_; }

private:
    unsigned program_{0};
    bool loaded_{false};
};

} // namespace cube
