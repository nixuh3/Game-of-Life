#include "Rules.h"
#include <cctype>

namespace GoL {

RuleSet RuleSet::Parse(std::string_view ruleStr) {
    RuleSet rules;

    enum class ParseMode : uint8_t { NONE, BIRTH, SURVIVAL };
    ParseMode mode = ParseMode::NONE;

    for (char c : ruleStr) {
        if (c == 'B' || c == 'b') {
            mode = ParseMode::BIRTH;
        } else if (c == 'S' || c == 's') {
            mode = ParseMode::SURVIVAL;
        } else if (std::isdigit(static_cast<unsigned char>(c))) {
            int digit = c - '0';
            if (mode == ParseMode::BIRTH) {
                rules.birthMask |= (1 << digit);
            } else if (mode == ParseMode::SURVIVAL) {
                rules.survivalMask |= (1 << digit);
            }
        }
    }

    return rules;
}

}
