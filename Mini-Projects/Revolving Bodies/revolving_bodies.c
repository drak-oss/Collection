#include <stdio.h>
#include <stdlib.h>
#include <SDL2/SDL.h>
#include <math.h>

#define WIDTH 1280
#define HEIGHT 720
#define DELTA_TIME 0.1f

#define GRAVITATIONAL_CONST 10000

typedef struct Circle {
    float x0 , y0 ;
    float radius ;
    float velX , velY ;
    float mass ;
    float accX , accY ;
} Circle ;

typedef struct Camera {
    float camX , camY ;
} Camera ;

Circle* initCircle(float x0 , float y0 , float radius , float velX , float velY , float mass) {
    Circle* circle = malloc(sizeof(Circle)) ;
    circle->x0 = x0 ;
    circle->y0 = y0 ;
    circle->radius = radius ;

    circle->velX = velX ;
    circle->velY = velY ;
    circle->mass = mass ;

    circle->accX = 0 ;
    circle->accY = 0 ;
    return circle ;
}

void DrawCircle(SDL_Renderer* r, Circle* circle , Camera* cam) {
    int x = circle->radius ;
    int y = 0 ;
    int d = 1 - circle->radius ;

    int sx = circle->x0 - cam->camX + WIDTH/2 ;
    int sy = circle->y0 - cam->camY + HEIGHT/2 ;

    while (x >= y) {
        SDL_RenderDrawLine(r, sx - x, sy + y, sx + x, sy + y) ;
        SDL_RenderDrawLine(r, sx - x, sy - y, sx + x, sy - y) ;
        SDL_RenderDrawLine(r, sx - y, sy + x, sx + y, sy + x) ;
        SDL_RenderDrawLine(r, sx - y, sy - x, sx + y, sy - x) ;
        y++;
        if (d <= 0) d += 2 * y + 1 ;
        else {
            x--;
            d += 2 * (y - x) + 1 ;
        }
    }
}



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

/*void DrawCircle(SDL_Renderer* renderer , Circle* circle , Camera* cam) {
    int r = circle->radius ;
    int sx = circle->x0 - cam->camX + WIDTH/2 ;
    int sy = circle->y0 - cam->camY + HEIGHT/2 ;
    for(int i = -r ; i <= r ; i++) {
        for(int j = -r ; j <= r ; j++) {
            if(i*i + j*j <= r*r) SDL_RenderDrawPoint(renderer , sx + i , sy + j) ;
        }
    }
}*/

void updateVel(Circle* circle , float forceX , float forceY) {
    circle->accX = forceX / circle->mass ;
    circle->accY = forceY / circle->mass ;
    circle->velX += circle->accX * DELTA_TIME ;
    circle->velY += circle->accY * DELTA_TIME ;
}

void PositionalCorrection(Circle* c1, Circle* c2, float nx, float ny, float dist) {
    float overlap = c1->radius + c2->radius - dist ;
    if (overlap <= 0) return ; 

    float percent = 0.8f ;  
    float slop = 0.01f ;

    float correction = fmaxf(overlap - slop, 0.0f) / (c1->mass + c2->mass) * percent ;

    c1->x0 += correction * c2->mass * nx ;
    c1->y0 += correction * c2->mass * ny ;
    c2->x0 -= correction * c1->mass * nx ;
    c2->y0 -= correction * c1->mass * ny ;
}

/*void CollisionResolver(Circle* circle1 , Circle* circle2 , float nx , float ny , float dist) {
    if(((circle1->velX - circle2->velX) * nx + (circle1->velY - circle2->velY) * ny) > 0) return ;

    float tx = -ny, ty = nx ;
    float velRadial1 = circle1->velX * nx + circle1->velY * ny ;
    float velPerp1 = circle1->velX * tx + circle1->velY * ty ;

    float m1 = circle1->mass , m2 = circle2->mass ;

    float velRadial2 = circle2->velX * nx + circle2->velY * ny ;
    float velPerp2 = circle2->velX * tx + circle2->velY * ty ;

    float velRadial_1 = ((m1 - m2) / (m1 + m2)) * velRadial1 + (2 * m2 / (m1 + m2)) * velRadial2 ;
    float velRadial_2 =  (2 * m1 / (m1 + m2)) * velRadial1 - ((m1 - m2) / (m1 + m2)) * velRadial2 ;

    circle1->velX = velRadial_1 * nx + velPerp1 * tx ;
    circle1->velY = velRadial_1 * ny + velPerp1 * ty ;

    circle2->velX = velRadial_2 * nx + velPerp2 * tx ;
    circle2->velY = velRadial_2 * ny + velPerp2 * ty ;

    float overlap = circle1->radius + circle2->radius - dist ;
    float corr = overlap * 0.5f ;
    circle1->x0 += corr * nx ;
    circle1->y0 += corr * ny ;

    circle2->x0 -= corr * nx ;
    circle2->y0 -= corr * ny ;
}*/

