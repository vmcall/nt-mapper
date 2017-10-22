// stdafx.h : include file for standard system include files,
// or project specific include files that are used frequently, but
// are changed infrequently
//

#define PTR(t) reinterpret_cast<uintptr_t>(t)
#define PPTR(t) reinterpret_cast<uintptr_t*>(t)

#pragma once

#include "targetver.h"

#include <stdio.h>
#include <tchar.h>
#include <cstdint>
#include <Windows.h>
#include <WInternl.h>
#include <iostream>
#include <memory>
#include <string>
#include <Psapi.h>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <memory>
#include <iterator>
#include <algorithm>
#include <locale>
#include <codecvt>
#include <variant>

#include "logger.hpp"
#include "loadlibrary.hpp"
#include "manualmap.hpp"
#include "process.hpp"
#include "binary_file.hpp"
#include "safe_handle.hpp"


// TODO: reference additional headers your program requires here
