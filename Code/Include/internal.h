#pragma once

#include <iostream>
#include <cstdint>
#include <format>
#include <vector>
#include <cstring>
#include <algorithm>

#include "audioData.h"
#include "audioPlayer.h"
#include "files.h"

#define SDL_MAIN_USE_CALLBACKS 1  /* use the callbacks instead of main() */
#include <SDL3/SDL.h>
#include <SDL3/SDL_main.h>