#include "gen_levels.h"
#include "forces.h"
#include "shape.h"
#include "enemies.h"
#include "sdl_wrapper.h"
#include "gen_forces.h"
#include <math.h>

const RGBColor BLACK = (RGBColor){0,0,0};
const RGBColor BLUE = (RGBColor){0,0,1};
const RGBColor GREEN = (RGBColor){0,1,0};
const RGBColor RED = (RGBColor){1, 0, 0};
const double PLAYER_MASS = 50;
//const double S = 500; //this is the K value
const double S = 300E80;
const double M = 60E80;
//const double M = 60;
const double GRAVITY_MASS = 6E20;
const double GRAVITY_Y_OFFSET = -5E6;
const double TRIANGLE_RADIUS = 50;
const double BULL_SPEED = 1500;

void body_info_free(BodyInfo *body_info){
    free(body_info);
}
BODY_TYPE get_body_type(Body *powerup){
    return ((BodyInfo *)body_get_info(powerup))->type;
}

BodyInfo *create_body_info(BODY_TYPE type, BODY_MOVEMENT movement){
    BodyInfo *info = malloc(sizeof(BodyInfo));
    info->type = type;
    info->movement = movement;
    info->touch = TOUCHING_NONE;
    info->num = 1;
    info->bullet_count = 0;
    info->bullet_time_shot = 0.0;
    info->MAX_BULLETS = 3;
    return info;
}

Body* add_powerup(Scene *scene, Body *player, double radius, double mass,
  Vector centroid, RGBColor color, BODY_TYPE powerup_type){
    BodyInfo *info = create_body_info(powerup_type, NONE);
    Body *powerup = body_init_with_info(shape_estrella(radius), mass, color, info, (FreeFunc)body_info_free);
    body_set_centroid(powerup, centroid);
    scene_add_body(scene, powerup);
    create_powerup_collision(scene, player, powerup);
    return powerup;
}

Body* add_rectangle_floor(Scene* scene, double width, double height, double mass, Vector centroid,
  RGBColor color, BODY_TYPE type){
    BodyInfo *info = create_body_info(type, NONE);
    Body* floor = body_init_with_info(shape_rectangle(width, height), mass, color, info, (FreeFunc)body_info_free);
    body_set_centroid(floor, centroid);
    scene_add_body(scene, floor);
    return floor;
}

void add_gravity_body(Scene *scene){
    BodyInfo *info = create_body_info(GRAVITY_BODY, NONE);
    Body *gravity = body_init_with_info(shape_rectangle(1, 1), GRAVITY_MASS, BLACK, info, (FreeFunc)body_info_free);
    body_set_centroid(gravity, (Vector){0, GRAVITY_Y_OFFSET});
    scene_add_body(scene, gravity);
}

// Floor that moves with spring constant k. Displacement specifies how far from
// center the floor moves (amplitude).
Body* add_moving_platform(Scene* scene, double width, double height, double mass, double k,
  Vector centroid, Vector displacement, RGBColor color){
    Body *floor = add_rectangle_floor(scene, width, height, mass, centroid, color, MOVING_FLOOR);
    Body *anchor = body_init(shape_rectangle(0.5, 0.5), mass, color);
    body_set_centroid(anchor, centroid);
    body_set_centroid(floor, vec_add(centroid, displacement));
    create_spring(scene, k, floor, anchor);
    return floor;
}

Body* add_spike(Scene *scene, double radius, Vector spawn_point){
    BodyInfo *info = create_body_info(SPIKE, NONE);
    List *shape = shape_triangle(radius);
    Body *spike = body_init_with_info(shape, M, RED, info, (FreeFunc)body_info_free);
    body_set_centroid(spike, spawn_point);
    scene_add_body(scene, spike);
    return spike;
}

Body *add_upsidedown_spike(Scene *scene, double radius, Vector spawn_point){
    Body *spike = add_spike(scene, radius, spawn_point);
    body_set_rotation(spike, M_PI);
    return spike;
}

void add_spike_row(Scene *scene, double tri_rad, Vector init_spawn, int len_row){
    for(int i = 0; i < len_row; i++){
        add_spike(scene, tri_rad, (Vector){init_spawn.x + i*tri_rad, init_spawn.y+10});
    }
}

void add_upsidedown_spike_row(Scene *scene, double tri_rad, Vector init_spawn, int len_row){
    for(int i = 0; i < len_row; i++){
        add_upsidedown_spike(scene, tri_rad, (Vector){init_spawn.x + i * tri_rad, init_spawn.y+10});
    }
}

