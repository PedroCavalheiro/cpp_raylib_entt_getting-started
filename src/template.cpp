// template.cpp

// Import the Raylib library
#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// Import EnTT ECS
#include <entt/entt.hpp>

int main() {
  // Initialize the window with a size and a name
  InitWindow(800, 450, "raylib + EnTT template");
  // The target fps will make it so your PC doesn't explode running this at
  // 1_000_000 fps
  SetTargetFPS(60);

  // This is all you need to setup the ECS system
  entt::registry registry;
  
  // Logic and Rendering systems should be in separate phases of the game loop
  // Logic first and rendering after so that the player always has the last
  // computed state when it is rendered
  while (!WindowShouldClose()) {
    // Here should be the update systems

    // BeginDrawing is for 2d. 3d is BeginMode3D(camera) which requires
    // us to setup a camera object.
    BeginDrawing();
    // Clearing the framebuffer will prevent previously drawn sprites graphics
    // from appearing on the next frame
    ClearBackground(RAYWHITE);

    // Rendering systems here

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
