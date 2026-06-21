#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <SDL2/SDL_image.h>
#include <SDL2/SDL_ttf.h>
#include <math.h>
#include <stdbool.h>
#include <string.h>

#define WIDTH 1280
#define HEIGHT 720

#define TEXT_INPUT_WIDTH 400
#define TEXT_INPUT_HEIGHT 40
#define LOGO_WIDTH 400
#define LOGO_HEIGHT 170

typedef struct InputBox {
    char* title ;
    SDL_Rect titleRect ;
    SDL_Rect box ;
    char text[256] ;
    bool active ;
} InputBox ;

typedef struct Text {
    SDL_Texture* texture ;
    SDL_Rect rect ;
    TTF_Font *font ;
} Text ;

typedef enum {
    STATE_MENU ,
    STATE_SIMULATION ,
    STATE_OVER ,
    STATET_NONE
} SimulationStateType ;

typedef struct SimulationState {
    SimulationStateType state ;
} SimulationState ;

typedef struct StartMenu {
    SDL_Texture* logo ;
    SDL_Rect logoRect ;
    TTF_Font* font ;
    InputBox Body1 , Body2 ;
    Text InstructionText1 ;
    Text InstructionText2 ;
} StartMenu ;

typedef struct SimOver {

} SimOver ;

StartMenu* initStartMenu(SDL_Renderer* renderer) {
    StartMenu* menu = malloc(sizeof(StartMenu)) ;
        
    SDL_Texture* plogo = IMG_LoadTexture(renderer , "data/e.png") ;
    if(!plogo) printf("IMG_LoadTexture Error: %s\n", IMG_GetError()) ;
    menu->logo = plogo ;

    menu->logoRect.h = LOGO_HEIGHT ;
    menu->logoRect.w = LOGO_WIDTH ;
    menu->logoRect.x = WIDTH/2 - LOGO_WIDTH/2 ;
    menu->logoRect.y = 0 ;
    
    TTF_Font* font = TTF_OpenFont("Arial.ttf" , 24) ;
    menu->font = font ;

    menu->Body1.box = (SDL_Rect) {200 , 250 , TEXT_INPUT_WIDTH , TEXT_INPUT_HEIGHT} ;
    menu->Body1.text[0] = '\0' ;
    menu->Body1.active = false ;
    menu->Body1.title = "Body 1" ;
    menu->Body1.titleRect = (SDL_Rect){menu->Body1.box.x , menu->Body1.box.y - 30 , 0 , 0} ;

    menu->Body2.box = (SDL_Rect) {200 , 250 , TEXT_INPUT_WIDTH , TEXT_INPUT_HEIGHT} ;
    menu->Body2.text[0] = '\0' ;
    menu->Body2.active = false ;
    menu->Body2.title = "Body 2" ;
    menu->Body2.titleRect = (SDL_Rect){menu->Body2.box.x , menu->Body2.box.y - 30 , 0 , 0} ;

    TTF_Font *font = TTF_OpenFont("arial.ttf" , 30) ;
    if(!font) printf("Failed To Load Font : %s\n" , TTF_GetError()) ;
    menu->InstructionText1.font = font ;
    SDL_Color white = {255, 255, 255, 255};

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, "Instructions : Input All Parameters Into Bodies Seperated By ','. The Order Of Input Is As Follows", white);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    menu->InstructionText1.texture = textTexture ;
    menu->InstructionText1.rect.x = 20 ;
    menu->InstructionText1.rect.y = 20 ;
    menu->InstructionText1.rect.w = textSurface->w ;
    menu->InstructionText1.rect.h = textSurface->h ;
    SDL_FreeSurface(textSurface);

    TTF_Font *font = TTF_OpenFont("arial.ttf" , 30) ;
    if(!font) printf("Failed To Load Font : %s\n" , TTF_GetError()) ;
    menu->InstructionText2.font = font ;

    SDL_Surface* textSurface = TTF_RenderText_Blended(font, "Initial X-Center Coord , Initial Y-Center Coord , Radius , Mass , Initial X Velocity , Initial Y Velocity", white);
    SDL_Texture* textTexture = SDL_CreateTextureFromSurface(renderer, textSurface);
    menu->InstructionText2.texture = textTexture ;
    menu->InstructionText2.rect.x = 20 ;
    menu->InstructionText2.rect.y = 50 ;
    menu->InstructionText2.rect.w = textSurface->w ;
    menu->InstructionText2.rect.h = textSurface->h ;
    SDL_FreeSurface(textSurface);
}

SimulationState* initSimulationState() {
    SimulationState* simulation = malloc(sizeof(SimulationState)) ;
    return simulation ;
}

SimOver* initSimOverState() {
    SimOver* simover = malloc(sizeof(SimOver)) ;
    return simover ;
}

