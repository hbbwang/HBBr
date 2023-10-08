#include <stdlib.h>
#include <stdio.h>
#include <time.h>
 
#include "SDL3/SDL.h"
//SDL3 除了Win32(pc)平台可以不再需要SDL_main,其余平台还是有这个规矩，需要包含SDL_main.h
#include "SDL3/SDL_main.h"

#include "RendererCore_Android.h"

int main(int argc, char* argv[])
{
    // SDL_Init(SDL_INIT_EVERYTHING);

    // SDL_Window* window;
    // SDL_Renderer* renderer;

    // if (SDL_CreateWindowAndRenderer(512,512, 0, &window, &renderer) < 0)
    // {
    //     SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_ERROR,"Error","SDL_CreateWindow Failed.",NULL);
    //     exit(2);
    // }

    // /* Main render loop */
    // Uint8 quit = 0;
    // SDL_Event event;
    // while (!quit)
    // {
    //     /* Check for events */
    //     while (SDL_PollEvent(&event))
    //     {
    //         if (event.type == SDL_EVENT_QUIT || event.type == SDL_EVENT_FINGER_DOWN)
    //         {
    //             SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "msg", "SDL_Window quit.", NULL);
    //             quit = 1;
    //         }
    //     }

    //     SDL_SetRenderDrawColor(renderer,255,255,0, SDL_ALPHA_OPAQUE);
    //     SDL_RenderClear(renderer);

    //     SDL_RenderPresent(renderer);
    //     SDL_UpdateWindowSurface(window);

    //     SDL_Delay(10);
    // }
    // SDL_DestroyRenderer(renderer);
    // SDL_DestroyWindow(window);
    // SDL_Quit();

    
    SDL_ShowSimpleMessageBox(SDL_MESSAGEBOX_INFORMATION, "msg", "Hello, This is a SDL3 MessageBox!!", NULL);
    RendererCore_Android* renderer = new RendererCore_Android();

    while (true)
    {
        renderer->RenderUpdate();
    }

    delete renderer;
    renderer = NULL;

    return 0;
}