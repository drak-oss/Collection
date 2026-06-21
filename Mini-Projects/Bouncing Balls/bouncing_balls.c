#include <stdio.h>
#include <SDL2/SDL.h>
#include <stdbool.h>
#include <math.h>
#include <time.h>

#define WIDTH 1280
#define HEIGHT 720
#define WHITE 0xffffffff
#define BLACK 0x00000000
#define RED 0xffff0000
#define DELTA_TIME 0.001f
#define GRAVITY 600.0f
#define RESTITUTION_CONST 0.8f
#define MU 0.01f
#define EPS 0.05f
#define FIXED_RADIUS 10
#define MAX_TRAJECTORY_POS 500
#define TRAJ_SIZE 2

typedef struct Circle {
    float Xcenter , yCenter ;
    float radius ;
    float velX , velY ;
} Circle ;

typedef struct TrajectoryPoint {
    float x , y ;
} TrajectoryPoint ;

typedef struct TrajectoryBuffer {
    TrajectoryPoint points[MAX_TRAJECTORY_POS] ;
    int head , count ;
} TrajectoryBuffer ;

int randomInt(int min , int max) {
    return min + rand() % (max - min + 1) ;
}

float randomFloat(float min , float max) {
    return min + ((float) rand() / (float) RAND_MAX) * (max - min) ;
}

void FillCircle(SDL_Surface* surface , Circle* circle , Uint32 color) {
    float xLow = circle->Xcenter - circle->radius ;
    float yLow = circle->yCenter - circle->radius ;
    float xHigh = circle->Xcenter + circle->radius ;
    float yHigh = circle->yCenter + circle->radius ;

    for(int i = (int) xLow ; i < (int) xHigh ; i++) {
        for(int j = (int) yLow ; j < (int) yHigh ; j++) {
            float dist = (i - circle->Xcenter) * (i - circle->Xcenter) + (j - circle->yCenter) * (j - circle->yCenter) ;
            if(dist <= circle->radius * circle->radius) {
                SDL_Rect rect = {i , j , 1 , 1} ;
                SDL_FillRect(surface , &rect , color) ;
            }
        }
    }
}

void FillCircles(SDL_Surface* surface , Circle** circles , int noOfBodies) {
    for(int k = 0 ; k < noOfBodies ; k++) {
        FillCircle(surface , circles[k] , WHITE) ;
    }
}

void CircleStep(Circle** circles , int noOfBodies) {
    for(int i = 0 ; i < noOfBodies ; i++) {
        circles[i]->Xcenter += circles[i]->velX * DELTA_TIME ;
        circles[i]->yCenter += circles[i]->velY * DELTA_TIME ;
        circles[i]->velY += GRAVITY * DELTA_TIME ; 
    }
}

void PollEvents(SDL_Window** window , bool* running) {
    SDL_Event event ;
    while(SDL_PollEvent(&event)) {
        if(event.type == SDL_QUIT) *running = false ;
        else if(event.type == SDL_KEYDOWN) {
            if(event.key.keysym.sym == SDLK_ESCAPE) *running = 0 ;
        }
        else if(event.type == SDL_WINDOWEVENT_CLOSE) {
            if(*window != NULL) {
                SDL_DestroyWindow(*window) ;
                *window = NULL ;
            }
        }
    }
}

void ResolveImpulse(Circle* circle , float nx , float ny) {
    float vn = circle->velX * nx + circle->velY * ny ;
    if(vn > 0) return ;

    float jn = -(1 + RESTITUTION_CONST) * vn ;
    circle->velX += jn * nx ;
    circle->velY += jn * ny ;

    float tx = -ny , ty = nx ;
    float jt = -(circle->velX * tx + circle->velY * ty) ;
    float maxfriction = MU * jn ;
    if(jt > maxfriction) jt = maxfriction ;
    if(jt < -maxfriction) jt = -maxfriction ;

    circle->velX += jt * tx ;
    circle->velY += jt * ty ;
}

void ResolveWallCollisions(Circle** circles , int noOfBodies) {
    for(int i = 0 ; i < noOfBodies ; i++) {
        if(circles[i]->Xcenter > WIDTH - circles[i]->radius) {
            circles[i]->Xcenter = WIDTH - circles[i]->radius ;
            ResolveImpulse(circles[i] , -1.0f , 0.0f) ;
        }
        if(circles[i]->Xcenter < circles[i]->radius) {
            circles[i]->Xcenter = circles[i]->radius ;
            ResolveImpulse(circles[i] , 1.0f , 0.0f) ;
        }
        if(circles[i]->yCenter > HEIGHT - circles[i]->radius) {
            circles[i]->yCenter = HEIGHT - circles[i]->radius ;
            ResolveImpulse(circles[i] , 0.0f , -1.0f) ;
        }
        if(circles[i]->yCenter < circles[i]->radius) {
            circles[i]->yCenter = circles[i]->radius ;
            ResolveImpulse(circles[i] , 0.0f , 1.0f) ;
        }
        if (fabs(circles[i]->velX) < EPS) circles[i]->velX = 0;
        if (fabs(circles[i]->velY) < EPS) circles[i]->velY = 0;
    }
}