void add_moving_upsidedown_spike(Scene *scene, double radius, Vector spawn_point, Vector displacement, double k){
  Body *spike = add_upsidedown_spike(scene, radius, spawn_point);
  Body *anchor = body_init(shape_rectangle(0.5, 0.5), M, RED);
  body_set_centroid(anchor, spawn_point);
  body_set_centroid(spike, vec_add(spawn_point, displacement));
  create_spring(scene, k, spike, anchor);
}

// adds a body between two floors body1 and body2 by connecting the necessary points.
// body1 is on the left, and body2 is on the right.
Body* add_sloped_floor(Scene *scene, Body* body1, Body* body2, double mass, RGBColor color){
  List *floor_points = list_init(4, free);
  List *shape1 = body_get_shape(body1);
  List *shape2 = body_get_shape(body2);

  list_add(floor_points, list_get(shape2, 1));
  list_add(floor_points, list_get(shape1, 0));
  list_add(floor_points, list_get(shape1, 3));
  list_add(floor_points, list_get(shape2, 2));

  BodyInfo *info = create_body_info(FLOOR, NONE);
  Body *sloped_floor = body_init_with_info(floor_points, mass, color,
    info, (FreeFunc)body_info_free);
  scene_add_body(scene, sloped_floor);
  return sloped_floor;
}

void add_stairs(Scene *scene, Body* body1, Body* body2, double mass, double player_size,
   RGBColor color){
     List *shape1 = body_get_shape(body1);
     List *shape2 = body_get_shape(body2);

     // Points labeled 0, 1, 2, 3 counterclockwise from top right.
     Vector *shape1_pt3 = malloc(sizeof(Vector));
     *shape1_pt3 = *(Vector *)list_get(shape1, 3);
     Vector *shape1_pt0 = malloc(sizeof(Vector));
     *shape1_pt0 = *(Vector *)list_get(shape1, 0);
     Vector *shape2_pt1 = malloc(sizeof(Vector));
     *shape2_pt1 = *(Vector *)list_get(shape2, 1);

    int num_stairs = (int) (shape2_pt1->y - shape1_pt0->y)/player_size;
    double stair_width = (shape2_pt1->x - shape1_pt0->x)/num_stairs;

    for (int i = 0; i < num_stairs; i++){
      List *stair_points = list_init(4, free);
      Vector add_0 = {(i + 1) * stair_width, (i + 1) * player_size};
      Vector add_1 = {i * stair_width, (i + 1) * player_size};
      Vector add_2 = {i * stair_width, 0};
      Vector add_3 = {(i + 1) * stair_width, 0};

      Vector *stair_pt0 = malloc(sizeof(Vector));
      Vector *stair_pt1 = malloc(sizeof(Vector));
      Vector *stair_pt2 = malloc(sizeof(Vector));
      Vector *stair_pt3 = malloc(sizeof(Vector));

      *stair_pt0 = vec_add(*shape1_pt0, add_0);
      *stair_pt1 = vec_add(*shape1_pt0, add_1);
      *stair_pt2 = vec_add(*shape1_pt3, add_2);
      *stair_pt3 = vec_add(*shape1_pt3, add_3);

      list_add(stair_points, stair_pt0);
      list_add(stair_points, stair_pt1);
      list_add(stair_points, stair_pt2);
      list_add(stair_points, stair_pt3);

      double j = (double) i;
      RGBColor color1 = (RGBColor) {1/(j+2), 1/(j+43), 1/(j+16)};

      BodyInfo *info = create_body_info(FLOOR, NONE);
      Body *stair = body_init_with_info(stair_points, mass, color1,
        info, (FreeFunc)body_info_free);
      scene_add_body(scene, stair);
   }
}

void one_by_two_floors(Scene* scene, double mass, RGBColor color, List* centroids,
  double player_size){
    for (size_t i = 0; i < list_size(centroids); i++){
      add_rectangle_floor(scene, 2 * player_size, player_size, mass,
        *(Vector *)list_get(centroids, i), color, FLOOR);
    }
}

Body *gen_player_sq(double player_size, Scene *scene){
    List *shape = shape_rectangle(player_size, player_size);
    BodyInfo *info = create_body_info(PLAYER, FALLING);
    Body *rect = body_init_with_info(shape, PLAYER_MASS, GREEN, info, (FreeFunc)body_info_free);
    body_set_centroid(rect, (Vector){player_size*2, player_size*2});
    scene_add_body(scene, rect);
    return rect;
}

