#pragma once

#include <string_view>

namespace GoL {

struct RuleSet {
    uint16_t birthMask = 0;
    uint16_t survivalMask = 0;

    static RuleSet Parse(std::string_view ruleStr);
};

}