void processMenuEvents(SDL_Window** window , StartMenu* menu , int* gameLoop) {
    SDL_Event event ;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_WINDOWEVENT_CLOSE) {
            if(window) {
                SDL_DestroyWindow(window) ;
                window = NULL ;
            }
        }
        if(event.type == SDL_KEYDOWN) if(event.key.keysym.sym == SDLK_ESCAPE) *gameLoop = 0 ;
        if(event.type == SDL_QUIT) *gameLoop = 0 ;

        if(event.type == SDL_MOUSEBUTTONDOWN) {
            int mx = event.button.x ;
            int my = event.button.y ;

            menu->Body1.active =
                (mx >= menu->Body1.box.x &&
                mx <= menu->Body1.box.x + menu->Body1.box.w &&
                my >= menu->Body1.box.y &&
                my <= menu->Body1.box.y + menu->Body1.box.h) ;
            
            menu->Body2.active =
                (mx >= menu->Body2.box.x &&
                mx <= menu->Body2.box.x + menu->Body2.box.w &&
                my >= menu->Body2.box.y &&
                my <= menu->Body2.box.y + menu->Body2.box.h) ;
        }

        if(event.type == SDL_TEXTINPUT && menu->Body1.active) if(strlen(menu->Body1.text) < 255) strcat(menu->Body1.text , event.text.text) ;
        if(event.type == SDL_TEXTINPUT && menu->Body2.active) if(strlen(menu->Body2.text) < 255) strcat(menu->Body2.text , event.text.text) ;

        if (event.type == SDL_KEYDOWN && menu->Body1.active) {
            if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(menu->Body1.text) > 0) {
                menu->Body1.text[strlen(menu->Body1.text) - 1] = '\0';
            }
        }

        if (event.type == SDL_KEYDOWN && menu->Body2.active) {
            if (event.key.keysym.sym == SDLK_BACKSPACE && strlen(menu->Body2.text) > 0) {
                menu->Body2.text[strlen(menu->Body2.text) - 1] = '\0';
            }
        }
    }
}

void RenderInputBoxTitle(SDL_Renderer* renderer , InputBox* box) {
    TTF_Font *font = TTF_OpenFont("arial.ttf" , 30) ;
    if(!font) if(!font) printf("Failed To Load Font : %s\n" , TTF_GetError()) ;
    SDL_Color color = {200, 200, 200, 255} ;
    SDL_Surface* surface = TTF_RenderText_Blended(font, box->title , color) ;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface);
    SDL_Rect dst = {
        box->titleRect.x,
        box->titleRect.y,
        surface->w,
        surface->h
    } ;
    SDL_RenderCopy(renderer, texture, NULL, &dst);

    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void RenderInputBox(SDL_Renderer* renderer , InputBox* ib , StartMenu* menu) {
    SDL_SetRenderDrawColor(renderer, 255, 255, 255, 255) ;
    SDL_RenderDrawRect(renderer, &ib->box) ;

    SDL_Color textColor = {255, 255, 255, 255} ;
    SDL_Surface* surface = TTF_RenderText_Blended(menu->font , ib->text , textColor) ;
    SDL_Texture* texture = SDL_CreateTextureFromSurface(renderer, surface) ;
    SDL_Rect textRect = {
        ib->box.x + 5 ,
        ib->box.y + 5 ,
        surface->w ,
        surface->h
    } ;
    SDL_RenderCopy(renderer, texture, NULL, &textRect);
    SDL_FreeSurface(surface);
    SDL_DestroyTexture(texture);
}

void renderMenuWindow(SDL_Renderer* renderer , StartMenu* menu) {
    SDL_RenderClear(renderer) ;

    SDL_RenderCopy(renderer , menu->logo , NULL , &(menu->logoRect)) ;
    SDL_RenderCopy(renderer, menu->InstructionText1.texture , NULL, &(menu->InstructionText1.rect));
    SDL_RenderCopy(renderer, menu->InstructionText2.texture , NULL, &(menu->InstructionText2.rect));

    RenderInputBoxTitle(renderer , &menu->Body1) ;
    RenderInputBox(renderer , &menu->Body1 , menu) ;
    RenderInputBoxTitle(renderer , &menu->Body2) ;
    RenderInputBox(renderer , &menu->Body2 , menu) ;

    SDL_RenderPresent(renderer) ;
}

int main() {
    SDL_Init(SDL_INIT_VIDEO) ;
    IMG_Init(IMG_INIT_PNG) ;
    TTF_Init();

    SDL_Window* pwindow = SDL_CreateWindow("Orbit Simulation" , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED , WIDTH , HEIGHT , 0) ;
    if(pwindow == NULL) printf("%s\n" , SDL_GetError()) ;

    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow , -1 , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) ;
    if(prenderer == NULL) printf("%s\n" , SDL_GetError()) ;

    StartMenu* menu = initStartMenu(prenderer) ;
    SimulationState* simulation = initSimulationState() ;
    SimOver* simover = initSimOverState() ;


    int gameLoop = 1 ;
    simulation->state = STATE_MENU ;
    while(gameLoop) {
        if(simulation->state == STATE_MENU) {
            SDL_StartTextInput() ;
            processMenuEvents(&pwindow , menu , &gameLoop) ;
            renderMenuWindow(prenderer , menu) ;
        }
        else if(simulation->state == STATE_SIMULATION) {
            //processSimulationEvents(&pwindow , simulation) ;
            //renderSimulationWindow(prenderer , simulation) ;
        }
        else {
            //processSimOverEvents(&pwindow , simover) ;
            //renderSimulationOverWindow(prenderer , simover) ;
        }
    }

    //DestroyAllTextures(menu , simover , simulation) ;
    //FreeMemory(menu , simover , simulation) ;
    SDL_DestroyRenderer(prenderer);
    SDL_DestroyWindow(pwindow);

    IMG_Quit();
    SDL_Quit();
}

