# Notes

## Compiling the Visualizer
With g++:
- no C++ 20
``` Bash
# Dynamic (Copy the dll over)
g++ main.cpp -std=c++20 -I../SDL/include
```
With clang:
``` Bash
# Dynamic (Copy the dll over)
clang-cl .\Code\main.cpp /I ".\SDL\include" /I ".\Code\Include" /std:c++20 /W4 -EHsc /MD /link /LIBPATH:".\SDL\build\Debug" SDL3.lib
```

## Compiling SDL

``` Bash
cmake .. -DSDL_STATIC=ON -DCMAKE_BUILD_TYPE=Debug
cmake --build .
```

## Differences between SDL Versions
### SDL2

```C
// .. entry point

if (SDL_Init(SDL_INIT_VIDEO) < 0) {
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't initialize SDL: %s", SDL_GetError());
    return 1;
}

SDL_Window *window = SDL_CreateWindow(
    "Audio Visualizer", 
    SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 
    300, 300, 
    SDL_WINDOW_ALLOW_HIGHDPI | SDL_WINDOW_ALWAYS_ON_TOP | SDL_WINDOW_BORDERLESS
);
    
SDL_Renderer *renderer = SDL_CreateRenderer(
    window, 
    -1, 
    SDL_RENDERER_ACCELERATED | SDL_RENDERER_TARGETTEXTURE
);

if (!renderer || !window)
{
    SDL_LogError(SDL_LOG_CATEGORY_APPLICATION, "Couldn't create window and renderer: %s", SDL_GetError());
    throw std::runtime_error("Couldn't create window and renderer");
}

std::vector<SDL_FPoint> amplitudes;
amplitudes.resize(FREQUENCIES);

SDL_Event e;
bool running = true;
srand(time(NULL));
while (running)
{
    while( SDL_PollEvent(&e) )
    {
        if ( e.key.keysym.sym == SDLK_ESCAPE ) { running=false; continue;}
        if ( e.type == SDL_QUIT ) {running=false; continue;}
    }
    // Clear Screen
    SDL_SetRenderDrawColor(renderer, 0, 0, 0, 0);
    SDL_RenderClear(renderer);

    // Calculate Geometry
    for (int i=0; i<amplitudes.size(); i++) 
        amplitudes.push_back(
            {
                static_cast<float>(i), randf()* 400.f,
            }
        );
    
    // Visualize
    // SDL_RenderGeometry(renderer, NULL, v.data(), v.size(), i.data(), i.size());
    SDL_RenderDrawLinesF(renderer, amplitudes.data(), amplitudes.size());

    // Show on Screen
    SDL_RenderPresent(renderer);
}


SDL_DestroyRenderer(renderer);
SDL_DestroyWindow(window);

SDL_Quit();

```

### SDL3
Look at `audioVisualizer.h`

## Ressources
https://patorjk.com/software/taag/#p=display&h=2&f=Doom&t=Unit%20Tests