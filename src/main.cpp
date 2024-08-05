#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <fstream>
#include <semaphore>

using namespace std;

const int INPUT = 0x0;
const int PC = 0x2;
const int GRAPHIC = 0x5;
const int AUDIO = 0x6;

binary_semaphore sem(1);

void wait_tick();
void get_input(vector<unsigned char> &RAM);
void cpu(vector<unsigned char> &RAM);
void send_graphics(vector<unsigned char> &RAM, SDL_Renderer* renderer, SDL_Texture* texture);
void send_audio();

Uint32 timerCallback(Uint32 interval, void *param) {
    sem.release();
    return interval;
}
vector<unsigned char> loadROM() {
    ifstream file("rom/ScrollingLogo.BytePusher", ios::binary);
    if (!file) {
        cerr << "Error opening file\n";
    }
    file.unsetf(std::ios::skipws);

    // get its size:
    std::streampos fileSize;

    file.seekg(0, std::ios::end);
    fileSize = file.tellg();
    file.seekg(0, std::ios::beg);

    // reserve capacity
    vector<unsigned char> RAM;
    RAM.reserve(16*1024*1024 + 8);
    // read the data:
    RAM.insert(RAM.begin(),
               std::istream_iterator<unsigned char>(file),
               std::istream_iterator<unsigned char>());

    file.close();
    return RAM;
}

int main() {
    SDL_Window* window = nullptr;
    SDL_Renderer* renderer = nullptr;
    SDL_Init(SDL_INIT_TIMER | SDL_INIT_VIDEO);
    SDL_TimerID timerID = SDL_AddTimer(1000/60, timerCallback, nullptr);
    SDL_CreateWindowAndRenderer(256,256,0, &window, &renderer);
    SDL_Texture* texture = SDL_CreateTexture(renderer,
                               SDL_PIXELFORMAT_RGBA8888,
                               SDL_TEXTUREACCESS_STREAMING,
                               256, 256); 

    SDL_SetWindowTitle(window, "Alvan, Fion - Bytepusher");
    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);

    vector<unsigned char> RAM = loadROM();
    SDL_Event e;
    bool quit = false;
    while (!quit){
        while (SDL_PollEvent(&e)){
            if (e.type == SDL_QUIT){
                quit = true;
            }
        }
        cout << SDL_GetTicks() << "waiting for tick" << endl;
        wait_tick();
        cout << SDL_GetTicks() <<"getting input" << endl;
        get_input(RAM);
        cout << SDL_GetTicks() << "running cpu" << endl;
        cpu(RAM);
        cout << SDL_GetTicks() << "sending graphics" << endl;
        send_graphics(RAM, renderer, texture);
        cout << SDL_GetTicks() << "finished graphics" << endl;
        send_audio();
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    SDL_RemoveTimer(timerID);
    return 0;
}

void wait_tick() {
    sem.acquire();
}

void get_input(vector<unsigned char> &RAM) {
    SDL_Event event;
    SDL_PumpEvents(); 
    const Uint8 *keyboardState = SDL_GetKeyboardState(NULL);

    RAM[INPUT] = 0;
    RAM[INPUT+1] = 0;

    const SDL_Scancode keys[] = {
        SDL_SCANCODE_0, SDL_SCANCODE_1, SDL_SCANCODE_2, SDL_SCANCODE_3, 
        SDL_SCANCODE_4, SDL_SCANCODE_5, SDL_SCANCODE_6, SDL_SCANCODE_7, 
        SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_B, SDL_SCANCODE_C, 
        SDL_SCANCODE_D, SDL_SCANCODE_A, SDL_SCANCODE_E, SDL_SCANCODE_F
    };
    
    for (int i = 0; i < 8; ++i) {
        if (keyboardState[keys[i]]) {
            RAM[INPUT+1] |= (0b1 << i);
        }
        if (keyboardState[keys[i+8]]) {
            RAM[INPUT] |= (0b1 << i);
        }
    }
}

void cpu(vector<unsigned char> &RAM) {
    for (int i = 0; i < 65536; i++) {
        // if (i < 3) {
        // for (int x = 0; x < 1000; x ++) {
        //     cout << "|" << x << ": "<<(unsigned int)RAM[x] << "| " ;
        // }
        // cout << endl;
        // }
        unsigned int pc_index =  RAM[PC]<<16 | RAM[PC+1]<<8 | RAM[PC+2]; // address of our instruction
        unsigned int A = RAM[pc_index]<<16 | RAM[pc_index+1]<<8 | RAM[pc_index+2]; // 100
        unsigned int B = RAM[pc_index+3]<<16 | RAM[pc_index+4]<<8 | RAM[pc_index+5]; // 120
        
        RAM[B] = RAM[A]; // set B to A

        RAM[PC] = RAM[pc_index + 6];
        RAM[PC+1] = RAM[pc_index + 7];
        RAM[PC+2] = RAM[pc_index + 8];
        // unsigned int next_index = RAM[PC]<<16 | RAM[PC+1]<<8 | RAM[PC+2];
        // if (i <3 ) {

        // cout <<pc_index << " " << next_index << endl;
        // }
    }
}

void send_graphics(vector<unsigned char> &RAM, SDL_Renderer* renderer, SDL_Texture* texture) {
    unsigned char ZZ = RAM[5];

    // vector<unsigned char> RAM;
    // RAM.reserve(fileSize);
    Uint32 frameBuffer[256*256];
    
    for (int y = 0; y<256; ++y) {
        for (int x=0; x<256; ++x) {
            unsigned char pixel = RAM[(ZZ << 16) + (y << 8) + x]; // 0xZZYYXX
            if (pixel >= 216) {
                // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
                frameBuffer[x+y*256] = 255;
            }
            else {
                int blue = pixel % 6;
                int green = (pixel % 36 - blue) / 6;
                int red = (pixel - blue - green * 6 ) / 36;
                frameBuffer[x+y*256] = (Uint32)(red * 0x33 << 24 | green * 0x33 << 16 | blue * 0x33 << 8 | 255);
                if (frameBuffer[x+y*256] != 255) {
                    // cout << frameBuffer[x+y*256] << " ";
                }
                //1714644735
                //
                // SDL_SetRenderDrawColor(renderer, red * 0x33, green * 0x33, blue * 0x33, 255);
            }
            // SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    // cout << "-------" << endl;
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, frameBuffer, 256 * sizeof(Uint32));
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);

    // cout << "Render" << endl;
}

void send_audio() {
    
}
