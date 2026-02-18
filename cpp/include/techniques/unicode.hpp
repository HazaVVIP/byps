#ifndef BYPS_TECHNIQUES_UNICODE_HPP
#define BYPS_TECHNIQUES_UNICODE_HPP

#include <string>
#include <vector>

namespace byps {

class Unicode {
public:
    Unicode();
    std::vector<std::string> generateNormalizationVariations(const std::string& str);
    std::vector<std::string> generateHomographs(const std::string& str);
};

} // namespace byps

#endif // BYPS_TECHNIQUES_UNICODE_HPP