Body *gen_bullet(double player_size, Scene *scene, BODY_TYPE btype){
    List *shape = shape_estrella(player_size/5);
    BodyInfo *info = create_body_info(btype, NONE);
    Body *star = body_init_with_info(shape, 20, BLUE, info, (FreeFunc)body_info_free);
    Body *player = get_first_body(scene, PLAYER);
    body_set_centroid(star, body_get_centroid(player));
    body_set_velocity(star, (Vector){BULL_SPEED, 0});
    scene_add_body(scene, star);
    return star;
}

void gen_tutorial_level(double player_size, Scene *scene, Body *player){
    add_gravity_body(scene);
    add_rectangle_floor(scene, 70 * player_size, player_size,
      INFINITY, (Vector){35 * player_size, 0}, BLACK, FLOOR);
    add_rectangle_floor(scene, 2 * player_size, 3 * player_size,
      INFINITY, (Vector){10 * player_size, 1.5 * player_size}, BLACK, FLOOR);
    add_spike_row(scene, TRIANGLE_RADIUS, (Vector){22.5 * player_size, 1.3 * TRIANGLE_RADIUS}, 2);
    gen_enemy(70, 50, scene, (Vector){28 * player_size, 3.5 * player_size});
    add_powerup(scene, player, 60, INFINITY, (Vector){45.75 * player_size, 1.5 * player_size}, BLUE, BULLET_POWERUP);
    add_rectangle_floor(scene, 2 * player_size, 3 * player_size,
      INFINITY, (Vector){55 * player_size, 1.5 * player_size}, BLACK, FLOOR);
    add_powerup(scene, player, 60, INFINITY, (Vector){70 * player_size, 1.5 * player_size}, GREEN, FINISHED_LEVEL_POWERUP);
}

void gen_first_level(double player_size, Scene *scene, Body *player){

    add_gravity_body(scene);

    List *one_by_two_centroids = list_init(6, free);
    Vector *v = malloc(sizeof(Vector));
    *v = (Vector){17 * player_size, 5.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){20 * player_size, 7.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){23 * player_size, 9.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){52 * player_size, 1.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){55 * player_size, 2.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){58 * player_size, 3.5 * player_size};
    list_add(one_by_two_centroids, v);
    one_by_two_floors(scene, INFINITY, BLACK, one_by_two_centroids, player_size);

    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){4 * player_size, 0}, BLACK, FLOOR);
    add_moving_platform(scene, 2 * player_size, player_size, M, S,
      (Vector){11 * player_size, 0}, (Vector){1.5 * player_size, 0}, BLACK);
    add_moving_platform(scene, 2 * player_size, player_size/2,  M, S,
      (Vector){14.5 * player_size, 4 * player_size}, (Vector){0, 2 * player_size}, BLACK);
    add_rectangle_floor(scene, 6 * player_size, player_size,
      INFINITY, (Vector){28 * player_size, 11.5 * player_size}, BLACK, FLOOR);
    add_powerup(scene, player, 0.5 * player_size, INFINITY,
      (Vector){28 * player_size, 13 * player_size}, BLUE, (BODY_TYPE) BULLET_POWERUP);
    Body *body8 = add_rectangle_floor(scene, 9 * player_size, player_size,
      INFINITY, (Vector){18.5 * player_size, 0}, BLACK, FLOOR);
    Body *body10 = add_rectangle_floor(scene, 3 * player_size, 3 * player_size,
      INFINITY, (Vector){29.5 * player_size, 1 * player_size}, BLACK, FLOOR);
    add_stairs(scene, body8, body10, INFINITY, player_size, BLACK);
    Body *body12 = add_rectangle_floor(scene, 4 * player_size, 6 * player_size,
      INFINITY, (Vector){37 * player_size, 2.5 * player_size}, BLACK, FLOOR);
    add_stairs(scene, body10, body12, INFINITY, player_size, BLACK);
    add_moving_platform(scene, 2 * player_size, player_size, M, S,
      (Vector){42 * player_size, 2 * player_size}, (Vector){-player_size, player_size}, BLACK);
    add_rectangle_floor(scene, 5 * player_size, player_size,
      INFINITY, (Vector){47.5 * player_size, 0}, BLACK, FLOOR);
    add_spike_row(scene, TRIANGLE_RADIUS, (Vector){47 *player_size, 1.3*TRIANGLE_RADIUS}, 2);
    add_rectangle_floor(scene, 24 * player_size, 4 * player_size,
      INFINITY, (Vector){72 * player_size, 1.5 * player_size}, BLACK, FLOOR);
    gen_enemy(70, 50, scene, (Vector){72 * player_size, 4 * player_size});
    add_moving_platform(scene, 9 * player_size, 1 * player_size, M, S,
      (Vector){94 * player_size, 8 * player_size}, (Vector){3 * player_size, 3 * player_size}, BLACK);
    add_rectangle_floor(scene, 20 * player_size, 2 * player_size,
      INFINITY, (Vector){114 * player_size, 11.5 * player_size}, BLACK, FLOOR);
    add_powerup(scene, player, 60, INFINITY, (Vector){120 * player_size, 13.5 * player_size},
      GREEN, FINISHED_LEVEL_POWERUP);
    gen_enemy(70, 50, scene, (Vector){2820, 300});
    gen_enemy(70, 50, scene, (Vector){7157, 420});

}

