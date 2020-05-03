// Copyright © Allan Moore April 2020

#pragma once

#include <iostream>
#include <string>
#include <iostream>
#include <fstream>
#include <vector>
#include "Types.h"

#include <SDL2/SDL.h>
#include <string>
#include <functional>
#include <cmath>

using String = std::string;

template <class T>
using UniquePtr = std::unique_ptr<T>;

template <class T>
using SharedPtr = std::shared_ptr<T>;

#undef main