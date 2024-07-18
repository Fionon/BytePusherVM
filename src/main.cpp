#include <iostream>
#include <vector>
#include <SDL2/SDL.h>

using namespace std;

const int INPUT = 0x0;
const int PC = 0x2;
const int GRAPHIC = 0x5;
const int AUDIO = 0x6;

bool isWaiting = true;

void wait_tick();
void get_input(vector<unsigned char> &RAM);
void cpu(vector<unsigned char> &RAM);
void send_graphics();
void send_audio();

Uint32 timerCallback(Uint32 interval, void *param) {
    cout << "timerCallback is called" << endl;
    if (isWaiting == true) isWaiting = false;
    return interval;
}

int main() {
    cout << "Hello World!"<< endl;
    
    SDL_Init(SDL_INIT_TIMER);
    SDL_TimerID timerID = SDL_AddTimer(1000, timerCallback, &isWaiting);

    SDL_Window* window = SDL_CreateWindow("Bytepusher", SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED, 256, 256, SDL_WINDOW_SHOWN);
    
    vector<unsigned char> RAM(0x1000000, 0);
    
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
    while(1) {
        wait_tick();
        get_input(RAM);
        cpu(RAM);
        send_graphics();
        send_audio();
    }

    for (int x = 0; x < 201; x ++) {
        cout <<x << ":"<< (int)RAM[x] << " " ;
    }
   cout << endl;
   
    SDL_RemoveTimer(timerID);
    return 0;
}

void wait_tick() {
    isWaiting = true;
    cout << "isWaiting is true" << endl;
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

    cout << (int)RAM[INPUT] <<  " " << (int)RAM[INPUT+1] << endl;
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

void send_graphics() {

}

void send_audio() {
    
}