void ResolveParticleCollisions(Circle** circles , int noOfBodies) {
    for(int i = 0 ; i < noOfBodies ; i++) {
        for(int j = i + 1 ; j < noOfBodies ; j++) {
            float dx = circles[j]->Xcenter - circles[i]->Xcenter , dy = circles[j]->yCenter - circles[i]->yCenter ;
            float dist2 = dx * dx + dy * dy , r = circles[i]->radius + circles[j]->radius ;
            if(dist2 >= r * r) continue ;

            float dist = sqrtf(dist2) ;
            if(dist == 0.0f) continue ;

            float nx = dx / dist , ny = dy / dist ;
            float rvx = circles[j]->velX - circles[i]->velX , rvy = circles[j]->velY - circles[i]->velY ;
            float vn = rvx * nx + rvy * ny ;
            if(vn > 0) continue ;

            float jn = -(1 + RESTITUTION_CONST) * vn * 0.5f ;
            circles[i]->velX -= jn * nx ;
            circles[i]->velY -= jn * ny ;

            circles[j]->velX += jn * nx ;
            circles[j]->velY += jn * ny ;
            
            float penetration = r - dist ;
            float correction = penetration * 0.5f ;

            circles[i]->Xcenter -= correction * nx ;
            circles[i]->yCenter -= correction * ny ;
            circles[j]->Xcenter += correction * nx ;
            circles[j]->yCenter += correction * ny ;
        }
    }
}

void freeCircles(Circle** circles , int noOfBodies) {
    for(int i = 0 ; i < noOfBodies ; i++) free(circles[i]) ;
    free(circles) ;
}

Circle** initCircles(float fixedRadius , int noOfBodies) {
    Circle** arrcircles = malloc(sizeof(Circle*) * noOfBodies) ;
    for(int i = 0 ; i < noOfBodies ; i++) {
        arrcircles[i] = (Circle*) malloc(sizeof(Circle)) ;
        arrcircles[i]->radius = fixedRadius ;
        arrcircles[i]->Xcenter = randomFloat(arrcircles[i]->radius , WIDTH - arrcircles[i]->radius) ;
        arrcircles[i]->yCenter = randomFloat(arrcircles[i]->radius , HEIGHT - arrcircles[i]->radius) ;
        arrcircles[i]->velX = randomFloat(-100 , 100) ;
        arrcircles[i]->velY = randomFloat(-100 , 100) ;
    }

    return arrcircles ;
}

TrajectoryBuffer* initTrajectoryBuffer(Circle** circles , int noOfBodies) {
    TrajectoryBuffer* trajHeads = malloc(sizeof(TrajectoryBuffer) * noOfBodies) ;
    for(int i = 0 ; i < noOfBodies ; i++) {
        trajHeads[i].points[0] = (TrajectoryPoint) {circles[i]->Xcenter , circles[i]->yCenter} ;
        trajHeads[i].count++ ;
        trajHeads[i].head = (trajHeads[i].head + 1) % MAX_TRAJECTORY_POS ;
    }
    return trajHeads ;
}

void AddTrajectoryPoints(TrajectoryBuffer* tb , Circle** circles , int noOfBodies) {
    for(int i = 0 ; i < noOfBodies ; i++) {
        tb[i].points[tb[i].head] = (TrajectoryPoint) {circles[i]->Xcenter , circles[i]->yCenter} ;
        tb[i].head = (tb[i].head + 1) % MAX_TRAJECTORY_POS ;
        if(tb[i].count < MAX_TRAJECTORY_POS) tb[i].count++ ;
    }
}

void DrawTrajectory(SDL_Surface* surface , TrajectoryBuffer* tb , int noOfBodies) {
    for(int i = 0 ; i < noOfBodies ; i++) {
        if(tb[i].count == 0) return ;
        int start = (tb[i].head - tb[i].count + MAX_TRAJECTORY_POS) % MAX_TRAJECTORY_POS ;
        for(int j = 0 ; j < tb[i].count ; j++) {
            int idx = (start + j) % MAX_TRAJECTORY_POS ;
            float x = tb[i].points[idx].x ;
            float y = tb[i].points[idx].y ;

            int minAlpha = 20 , maxAlpha = 255 ;
            float t = (float)i / (tb[i].count);
            Uint8 alpha = minAlpha + t * (maxAlpha - minAlpha);
            Uint32 trajecColor = SDL_MapRGBA(surface->format , 255 , 0 , 0 , alpha) ;
 
            SDL_Rect trajRect = {(int)x , (int)y , TRAJ_SIZE , TRAJ_SIZE} ;
            SDL_FillRect(surface , &trajRect , trajecColor) ;
        }
    }
}

int main() {
    srand((unsigned int)time(NULL)) ;
    int noOfBodies ;
    printf("Enter The Number Of Balls Rendered In The Simulation : ") ;
    scanf("%d" , &noOfBodies) ;

    SDL_Init(SDL_INIT_VIDEO) ;
    SDL_Window* window = SDL_CreateWindow("Bouncing Ball" , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED , WIDTH , HEIGHT , 0) ;
    SDL_Surface* surface = SDL_GetWindowSurface(window) ;
    SDL_SetSurfaceBlendMode(surface, SDL_BLENDMODE_BLEND);

    Circle** arrcircle = initCircles(FIXED_RADIUS , noOfBodies) ;
    FillCircles(surface , arrcircle , noOfBodies) ;
    SDL_UpdateWindowSurface(window) ;

    TrajectoryBuffer* trajecHeads = initTrajectoryBuffer(arrcircle , noOfBodies) ;

    bool running = true ;
    while(running) {
        SDL_FillRect(surface , NULL , BLACK) ;
        PollEvents(&window , &running) ;
        CircleStep(arrcircle , noOfBodies) ;
        ResolveParticleCollisions(arrcircle , noOfBodies) ;
        ResolveWallCollisions(arrcircle , noOfBodies) ;
        AddTrajectoryPoints(trajecHeads , arrcircle , noOfBodies) ;
        DrawTrajectory(surface , trajecHeads , noOfBodies) ;
        FillCircles(surface , arrcircle , noOfBodies) ;
        SDL_UpdateWindowSurface(window) ;
    }

    freeCircles(arrcircle , noOfBodies) ;
    free(trajecHeads) ;
    SDL_DestroyWindowSurface(window) ;
    SDL_Quit() ;

    return 0 ;
}
