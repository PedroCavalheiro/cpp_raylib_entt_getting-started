// main.cpp
/**

This template includes some stuff that could be completely useless for your
game, delete everything you want to. Regions marked useless with regions.

//#region useless
// ...
//#endregion

Delete everything between those to start fresh.

If you would like a completely fresh template, delete this file and replace
with template.cpp like `rm src/main.cpp && cp src/template.cpp src/main.cpp`
*/

// Import the Raylib library
#include <raylib.h>
#include <raymath.h>
#define RAYGUI_IMPLEMENTATION
#include <raygui.h>

// Import EnTT ECS
#include <entt/entt.hpp>

//#region useless
// Typically, you have to define stuff before used as c is compiled from top
// to bottom. I'm not entirely sure if c++ is the same but we keep it that way

// This empty struct is basically a marker for our player entity
struct Player {};
// This is also a marker
struct Box {
  Vector2 size;
};

// I wanted to use the `using Position = Vector2` type alias as it would make
// code less verbose but it clashes with EnTT's component architecture
struct Position {
  float x;
  float y;
};
struct Velocity {
  float x;
  float y;
};

// This will be for the box when it is pushed
struct Friction {
  float drag;
};
struct CollidingWithPlayer {
  // You can even use a reference to an entity as a property
  entt::entity player_entity;
};

// An example marker
struct DumbComponent {};

// This is an example system that handles movement of entities by modifying the
// position according to the entity's velocity
void update_position(entt::registry &registry) {
  // This is how we retrieve entities' components from the registry
  // This view function will query the world to find every entity that has
  // these values
  auto view = registry.view<Position, const Velocity>();
  // This is a raylib function to get the time elapsed between last frame and
  // this frame. It will keep the velocity looking constant
  float delta_time = GetFrameTime();

  for (auto [entity, pos, vel] : view.each()) {
    pos.x = pos.x + vel.x * delta_time;
    pos.y = pos.y + vel.y * delta_time;
  }
};

// This system handles the reduction in velocity from the friction
void update_velocity(entt::registry &registry) {
  auto view = registry.view<Velocity, const Friction>();
  float delta_time = GetFrameTime();

  for (auto [entity, vel, friction] : view.each()) {
    vel.x = vel.x - (vel.x * friction.drag * delta_time);
    vel.y = vel.y - (vel.y * friction.drag * delta_time);
  }
};

const auto PLAYER_SIZE = 10.f;

// This is also an example system to draw the player rectangle
void draw_player(entt::registry &registry) {
  auto view = registry.view<const Position, const Velocity, const Player>();

  for (auto [entity, pos, vel] : view.each()) {
    // Draw the rectangle at the center of the position
    // Raylib will typically render with the origin at the top left of the
    // rectangle or sprite so there is some math involved to draw it with the
    // required offset to be at the center of the position
    DrawRectangleRec((Rectangle){pos.x - PLAYER_SIZE / 2,
                                 pos.y - PLAYER_SIZE / 2, PLAYER_SIZE,
                                 PLAYER_SIZE},
                     BLUE);
    // You can replace this later with a sprite or even player states so that
    // the sprite is flipped when they move left to right
  }
}

const auto PLAYER_MOVE_SPEED = 120.f;

void read_player_input(entt::registry &registry) {
  auto view = registry.view<Velocity, const Player>();

  Vector2 direction = Vector2Zero();
  // Player input in raylib is very simple, you just check every frame
  // if the key is pressed down. If we wanted a more complex system we could
  // wrap around it and use EnTT events to dispatch input instead
  // Today we're going simple
  if (IsKeyDown(KEY_LEFT)) {
    direction.x = -1;
  }
  if (IsKeyDown(KEY_RIGHT)) {
    direction.x = 1;
  }
  if (IsKeyDown(KEY_UP)) {
    direction.y = -1;
  }
  if (IsKeyDown(KEY_DOWN)) {
    direction.y = 1;
  }

  // This will clamp the Vector to ensure the length is always max of 1
  // This avoids the player moving at double the speed when moving diagonally
  Vector2 normalized_direction = Vector2Normalize(direction);

  for (auto [entity, vel] : view.each()) {
    vel.x = normalized_direction.x * PLAYER_MOVE_SPEED;
    vel.y = normalized_direction.y * PLAYER_MOVE_SPEED;
  }
}

void draw_boxes(entt::registry &registry) {
  auto view = registry.view<const Position, const Velocity, const Box>();

  for (auto [entity, pos, vel, box] : view.each()) {
    DrawRectangleRec((Rectangle){pos.x - box.size.x / 2, pos.y - box.size.y / 2,
                                 box.size.x, box.size.y},
                     RED);
  }
}

