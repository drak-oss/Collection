#include "raylib.h"
#include <stdio.h>
#include <stdlib.h>
#include <math.h>

static const int screenWidth = 1280 ;
static const int screenHeight = 720 ;

static const float stiffness = 60.0f ;
static const float restLength = 250.0f ;
static const int points = 4 ;
static const float mu = 1.0f ;
static const float gravity = 10.0f ;

void DrawFloor() {
    static const int borderHeight = 650 ;
    Vector2 startPos = {0,borderHeight} , endPos = {1280,borderHeight} ;
    DrawLineEx(startPos , endPos , 12.0f , RAYWHITE) ;
}

void DrawObject(Vector2 objPos) {
    static const Vector2 objectDim = {100,100} ;
    DrawRectangleV(objPos , objectDim , RED) ;
}

void updateObjPos(float dt , float objVel , Vector2 objPos) {
    objPos.x += objVel * dt ;
}

float applyForces(Vector2 baseRectPos , Vector2 objPos , float objMass , float objVel) {
    float x_delta = (objPos.x - baseRectPos.x) - restLength ;
    float force_x = - stiffness * x_delta - mu * objVel ;

    float acc = force_x / objMass ;
    return acc ;
}

float drawSpring(Vector2 springBasePos , Vector2 objPos , int points , const float coilDist) {
    Vector2 pos = springBasePos ;
    float unit_dist = (objPos.x - springBasePos.x) / points ;
    for(int i = 0 ; i < points ; i++) {
        Vector2 tempmaxPoint = {pos.x + (unit_dist/2) , pos.y - sqrtf(coilDist * coilDist - (unit_dist * unit_dist/4))} ;
        Vector2 tempminPoint = {pos.x + unit_dist , springBasePos.y} ;
        DrawLineEx(pos , tempmaxPoint , 6.0f , RAYWHITE) ;
        DrawLineEx(tempmaxPoint , tempminPoint , 6.0f , RAYWHITE) ;
        pos.x += unit_dist ;
    }
}

int main() {
    InitWindow(screenWidth , screenHeight , "Spring-Mass System") ;
    Vector2 objPos = {60 , 544} ;
    float objVel = 20.0f ;
    float objMass = 10.0f ;

    Vector2 springBasePos = {50,594} ;
    Vector2 springInitMaxpoint = {springBasePos.x + ((objPos.x - springBasePos.x) / (2 *points)) , objPos.y} ;
    float diff_x = springBasePos.x - springInitMaxpoint.x , diff_y = springBasePos.y - springInitMaxpoint.y ;
    const float dist = sqrt(diff_x * diff_x + diff_y * diff_y) ;


    while(!WindowShouldClose()) {
        BeginDrawing() ;
        ClearBackground(BLACK) ;
        DrawText("Spring Mass Simulation" , screenWidth / 10 , 100 , 30 , GREEN); 
        DrawText(TextFormat("Velocity : %.2f" , objVel) , screenWidth / 10 , 150 , 30 , GREEN) ;

        DrawFloor() ;

        Vector2 baseRectpos = {0,544} , baseRectdim = {50,100} ;
        DrawRectangleV(baseRectpos , baseRectdim , BROWN) ;
        drawSpring(springBasePos , objPos , points , dist) ;

        float dt = GetFrameTime() ;
        float objAcc = applyForces(baseRectpos , objPos , objMass , objVel) ;
        objVel += objAcc * dt ;
        objPos.x += objVel * dt ;

        DrawObject(objPos) ;
        EndDrawing() ;
    }
    CloseWindow() ;
    return 0 ;
}
