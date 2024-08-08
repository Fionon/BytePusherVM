#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <fstream>
#include <mutex>
#include <condition_variable>

using namespace std;

const int INPUT = 0x0;
const int PC = 0x2;
const int GRAPHIC = 0x5;
const int AUDIO = 0x6;

mutex mtx;
condition_variable cv;
bool ready = false;

void wait_tick();
void get_input(vector<unsigned char> &RAM);
void cpu(vector<unsigned char> &RAM);
void send_graphics(vector<unsigned char> &RAM, SDL_Renderer* renderer, SDL_Texture* texture);
void send_audio();

Uint32 timerCallback(Uint32 interval, void *param) {
    {
        unique_lock<mutex> lck(mtx);
        ready = true;
    }
    cv.notify_one();
    return interval;
}
vector<unsigned char> loadROM() {
    ifstream file("rom/KeyboardTest.BytePusher", ios::binary);
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
        wait_tick();
        get_input(RAM);
        cpu(RAM);
        send_graphics(RAM, renderer, texture);
        send_audio();
    }

    // Cleanup
    SDL_DestroyWindow(window);
    SDL_Quit();
    SDL_RemoveTimer(timerID);
    return 0;
}
void wait_tick() {
    unique_lock<mutex> lck(mtx);
    cv.wait(lck,[]{return ready;});
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
        SDL_SCANCODE_8, SDL_SCANCODE_9, SDL_SCANCODE_A, SDL_SCANCODE_B, 
        SDL_SCANCODE_C, SDL_SCANCODE_D, SDL_SCANCODE_E, SDL_SCANCODE_F
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

    unsigned int pc_index =  RAM[PC]<<16 | RAM[PC+1]<<8 | RAM[PC+2]; // address of our instruction
    for (int i = 0; i < 65536; i++) {
   
        unsigned int A = RAM[pc_index]<<16 | RAM[pc_index+1]<<8 | RAM[pc_index+2]; // 100
        unsigned int B = RAM[pc_index+3]<<16 | RAM[pc_index+4]<<8 | RAM[pc_index+5]; // 120
        
        RAM[B] = RAM[A];

        pc_index =  RAM[pc_index+6]<<16 | RAM[pc_index+7]<<8 | RAM[pc_index+8];
    }
}

void send_graphics(vector<unsigned char> &RAM, SDL_Renderer* renderer, SDL_Texture* texture) {
    unsigned char ZZ = RAM[5];

    Uint32 frameBuffer[256*256];
    
    for (int y = 0; y<256; ++y) {
        for (int x=0; x<256; ++x) {
            unsigned char pixel = RAM[(ZZ << 16) + (y << 8) + x]; // 0xZZYYXX
            if (pixel >= 216) {
                frameBuffer[x+y*256] = 255;
            }
            else {
                int blue = pixel % 6;
                int green = (pixel % 36 - blue) / 6;
                int red = (pixel - blue - green * 6 ) / 36;
                frameBuffer[x+y*256] = (Uint32)(red * 0x33 << 24 | green * 0x33 << 16 | blue * 0x33 << 8 | 255);
                if (frameBuffer[x+y*256] != 255) {
                }
            }
        }
    }
    SDL_RenderClear(renderer);
    SDL_UpdateTexture(texture, NULL, frameBuffer, 256 * sizeof(Uint32));
    SDL_RenderCopy(renderer, texture, 0, 0);
    SDL_RenderPresent(renderer);

}

void send_audio() {
    
}
