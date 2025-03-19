#pragma once

#include "core/grid_synth.hpp"

namespace gs
{

class editor
{
public:
    editor();
    ~editor() = default;
    void update();
private:
    grid_synth m_synth;
};

}

