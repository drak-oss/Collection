#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>

#define WIDTH 1280
#define HEIGHT 720
#define MAX_AGENTS 12

#define AGENT_HEIGHT 5
#define AGENT_WIDTH 5

#define SCALE 10

typedef struct WalkObj {
    Uint32 color ;
    int cur_x , cur_y ;
    int height , width ;
} WalkObj ;

void PollEvents(SDL_Window** window , int* game_loop) {
    SDL_Event event ;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) *game_loop = 0 ;
        else if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_ESCAPE) *game_loop = 0 ;
        }
        else if(event.type == SDL_WINDOWEVENT_CLOSE) {
            if(*window != NULL) {
                SDL_DestroyWindow(*window) ;
                *window = NULL ;
            }
        }
    }
}

void initialisePlayingWindow(SDL_Window* window , SDL_Surface* surface , int num_agents , WalkObj* AgentPtr) {
    for(int agentCount = 0 ; agentCount < num_agents ; agentCount++) {
        AgentPtr[agentCount].cur_x += 1 ;
        SDL_Rect rect = {AgentPtr[agentCount].cur_x , AgentPtr[agentCount].cur_y , AgentPtr[agentCount].width , AgentPtr[agentCount].height} ;
        SDL_FillRect(surface , &rect , AgentPtr[agentCount].color) ;
    }
    SDL_UpdateWindowSurface(window) ;
}

void SetWalkingObj(WalkObj* agentPtr , int numAgents , const Uint32 colorSet[] , int colorSetSize) {
    if(numAgents > colorSetSize) {
        printf("Color Set Too Small To Accomodate Agents !\n") ;
        return ;
    }
    for(int agentCount = 0 ; agentCount < numAgents ; agentCount++) {
        Uint32 curColor = colorSet[agentCount] ;
        agentPtr[agentCount].color = curColor ;

        agentPtr[agentCount].height = AGENT_HEIGHT ;
        agentPtr[agentCount].width = AGENT_WIDTH ;
        
        agentPtr[agentCount].cur_x = WIDTH/2 - AGENT_WIDTH/2 ;
        agentPtr[agentCount].cur_y = HEIGHT/2 - AGENT_HEIGHT/2 ;
    }
}

void resolveBoundaryProblems(WalkObj* AgentPtr , int agentCount) {
    int max_x = WIDTH - AgentPtr[agentCount].width , min_x = 0 ;
    int max_y = HEIGHT - AgentPtr[agentCount].height , min_y = 0 ;
    if(AgentPtr[agentCount].cur_x > max_x) AgentPtr[agentCount].cur_x = 0 ;
    if(AgentPtr[agentCount].cur_x < min_x) AgentPtr[agentCount].cur_x = max_x ;
    if(AgentPtr[agentCount].cur_y > max_y) AgentPtr[agentCount].cur_y = 0 ;
    if(AgentPtr[agentCount].cur_y < min_y) AgentPtr[agentCount].cur_y = max_y ;
}

void fillRectsInPos(SDL_Window* window , SDL_Surface* surface , WalkObj* AgentPtr , int agentCount , int choice){
    if(choice == 0) {
        for(int i = 0 ; i < SCALE ; i++) {
            AgentPtr[agentCount].cur_x += 1 ;
            SDL_Rect rect = {AgentPtr[agentCount].cur_x , AgentPtr[agentCount].cur_y , AgentPtr[agentCount].width , AgentPtr[agentCount].height} ;
            SDL_FillRect(surface , &rect , AgentPtr[agentCount].color) ;
            SDL_UpdateWindowSurface(window) ;
        }
    }
    else if(choice == 1) {
        for(int i = 0 ; i < SCALE ; i++) {
            AgentPtr[agentCount].cur_x -= 1 ;
            SDL_Rect rect = {AgentPtr[agentCount].cur_x , AgentPtr[agentCount].cur_y , AgentPtr[agentCount].width , AgentPtr[agentCount].height} ;
            SDL_FillRect(surface , &rect , AgentPtr[agentCount].color) ;
        }
    }
    else if(choice == 2) {
        for(int i = 0 ; i < SCALE ; i++) {
            AgentPtr[agentCount].cur_y += 1 ;
            SDL_Rect rect = {AgentPtr[agentCount].cur_x , AgentPtr[agentCount].cur_y , AgentPtr[agentCount].width , AgentPtr[agentCount].height} ;
            SDL_FillRect(surface , &rect , AgentPtr[agentCount].color) ;
        }
    }
    else {
        for(int i = 0 ; i < SCALE ; i++) {
            AgentPtr[agentCount].cur_y -= 1 ;
            SDL_Rect rect = {AgentPtr[agentCount].cur_x , AgentPtr[agentCount].cur_y , AgentPtr[agentCount].width , AgentPtr[agentCount].height} ;
            SDL_FillRect(surface , &rect , AgentPtr[agentCount].color) ;
        }
    }
}

void updatePositions(SDL_Window* window , SDL_Surface* surface , WalkObj* AgentPtr , int numAgents) {
    for(int agentCount = 0 ; agentCount < numAgents ; agentCount++) {
        int choice = rand() % 4 ;
        fillRectsInPos(window , surface , AgentPtr , agentCount , choice) ;
        resolveBoundaryProblems(AgentPtr , agentCount) ;
    }
    SDL_UpdateWindowSurface(window) ;
}

WalkObj* CreateWalkObj(int num_agents , WalkObj* agentPtr) {
    agentPtr = (WalkObj*) malloc(sizeof(WalkObj) * num_agents) ;
    return agentPtr ;
}

int main(int argc , const char* argv[]) {
    int def_agents ;
    if(argc == 1) def_agents = 5 ;
    else if(argc == 2) {
        def_agents = atoi(argv[1]) ;
        if(def_agents > MAX_AGENTS) {
            printf("Max Agents Exceeded !") ;
            return 0 ;
        }
    }
    else printf("Usage : %s 'num of agents'\n" , argv[0]) ;

    SDL_Window *pwindow = SDL_CreateWindow("Random Walk" , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED , WIDTH , HEIGHT , 0) ;
    SDL_Surface *psurface = SDL_GetWindowSurface(pwindow) ;
    if(psurface == NULL) printf("%s\n" , SDL_GetError()) ;
    if(pwindow == NULL) printf("%s\n" , SDL_GetError()) ;
    
    const Uint32 color_set[12] = {
        0xFFFF1A1A, // Bright Red
        0xFF1AFF1A, // Bright Green
        0xFF1A1AFF, // Bright Blue
        0xFFFFFF1A, // Bright Yellow
        0xFF1AFFFF, // Bright Cyan
        0xFFFF1AFF, // Bright Magenta

        0xFFFF7F1A, // Neon Orange
        0xFF7F1AFF, // Neon Purple
        0xFF1AFF7F, // Neon Teal/Greenish Cyan
        0xFFFF4D4D, // Light Neon Red
        0xFF4DFF4D, // Light Neon Green
        0xFF4D4DFF  // Light Neon Blue
    } ;

    WalkObj* agentPtr = CreateWalkObj(def_agents , agentPtr) ;
    SetWalkingObj(agentPtr , def_agents , color_set , MAX_AGENTS) ; 
 
    int is_running = 1 ;
    initialisePlayingWindow(pwindow , psurface , def_agents , agentPtr) ;
    while(is_running) {
        PollEvents(&pwindow , &is_running) ;
        updatePositions(pwindow , psurface , agentPtr , def_agents) ;
    }
    return 0 ;
}