#pragma once

#include <fmt/core.h>

#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>

#include <array>
#include <charconv>
#include <concepts>
#include <deque>
#include <filesystem>
#include <fstream>
#include <initializer_list>
#include <iostream>
#include <map>
#include <span>
#include <stdexcept>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "logging.hpp"

using usize = std::size_t;
using u8 = uint8_t;
using u16 = uint16_t;
using u32 = uint32_t;
using u64 = uint64_t;
using i8 = int8_t;
using i16 = int16_t;
using i32 = int32_t;
using i64 = int64_t;
using f32 = float;
using f64 = double;

using Byte = uint8_t;
using Word = uint16_t;
