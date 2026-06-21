#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include <SDL2/SDL.h>

#define SCALING_FACTOR 10
#define EDGES 12
#define VERTICES 8
#define DIMENSIONS 3
#define WIDTH 1280
#define HEIGHT 720
#define DELTA_ANGLE 0.02f
#define DELTA_DIST 0.1f
#define MOUSEREL_DELTA_ANGLE 0.005f
#define FOV 400.0f

typedef struct Cube {
    float** cubeVertex ;
    int noOfVertices , spatialCoords ;
    int** cubeEdges ;
    int noOfEdges ;
    int scalingFactor ;
} Cube ;

typedef struct Transform {
    float rotX , rotY , rotZ ;
    float posX , posY , posZ ;
    float camX , camY , camZ ;
} Transform ;

typedef struct TempVertices {
    float** cubeVertex ;
    float** projectedVertices ;
    int noOfVertices , spatialCoords ;
} TempVertices ;

Cube* initialiseCube() {
    Cube* cube = malloc(sizeof(Cube)) ;
    cube->noOfEdges = EDGES ;
    cube->noOfVertices = VERTICES ;
    cube->spatialCoords = DIMENSIONS ;

    cube->cubeVertex = (float**) malloc(sizeof(float*) * (cube->noOfVertices)) ;
    for(int i = 0 ; i < cube->noOfVertices ; i++) cube->cubeVertex[i] = (float*) malloc(sizeof(float) * cube->spatialCoords) ;

    cube->cubeEdges = (int**) malloc(sizeof(int*) * cube->noOfEdges) ;
    for(int i = 0 ; i < cube->noOfEdges ; i++) cube->cubeEdges[i] = (int*) malloc(sizeof(int) * (cube->spatialCoords - 1)) ;

    cube->scalingFactor = SCALING_FACTOR ;
    return cube ;
}

void dimensionsOfCube(Cube* cube , const int vertexArr[][3] , const int edgeArr[][2]) {
    for(int i = 0 ; i < cube->noOfVertices ; i++) {
        for(int j = 0 ; j < cube->spatialCoords ; j++) {
            cube->cubeVertex[i][j] = vertexArr[i][j] ;
        }
    }

    for(int i = 0 ; i < cube->noOfEdges ; i++) {
        for(int j = 0 ; j < cube->spatialCoords - 1 ; j++) {
            cube->cubeEdges[i][j] = edgeArr[i][j] ;
        }
    }
}

void PollEvents(SDL_Window** window , int* game_loop , Transform* trans) {
    SDL_Event event ;
    const Uint8* keys = SDL_GetKeyboardState(NULL) ;
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
        else if(event.type == SDL_MOUSEMOTION) {
            trans->rotY += event.motion.xrel * MOUSEREL_DELTA_ANGLE ;
            trans->rotX += event.motion.yrel * MOUSEREL_DELTA_ANGLE ;
        }
        if(keys[SDL_SCANCODE_UP]) trans->rotX -= DELTA_ANGLE ;
        if(keys[SDL_SCANCODE_DOWN]) trans->rotX += DELTA_ANGLE ;
        if(keys[SDL_SCANCODE_RIGHT]) trans->rotY += DELTA_ANGLE ;
        if(keys[SDL_SCANCODE_LEFT]) trans->rotZ -= DELTA_ANGLE ;

        if(keys[SDL_SCANCODE_W]) trans->posZ -= DELTA_DIST ;
        if(keys[SDL_SCANCODE_S]) trans->posZ += DELTA_DIST ;
        if(keys[SDL_SCANCODE_A]) trans->posX -= DELTA_DIST ;
        if(keys[SDL_SCANCODE_D]) trans->posX += DELTA_DIST ;
    }
}

void Rotate(char dir , float angle , float* dim) {
    float x = dim[0] , y = dim[1] , z = dim[2] ;
    if(dir == 'x') {
        dim[1] = y * cos(angle) - z * sin(angle) ;
        dim[2] = y * sin(angle) + z * cos(angle) ;
    }
    else if(dir == 'y') {
        dim[0] = x * cos(angle) + z * sin(angle) ;
        dim[2] = -x * sin(angle) + z * cos(angle) ;
    }
    else {
        dim[0] = x * cos(angle) - y * sin(angle) ;
        dim[1] = x * sin(angle) + y * cos(angle) ;
    }
}

void displayWindow(SDL_Window* window , SDL_Renderer* renderer , Cube* cube , TempVertices* tempCube) {
    SDL_SetRenderDrawColor(renderer , 0 , 0 , 0 , 255) ;
    SDL_RenderClear(renderer) ;
    SDL_SetRenderDrawColor(renderer , 255 , 255 , 255 , 255) ;
    for(int i = 0 ; i < cube->noOfEdges ; i++) {
        float x1 , x2 , y1 , y2 ;
        x1 = tempCube->projectedVertices[cube->cubeEdges[i][0]][0] ;
        y1 = tempCube->projectedVertices[cube->cubeEdges[i][0]][1] ;

        x2 = tempCube->projectedVertices[cube->cubeEdges[i][1]][0] ;
        y2 = tempCube->projectedVertices[cube->cubeEdges[i][1]][1] ;
        SDL_RenderDrawLine(renderer , (int)x1 , (int)y1 , (int)x2 , (int)y2) ;
    }
    SDL_RenderPresent(renderer) ;
}

Transform* initTransformVar(Cube* cube) {
    Transform* cubeTrans = malloc(sizeof(Transform)) ;
    cubeTrans->rotX = 0.0f ;
    cubeTrans->rotY = 0.0f ;
    cubeTrans->rotZ = 0.0f ;

    cubeTrans->posX = 0.0f ;
    cubeTrans->posY = 0.0f ;
    cubeTrans->posZ = 5.0f * cube->scalingFactor ;

    cubeTrans->camX = 0.0f ;
    cubeTrans->camY = 0.0f ;
    cubeTrans->camZ = 0.0f ;

    return cubeTrans ;
}