void update_collision(entt::registry &registry) {
  auto player_view = registry.view<const Position, Velocity, Player>();
  auto box_view = registry.view<const Position, const Velocity, const Box>();
  float delta_time = GetFrameTime();

  for (auto [player_entity, player_pos, player_vel] : player_view.each()) {
    // Instead of checking the current position we check the next position
    // I heard somewhere back in the day this was better than checking the
    // collision againt the next position the player will be at.
    // I don't remember why
    auto next_player_pos = (Vector2){
        player_pos.x + (player_vel.x * delta_time),
        player_pos.y + (player_vel.y * delta_time),
    };
    // Define hitboxes so we can use raylib instead of doing this "manually"
    auto next_player_hitbox = (Rectangle){
        next_player_pos.x - PLAYER_SIZE / 2,
        next_player_pos.y - PLAYER_SIZE / 2,
        PLAYER_SIZE,
        PLAYER_SIZE,
    };
    for (auto [box_entity, box_pos, box_vel, box_box] : box_view.each()) {
      auto box_hitbox = (Rectangle){
          box_pos.x - box_box.size.x / 2,
          box_pos.y - box_box.size.y / 2,
          box_box.size.x,
          box_box.size.y,
      };
      // This does the collision check
      bool collided = CheckCollisionRecs(next_player_hitbox, box_hitbox);
      if (collided) {
        // This returns the intersection of the collision. It's very useful
        // instead of having to calculate this ourselves
        Rectangle collision_rect =
            GetCollisionRec(next_player_hitbox, box_hitbox);
        // We can skip a lot of calculations by just checking if the player is
        // moving in the direction of the box before continuing
        // right->left collision
        if (player_vel.x > 0.f &&
            (collision_rect.x + collision_rect.width) > box_hitbox.x) {
          player_vel.x = 0;
        }
        // left->right collision
        if (player_vel.x < 0 &&
            collision_rect.x < (box_hitbox.x + box_hitbox.width)) {
          player_vel.x = 0;
        }
        // up->down collision
        if (player_vel.y > 0.f &&
            (collision_rect.y + collision_rect.height) > box_hitbox.y) {
          player_vel.y = 0;
        }
        // down->up collision
        if (player_vel.y < 0 &&
            collision_rect.y < (box_hitbox.y + box_hitbox.height)) {
          player_vel.y = 0;
        }
        registry.emplace_or_replace<CollidingWithPlayer>(box_entity,
                                                         player_entity);
      } else {
        registry.remove<CollidingWithPlayer>(box_entity);
      }
    }
  }
}

const float PLAYER_PUSH_POWER = 20.f;

// This system will handle the player clickin the E button
void read_player_push(entt::registry &registry) {
  if (IsKeyDown(KEY_E)) {
    auto view = registry.view<const Position, Velocity, const Box, const CollidingWithPlayer>();

    for (auto [entity, box_pos, box_vel, box, colliding] : view.each()) {
      auto player_pos_ptr = registry.try_get<const Position>(colliding.player_entity);
      // If the player doesn't exist, we have to remove this collider as it
      // might be bugged out
      if (player_pos_ptr == nullptr) {
        registry.remove<CollidingWithPlayer>(entity);
        continue;
      }

      Position player_pos = *player_pos_ptr;
      // Now we have to check which direction the box is in
      Vector2 direction = Vector2Zero();
      if (player_pos.x < box_pos.x) {
        direction.x = 1;
      }
      if (player_pos.x > box_pos.x) {
        direction.x = -1;
      }
      if (player_pos.y < box_pos.y) {
        direction.y = 1;
      }
      if (player_pos.y > box_pos.y) {
        direction.y = -1;
      }
      direction = Vector2Normalize(direction);

      // Push the box lol, how many comments can I write
      box_vel.x += direction.x * PLAYER_PUSH_POWER;
      box_vel.y += direction.y * PLAYER_PUSH_POWER;
    }
  }
}

//#endregion

int main() {
  // Initialize the window with a size and a name
  InitWindow(800, 450, "raylib + EnTT template - basic boxes");
  // The target fps will make it so your PC doesn't explode running this at
  // 1_000_000 fps
  SetTargetFPS(60);

  // This is all you need to setup the ECS system
  entt::registry registry;

  //#region useless
  // Let's create some objects
  // This will be our player object
  entt::entity player = registry.create();
  // This is how you can insert components into an entity
  registry.emplace<Player>(player);
  registry.emplace<Position>(player, 0.f, 0.f);
  registry.emplace<Velocity>(player, 0.f, 0.f);
  // If the component was already there and wanted to replace it
  // we can use emplace_or_replace with the new values
  // 400,200 should be around the middle of the window
  registry.emplace_or_replace<Position>(player, 400.f, 200.f);

  // This will be the movable box
  entt::entity box = registry.create();
  registry.emplace<Box>(box, (Vector2){20.f, 20.f});
  registry.emplace<Position>(box, 420.f, 200.f);
  registry.emplace<Velocity>(box, 0.f, 0.f);
  registry.emplace<Friction>(box, 0.80f);
  // We can also modify them directly by doing:
  auto &pos = registry.get<Position>(box);
  pos.x = pos.x + 10.0;
  // Let's "accidentally" add a component the box doesn't need
  registry.emplace<Player>(box);
  // We can remove it
  registry.remove<Player>(box);
  // If the component is not attached to box anyways, this function doesn't
  // throw
  registry.remove<DumbComponent>(box);
  //#endregion

  // Logic and Rendering systems should be in separate phases of the game loop
  // Logic first and rendering after so that the player always has the last
  // computed state when it is rendered
  while (!WindowShouldClose()) {
    // Here should be the update systems
    //#region useless
    read_player_input(registry);
    update_collision(registry);
    update_velocity(registry);
    update_position(registry);
    read_player_push(registry);
    //#endregion

    // BeginDrawing is for 2d. 3d is BeginMode3D(camera) which requires
    // us to setup a camera object.
    BeginDrawing();
    // Clearing the framebuffer will prevent previously drawn sprites graphics
    // from appearing on the next frame
    ClearBackground(RAYWHITE);

    // Rendering systems here
    //#region useless
    // Lets draw the controls on screen, so we don't forget!
    DrawText("Arrows to move", 190, 20, 22, LIGHTGRAY);
    DrawText("E to push the box", 190, 42, 22, LIGHTGRAY);

    draw_player(registry);
    draw_boxes(registry);
    //#endregion

    EndDrawing();
  }

  CloseWindow();
  return 0;
}