void gen_second_level(double player_size, Scene *scene, Body *player){
  add_gravity_body(scene);

  List *one_by_two_centroids = list_init(6, free);
  Vector *v = malloc(sizeof(Vector));
  *v = (Vector){27 * player_size, 6.25 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){24 * player_size, 7.25 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){21 * player_size, 8.25 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){21 * player_size, 11.75 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){35 * player_size, 5.5 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){43 * player_size, 5.5 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){51 * player_size, 5.5 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){54 * player_size, 12.5 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){57 * player_size, 10.5 * player_size};
  list_add(one_by_two_centroids, v);
  v = malloc(sizeof(Vector));
  *v = (Vector){60 * player_size, 5.5 * player_size};
  list_add(one_by_two_centroids, v);
  one_by_two_floors(scene, INFINITY, BLACK, one_by_two_centroids, player_size);

  add_moving_platform(scene, 2 * player_size, player_size,  M, S,
    (Vector){18 * player_size, 10 * player_size}, (Vector){2 * player_size, 0}, BLACK);

  add_spike_row(scene, TRIANGLE_RADIUS, (Vector){11 *player_size, 4.35 * TRIANGLE_RADIUS}, 2);
  add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){15.5 *player_size, 3.45 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S);
  add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){16.5 *player_size, 3.45 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S);

  gen_enemy(70, 50, scene, (Vector){3110, 520});
  gen_enemy(70, 50, scene, (Vector){2691, 1420});
  gen_enemy(70, 50, scene, (Vector){3667, 1420});
  gen_enemy(70, 50, scene, (Vector){4723, 1420});
  gen_enemy(70, 50, scene, (Vector){74.5 * player_size, 7.5 * player_size});

  add_rectangle_floor(scene, 8 * player_size, player_size,
    INFINITY, (Vector){4 * player_size, 0}, BLACK, FLOOR);
  add_rectangle_floor(scene, 4 * player_size, player_size,
    INFINITY, (Vector){11 * player_size, 1.5 * player_size}, BLACK, FLOOR);
  Body *left_side_stair = add_rectangle_floor(scene, 6 * player_size, player_size,
    INFINITY, (Vector){17 * player_size, 0 * player_size}, BLACK, FLOOR);
  Body *right_side_stair = add_rectangle_floor(scene, 8 * player_size, 5 * player_size,
    INFINITY, (Vector){29 * player_size, 2 * player_size}, BLACK, FLOOR);
  add_stairs(scene, left_side_stair, right_side_stair, INFINITY, player_size, BLACK);
  add_rectangle_floor(scene, 8 * player_size, player_size,
    INFINITY, (Vector){27 * player_size, 13 * player_size}, BLACK, FLOOR);
  add_rectangle_floor(scene, 8 * player_size, player_size,
    INFINITY, (Vector){37 * player_size, 13 * player_size}, BLACK, FLOOR);
  add_rectangle_floor(scene, 8 * player_size, player_size,
    INFINITY, (Vector){47 * player_size, 13 * player_size}, BLACK, FLOOR);
  add_rectangle_floor(scene, 24 * player_size, player_size,
    INFINITY, (Vector){75 * player_size, 5.5 * player_size}, BLACK, FLOOR);
  add_rectangle_floor(scene, 8 * player_size, player_size,
    INFINITY, (Vector){92 * player_size, 5.5 * player_size}, BLACK, FLOOR);
  add_rectangle_floor(scene, 1 * player_size, 2 * player_size,
    INFINITY, (Vector){70.5 * player_size, 7 * player_size}, BLACK, FLOOR);
  add_rectangle_floor(scene, 1 * player_size, 2 * player_size,
    INFINITY, (Vector){77.5 * player_size, 7 * player_size}, BLACK, FLOOR);
  add_powerup(scene, player, 60, INFINITY, (Vector){96 * player_size, 7.5 * player_size},
    GREEN, FINISHED_LEVEL_POWERUP);
}