void CollisionCorrection(Circle* c1, Circle* c2, float nx, float ny) {
    float rvx = c1->velX - c2->velX ;
    float rvy = c1->velY - c2->velY ;

    float velAlongNormal = rvx * nx + rvy * ny ;
    if (velAlongNormal > 0) return ;

    float e = 1.0f ;

    float j = -(1 + e) * velAlongNormal ;
    j /= (1 / c1->mass + 1 / c2->mass) ;

    float impulseX = j * nx ;
    float impulseY = j * ny ;

    c1->velX += impulseX / c1->mass ;
    c1->velY += impulseY / c1->mass ;
    c2->velX -= impulseX / c2->mass ;
    c2->velY -= impulseY / c2->mass ;
}


void computeCOM(Circle* circle1 , Circle* circle2 , Camera* cam) {
    float tot_mass = circle1->mass + circle2->mass ;

    cam->camX = (circle1->x0 * circle1->mass + circle2->x0 * circle2->mass) / tot_mass ;
    cam->camY = (circle1->y0 * circle1->mass + circle2->y0 * circle2->mass) / tot_mass ;
}

void CalcForce(Circle* circle1 , Circle* circle2) {
    float dx = circle1->x0 - circle2->x0 , dy = circle1->y0 - circle2->y0 ;
    float sqdist = sqrtf((dx*dx) + (dy*dy)) ;
    if(sqdist < 1e-6f) return ;
    float nx = dx / sqdist , ny = dy / sqdist ;
 
    float penetration = circle1->radius + circle2->radius - sqdist ;
    if(penetration > 0) {
        PositionalCorrection(circle1 , circle2 , nx , ny , sqdist) ;
        CollisionCorrection(circle1 , circle2 , nx , ny) ;
        return ;
    }

    float forceX = GRAVITATIONAL_CONST * circle1->mass * circle2->mass * nx / (sqdist * sqdist) ;
    float forceY = GRAVITATIONAL_CONST * circle1->mass * circle2->mass * ny / (sqdist * sqdist) ;

    updateVel(circle1 , -forceX , -forceY) ;
    updateVel(circle2 , forceX , forceY) ;
}

void RenderWindow(SDL_Window* window , SDL_Renderer* renderer , Circle* circle1 , Circle* circle2 , Camera* cam) {
    SDL_SetRenderDrawColor(renderer , 0 , 0 , 0 , 255) ;
    SDL_RenderClear(renderer) ;

    SDL_SetRenderDrawColor(renderer , 255 , 255 , 255 , 255) ;
    CalcForce(circle1 , circle2) ;
    circle1->x0 += circle1->velX * DELTA_TIME ;
    circle1->y0 += circle1->velY * DELTA_TIME ;

    circle2->x0 += circle2->velX * DELTA_TIME ;
    circle2->y0 += circle2->velY * DELTA_TIME ;

    computeCOM(circle1 , circle2 , cam) ;
    DrawCircle(renderer , circle1 , cam) ;
    DrawCircle(renderer , circle2 , cam) ;
    SDL_RenderPresent(renderer) ;
}

Camera* initCamera(float xCenter , float yCenter) {
    Camera* cam = malloc(sizeof(Camera)) ;
    cam->camX = xCenter ;
    cam->camY = yCenter ;
    return cam ;
}

int main() {
    SDL_Window* pwindow = SDL_CreateWindow("Revolving Bodies" , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED , WIDTH , HEIGHT , 0) ;
    if(pwindow == NULL) printf("%s\n" , SDL_GetError()) ;

    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow , -1 , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) ;
    if(prenderer == NULL) printf("%s\n" , SDL_GetError()) ;

    Circle* circle1 = initCircle(WIDTH/2 + 200, HEIGHT/2 , 10 , 20 , 0 , 10) ;
    Circle* circle2 = initCircle(WIDTH/2 - 200 , HEIGHT/2 , 10 , 0 , 15 , 20) ;

    Camera* cam = initCamera(WIDTH/2 , HEIGHT/2) ;

    int loop_life = 1 ;
    while(loop_life) {
        PollEvents(&pwindow , &loop_life) ;
        RenderWindow(pwindow , prenderer , circle1 , circle2 , cam) ;
    }

}