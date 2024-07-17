#include <iostream>
#include <vector>
// #include <SDL2/SDL.h>

using namespace std;
const int INPUT = 0x0;
const int PC = 0x2;
const int GRAPHIC = 0x5;
const int AUDIO = 0x6;

void wait_tick();
void get_input();
void cpu(vector<char> &RAM);
void send_graphics();
void send_audio();

int main() {
    cout << "Hello World!"<< endl;

    // SDL_Window* window = nullptr;
    // SDL_Renderer* renderer = nullptr;

    // SDL_Init(SDL_INIT_VIDEO);
    // SDL_CreateWindowAndRenderer(640*4, 480*4, 0, &window, &renderer);
    // SDL_RenderSetScale(renderer, 4, 4);

    // SDL_SetRenderDrawColor(renderer, 0, 0, 0, 255);
    // SDL_RenderClear(renderer);

    // SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255);
    // SDL_RenderDrawPoint(renderer, 640/2, 480/2);

    vector<char> RAM(0x1000000, 0);
    
    RAM[PC + 2] = 8;
    RAM[8 + 2] = 100; //Address for A
    RAM[11 + 2] = 120; //Address for B
    RAM[14 + 2] = 8;
    RAM[100] = 5;
    RAM[120] = 2;
   for (int x = 0; x < 201; x ++) {
    cout << (int)RAM[x] << " " ;
   }
   cout << endl;
    // while(1) {
        wait_tick();
        get_input();
        cpu(RAM);
        send_graphics();
        send_audio();
    // }

    for (int x = 0; x < 201; x ++) {
        cout <<x << ":"<< (int)RAM[x] << " " ;
    }
   cout << endl;

    return 0;
}

void wait_tick() {

}

void get_input() {
    
}

void cpu(vector<char> &RAM) {
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

void send_graphics() {

}

void send_audio() {
    
}