void gen_third_level(double player_size, Scene *scene, Body *player){

    add_gravity_body(scene);

    List *one_by_two_centroids = list_init(6, free);
    Vector *v = malloc(sizeof(Vector));
    *v = (Vector){11 * player_size, 0.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){14 * player_size, 1.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){17 * player_size, 2.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){20 * player_size, 3.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){23 * player_size, 4.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){35 * player_size, 4.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){38 * player_size, 3.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){41 * player_size, 2.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){44 * player_size, 1.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){47 * player_size, 0.5 * player_size};
    list_add(one_by_two_centroids, v);
    v = malloc(sizeof(Vector));
    *v = (Vector){81 * player_size, 8.5 * player_size};
    list_add(one_by_two_centroids, v);
    one_by_two_floors(scene, INFINITY, BLACK, one_by_two_centroids, player_size);

    gen_enemy(70, 50, scene, (Vector){2833, 665});
    gen_enemy(70, 50, scene, (Vector){5432, 120});
    gen_enemy(70, 50, scene, (Vector){6605, 1470});
    gen_enemy(70, 50, scene, (Vector){9444, 1600});


    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){4 * player_size, 0}, BLACK, FLOOR);

    add_upsidedown_spike_row(scene, TRIANGLE_RADIUS, (Vector){4 *player_size, 5 * TRIANGLE_RADIUS}, 5);

    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){29 * player_size, 5.5 * player_size}, BLACK, FLOOR);
    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){53 * player_size, 0}, BLACK, FLOOR);

    add_moving_platform(scene, 2 * player_size, player_size,  M, S,
      (Vector){60 * player_size, 7.5 * player_size}, (Vector){0, 5 * player_size}, BLACK);

    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){67 * player_size, 8.5 * player_size}, BLACK, FLOOR);
    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){67 * player_size, 13.5 * player_size}, BLACK, FLOOR);

    add_powerup(scene, player, 0.5 * player_size, INFINITY,
      (Vector){66.5 * player_size, 10.5 * player_size}, BLUE, (BODY_TYPE) BULLET_POWERUP);

    add_moving_platform(scene, 2 * player_size, player_size,  M, S,
      (Vector){77 * player_size, 13.5 * player_size}, (Vector){3 * player_size, 0}, BLACK);

    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){87 * player_size, 13.5 * player_size}, BLACK, FLOOR);

    add_moving_platform(scene, 8 * player_size, player_size,  M, S,
      (Vector){96 * player_size, 8.5 * player_size}, (Vector){0, 3 * player_size}, BLACK);

    add_upsidedown_spike_row(scene, TRIANGLE_RADIUS, (Vector){98.5 * player_size, 28 * TRIANGLE_RADIUS}, 5);

    add_rectangle_floor(scene, 8 * player_size, player_size,
      INFINITY, (Vector){105 * player_size, 9.5 * player_size}, BLACK, FLOOR);
    add_powerup(scene, player, 60, INFINITY, (Vector){109 * player_size, 11 * player_size},
      GREEN, FINISHED_LEVEL_POWERUP);
}


void gen_boss_level(double player_size, Scene *scene, Body *player){
    add_gravity_body(scene);
    add_rectangle_floor(scene, 40 * player_size, player_size, INFINITY, (Vector){10 * player_size, 0}, BLACK, FLOOR);

    add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){3 * player_size, 4 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S);
    add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){4 * player_size, 4 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S * 1.5);
    add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){5 * player_size, 4 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S);

    add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){-3 * player_size, 4 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S);
    add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){-2 * player_size, 4 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S * 1.5);
    add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){-1 * player_size, 4 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S);
    add_moving_upsidedown_spike(scene, TRIANGLE_RADIUS, (Vector){0 * player_size, 4 * TRIANGLE_RADIUS}, (Vector){0, player_size}, S * 1.5);
    add_spike_row(scene, TRIANGLE_RADIUS, (Vector){8 * player_size, 1.3 * TRIANGLE_RADIUS}, 5);

    add_powerup(scene, player, 60, INFINITY, (Vector){-5 * player_size, 1.5 * player_size}, BLUE, BULLET_POWERUP);
    add_powerup(scene, player, 60, INFINITY, (Vector){6 * player_size, 1.5 * player_size}, BLUE, BULLET_POWERUP);

    add_boss_forces(scene, gen_boss(700, scene, (Vector){18*player_size, 4 * player_size}));
    add_powerup(scene, player, 60, INFINITY, (Vector){18 * player_size, 4 * player_size},
      GREEN, FINISHED_LEVEL_POWERUP);
}
