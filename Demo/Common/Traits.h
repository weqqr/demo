#pragma once

namespace Demo {
template<typename T>
using UnderlyingType = __underlying_type(T);
}