void RotateAndTranslateCube(Cube* cube , Transform* trans , TempVertices* tempCube) {
    for(int i = 0 ; i < cube->noOfVertices ; i++) {
        tempCube->cubeVertex[i][0] = cube->cubeVertex[i][0] * cube->scalingFactor ;
        tempCube->cubeVertex[i][1] = cube->cubeVertex[i][1] * cube->scalingFactor ;
        tempCube->cubeVertex[i][2] = cube->cubeVertex[i][2] * cube->scalingFactor ;

        Rotate('x' , trans->rotX , tempCube->cubeVertex[i]) ;
        Rotate('y' , trans->rotY , tempCube->cubeVertex[i]) ;
        Rotate('z' , trans->rotZ , tempCube->cubeVertex[i]) ;

        tempCube->cubeVertex[i][0] += trans->posX - trans->camX ;
        tempCube->cubeVertex[i][1] += trans->posY - trans->camY ;
        tempCube->cubeVertex[i][2] += trans->posZ - trans->camZ ;
    }
}

TempVertices* initialiseTempCube() {
    TempVertices* tempcube = malloc(sizeof(TempVertices)) ;
    tempcube->noOfVertices = VERTICES ;
    tempcube->spatialCoords = DIMENSIONS ;

    tempcube->cubeVertex = (float**) malloc(sizeof(float*) * (tempcube->noOfVertices)) ;
    for(int i = 0 ; i < tempcube->noOfVertices ; i++) tempcube->cubeVertex[i] = (float*) malloc(sizeof(float) * tempcube->spatialCoords) ;

    tempcube->projectedVertices = (float**) malloc(sizeof(float*) * (tempcube->noOfVertices)) ;
    for(int i = 0 ; i < tempcube->noOfVertices ; i++) tempcube->projectedVertices[i] = (float*) malloc(sizeof(float) * (tempcube->spatialCoords - 1)) ;

    return tempcube ;
}

void Project3Dto2D(TempVertices* tempCube) {
    for(int i = 0 ; i < tempCube->noOfVertices ; i++) {
        if(tempCube->cubeVertex[i][2] > 0) {
            tempCube->projectedVertices[i][0] = (tempCube->cubeVertex[i][0] * FOV) / (tempCube->cubeVertex[i][2]) + WIDTH / 2 ;
            tempCube->projectedVertices[i][1] = (tempCube->cubeVertex[i][1] * FOV) / (tempCube->cubeVertex[i][2]) + HEIGHT / 2 ;
        }
    }
}

void freeCube(Cube* cube) {
    for(int i = 0 ; i < cube->noOfVertices ; i++) free(cube->cubeVertex[i]) ;
    for(int i = 0 ; i < cube->noOfEdges ; i++) free(cube->cubeEdges[i]) ;
    free(cube->cubeVertex) ;
    free(cube->cubeEdges) ;
    free(cube) ;
}

void freeTempCube(TempVertices* tempCube) {
    for(int i = 0 ; i < tempCube->noOfVertices ; i++) free(tempCube->cubeVertex[i]) ;
    for(int i = 0 ; i < tempCube->noOfVertices ; i++) free(tempCube->projectedVertices[i]) ;
    free(tempCube->cubeVertex) ;
    free(tempCube->projectedVertices) ;
    free(tempCube) ;
}

void freeTransform(Transform* trans) {
    free(trans) ;
}

int main() {
    SDL_Window* pwindow = SDL_CreateWindow("Rotating Cube" , SDL_WINDOWPOS_CENTERED , SDL_WINDOWPOS_CENTERED , WIDTH , HEIGHT , 0) ;
    if(pwindow == NULL) printf("%s\n" , SDL_GetError()) ;

    SDL_Renderer* prenderer = SDL_CreateRenderer(pwindow , -1 , SDL_RENDERER_ACCELERATED | SDL_RENDERER_PRESENTVSYNC) ;
    if(prenderer == NULL) printf("%s\n" , SDL_GetError()) ;

    const int vertexArr[8][3] = {
        {-1, -1, -1} ,
        {-1, -1,  1} ,
        {-1,  1, -1} ,
        {-1,  1,  1} ,
        { 1, -1, -1} ,
        { 1, -1,  1} ,
        { 1,  1, -1} ,
        { 1,  1,  1}
    } ;

    const int edgeArr[12][2] = {
        {0,1}, {0,2} , {0,4} ,
        {1,3} , {1,5} , {2,3} , 
        {2,6} , {3,7} , {4,5} , 
        {4,6} , {5,7} , {6,7} 
    } ;

    Cube* pcube = initialiseCube() ;
    dimensionsOfCube(pcube , vertexArr , edgeArr) ;

    TempVertices* tempcube = initialiseTempCube() ;

    Transform* ptrans = initTransformVar(pcube) ;

    int is_running = 1 ;
    SDL_SetRelativeMouseMode(SDL_TRUE) ;
    while(is_running) {
        PollEvents(&pwindow , &is_running , ptrans) ;
        RotateAndTranslateCube(pcube , ptrans , tempcube) ;
        Project3Dto2D(tempcube) ;
        displayWindow(pwindow , prenderer , pcube , tempcube) ;
    }

    SDL_DestroyRenderer(prenderer) ;
    SDL_Quit() ;
    freeCube(pcube) ;
    freeTempCube(tempcube) ;
    freeTransform(ptrans) ;
    return 0 ;
}

