#include <iostream>
#include <vector>
#include <SDL2/SDL.h>
#include <fstream>

using namespace std;

const int INPUT = 0x0;
const int PC = 0x2;
const int GRAPHIC = 0x5;
const int AUDIO = 0x6;

bool isWaiting = true;

void wait_tick();
void get_input(vector<unsigned char> &RAM);
void cpu(vector<unsigned char> &RAM);
void send_graphics(vector<unsigned char> &RAM, SDL_Renderer* renderer);
void send_audio();

Uint32 timerCallback(Uint32 interval, void *param) {
    // cout << "timerCallback is called" << endl;
    if (isWaiting == true) isWaiting = false;
    return interval;
}
vector<unsigned char> loadROM() {
    ifstream file("rom/nyan.bp", ios::binary);
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
    RAM.reserve(fileSize);

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
    SDL_TimerID timerID = SDL_AddTimer(1000, timerCallback, &isWaiting);
    SDL_CreateWindowAndRenderer(256,256,0, &window, &renderer);

    SDL_SetRenderDrawColor(renderer,0,0,0,255);
    SDL_RenderClear(renderer);

    vector<unsigned char> RAM = loadROM();
    
    while(1) {
        wait_tick();
        get_input(RAM);
        cpu(RAM);
        send_graphics(RAM, renderer);
        send_audio();
    }

    SDL_RemoveTimer(timerID);
    return 0;
}

void wait_tick() {
    isWaiting = true;
    cout << "in wait_tick" << endl;
    while (isWaiting == true) {

    }
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
    for (int i = 0; i < 65336; i++) {
        int pc_index =  RAM[PC]<<16 | RAM[PC+1]<<8 | RAM[PC+2]; // address of our instruction
        int A = RAM[pc_index]<<16 | RAM[pc_index+1]<<8 | RAM[pc_index+2]; // 100
        int B = RAM[pc_index+3]<<16 | RAM[pc_index+4]<<8 | RAM[pc_index+5]; // 120
        
        RAM[B] = RAM[A]; // set B to A

        RAM[PC] = RAM[pc_index + 6];
        RAM[PC+1] = RAM[pc_index + 7];
        RAM[PC+2] = RAM[pc_index + 8];
    }
}

void send_graphics(vector<unsigned char> &RAM, SDL_Renderer* renderer) {
    unsigned char ZZ = RAM[5];

    for (int x = 0; x<256; ++x) {
        for (int y=0; y<256; ++y) {
            unsigned char pixel = RAM[(ZZ << 16) + (y << 8) + x]; // 0xZZYYXX
            if (pixel >= 216) {
                SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
            }
            else {
                int blue = pixel % 6;
                int green = (pixel % 36 - blue) / 6;
                int red = (pixel - blue - green * 6 ) / 36;
                SDL_SetRenderDrawColor(renderer, red * 0x33, green * 0x33, blue * 0x33, 255);
            }
            SDL_RenderDrawPoint(renderer, x, y);
        }
    }
    SDL_RenderPresent(renderer);
}

void send_audio() {
    
}
