#include "forces.h"
#include "collision.h"
#include <math.h>
#include <assert.h>

typedef struct force_aux{
    double constant;
    List *bodies;
} ForceAux;

typedef struct jump_aux{
    double max_horiz_speed;
    double jump_impulse;
    Body *player;
    int *key_data;
} JumpAux;

typedef struct shoot_aux{
    Body *enemy;
    Body *player;
    Scene *scene;
    double chance;
} ShootAux;

typedef struct collision_aux{
    List *bodies;
    CollisionHandler handler;
    Scene *scene;
    void *aux;
    bool collided_last_tick;
} CollisionAux;

void free_force_aux(ForceAux *force_aux){
    free(force_aux);
}

void free_collision_aux(CollisionAux *collision_aux){
    free(collision_aux);
}

void add_forces_gravity(ForceAux* force_aux){
    Vector center1 = body_get_centroid(list_get(force_aux->bodies, 0));
    Vector center2 = body_get_centroid(list_get(force_aux->bodies, 1));
    Vector r12 = vec_subtract(center2, center1);
    double distance = sqrt(vec_dot(r12, r12));
    if(vec_distance(center1, center2) > body_radius(list_get(force_aux->bodies, 0)) + body_radius(list_get(force_aux->bodies, 1))){
        double mass1 = body_get_mass(list_get(force_aux->bodies, 0));
        double mass2 = body_get_mass(list_get(force_aux->bodies, 1));
        double scaling_factor = force_aux->constant * mass1 * mass2 / (pow(distance, 3));
        Vector f12 = vec_multiply(scaling_factor, r12);
        Vector f21 = vec_negate(f12);

        body_add_force(list_get(force_aux->bodies, 0), f12);
        body_add_force(list_get(force_aux->bodies, 1), f21);
    }
}

void create_newtonian_gravity(Scene *scene, double G, Body *body1, Body *body2){
    ForceAux *force_aux = malloc(sizeof(ForceAux));
    force_aux->constant = G;
    List *bodies = list_init(0, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    force_aux->bodies = bodies;
    scene_add_force_creator(scene, (ForceCreator)add_forces_gravity, force_aux, (FreeFunc)free_force_aux);
}

void add_forces_spring(ForceAux *force_aux){
    Vector center1 = body_get_centroid(list_get(force_aux->bodies, 0));
    Vector center2 = body_get_centroid(list_get(force_aux->bodies, 1));
    Vector r12 = vec_subtract(center2, center1);
    Vector f12 = vec_multiply(force_aux->constant, r12);
    Vector f21 = vec_negate(f12);

    body_add_force(list_get(force_aux->bodies, 0), f12);
    body_add_force(list_get(force_aux->bodies, 1), f21);
}

void create_spring(Scene *scene, double k, Body *body1, Body *body2){
    assert(k >= 0);
    ForceAux *force_aux = malloc(sizeof(ForceAux));
    force_aux->constant = k;
    List *bodies = list_init(0, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    force_aux->bodies = bodies;
    scene_add_force_creator(scene, (ForceCreator)add_forces_spring, force_aux, (FreeFunc)free_force_aux);
}

void add_forces_drag(ForceAux *force_aux){
    Vector force = vec_multiply(-1 * force_aux->constant, body_get_velocity(list_get(force_aux->bodies, 0)));
    body_add_force(list_get(force_aux->bodies, 0), force);
}

void create_drag(Scene *scene, double gamma, Body *body){
    assert(gamma >= 0);
    ForceAux *force_aux = malloc(sizeof(ForceAux));
    force_aux->constant = gamma;
    List *bodies = list_init(0, NULL);
    list_add(bodies, body);
    force_aux->bodies = bodies;
    scene_add_force_creator(scene, (ForceCreator)add_forces_drag, force_aux, (FreeFunc)free_force_aux);
}

void add_destructive(Body *body1, Body *body2, Vector axis, void *aux, CollisionEventType type){
    if(type == COLLISION_START){
        body_remove(body1);
        body_remove(body2);
    }
}

void add_bounce(Body *body1, Body *body2, Vector axis, void *aux, CollisionEventType type){
    if(type == COLLISION_START){
        double mass_1 = body_get_mass(body1);
        double mass_2 = body_get_mass(body2);
        Vector body_1_proj = vec_project(body_get_velocity(body1), axis);
        Vector body_2_proj = vec_project(body_get_velocity(body2), axis);

        double reduced_mass = (mass_1 * mass_2)/(mass_1 + mass_2);
        if (mass_1 == INFINITY){
          reduced_mass = mass_2;
        }
        else if (mass_2 == INFINITY){
          reduced_mass = mass_1;
        }

        double impulse_coeff = reduced_mass * (1 + *(double *)aux);
        Vector impulse_on_1 = vec_multiply(impulse_coeff, vec_subtract(body_2_proj, body_1_proj));
        body_add_impulse(body1, impulse_on_1);
        body_add_impulse(body2, vec_negate(impulse_on_1));
    }
}

void collision_detector(void *aux){
    CollisionAux *collision_aux = (CollisionAux *)aux;
    Body *body1 = list_get(collision_aux->bodies, 0);
    Body *body2 = list_get(collision_aux->bodies, 1);
    double center_distance = vec_distance(body_get_centroid(body1), body_get_centroid(body2));
    if (center_distance <= body_radius(body1) + body_radius(body2)){
        List *shape1 = body_get_shape(body1);
        List *shape2 = body_get_shape(body2);
        CollisionInfo collision_info = find_collision(shape1, shape2);
        list_free(shape1);
        list_free(shape2);
        CollisionEventType type = COLLISION_NONE;
        if(collision_info.collided && !collision_aux->collided_last_tick){
            type = COLLISION_START;
        } else if (collision_info.collided && collision_aux->collided_last_tick){
            type = COLLISION_TOUCHING;
        } else if (collision_aux->collided_last_tick){
            type = COLLISION_END;
        }
        if (type != COLLISION_NONE){
          collision_aux->handler(body1, body2, collision_info.axis, collision_aux->aux, type);
        }
        collision_aux->collided_last_tick = collision_info.collided;
    }
}

void create_collision(Scene *scene, Body *body1, Body *body2, CollisionHandler handler,
  void *aux, FreeFunc freer){

    List *bodies = list_init(2, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);

    CollisionAux *collision_aux = malloc(sizeof(CollisionAux));

    collision_aux->bodies = bodies;
    collision_aux->handler = handler;
    collision_aux->aux = aux;
    collision_aux->collided_last_tick = false;
    collision_aux->scene = scene;

    scene_add_bodies_force_creator(scene, (ForceCreator)collision_detector,
      collision_aux, bodies, (FreeFunc)free_collision_aux);
}

void create_destructive_collision(Scene *scene, Body *body1, Body *body2){
    create_collision(scene, body1, body2, (CollisionHandler)add_destructive, NULL, NULL);
}

void create_physics_collision(Scene *scene, double elasticity, Body *body1, Body *body2){
    assert (elasticity >= 0);
    ForceAux *elasticity_aux = malloc(sizeof(ForceAux));
    elasticity_aux->constant = elasticity;
    List *bodies = list_init(0, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    elasticity_aux->bodies = bodies;
    create_collision(scene, body1, body2, (CollisionHandler)add_bounce, elasticity_aux, (FreeFunc)free_force_aux);
}

void apply_powerup(Body *player, Body *powerup, Vector axis, void *aux){
    BodyInfo *type = (BodyInfo *)body_get_info(powerup);
    BodyInfo *info = (BodyInfo *)body_get_info(player);
    Scene *scene = (Scene *)aux;
    switch(type->type){
        case BULLET_POWERUP:
            info->MAX_BULLETS = info->MAX_BULLETS + 1;
            break;
        case FINISHED_LEVEL_POWERUP:
            scene_set_finished_level(scene);
            break;
        default:
            break;
  }
  body_remove(powerup);
}

void create_powerup_collision(Scene *scene, Body *player, Body *powerup){
    assert(player != NULL);
    assert(body_get_info(powerup) != NULL);
    create_collision(scene, player, powerup, (CollisionHandler)apply_powerup, scene, NULL);
}

void add_platform_gravity(ForceAux *aux){
    Body *body1 = list_get(aux->bodies, 0);
    Body *body2 = list_get(aux->bodies, 1);
    BODY_MOVEMENT movement1 = ((BodyInfo*)body_get_info(body1))->movement;
    BODY_MOVEMENT movement2 = ((BodyInfo*)body_get_info(body2))->movement;
    if(movement1 == FALLING || movement2 == FALLING){
        add_forces_gravity(aux);
    }
}

void create_platform_gravity(Scene *scene, double G,
   Body *body1, Body *body2){
    ForceAux *force_aux = malloc(sizeof(ForceAux));
    force_aux->constant = G;
    List *bodies = list_init(0, NULL);
    list_add(bodies, body1);
    list_add(bodies, body2);
    force_aux->bodies = bodies;
    scene_add_force_creator(scene, (ForceCreator)add_platform_gravity, force_aux, (FreeFunc)free_force_aux);
}

void add_friction(ForceAux *aux){
    Body *body = list_get(aux->bodies, 0);
    Vector velocity = body_get_velocity(body);
    velocity.y = 0;
    Vector force = vec_multiply(-1 * aux->constant, velocity);
    body_add_force(body, force);
}

void create_friction(Scene *scene, double gamma, Body *body){
    ForceAux *force_aux = malloc(sizeof(ForceAux));
    force_aux->constant = gamma;
    List *bodies = list_init(0, NULL);
    list_add(bodies, body);
    force_aux->bodies = bodies;
    scene_add_force_creator(scene, (ForceCreator)add_friction, force_aux, (FreeFunc)free_force_aux);
}


void add_enemy_bullet(ShootAux *aux){
    double chance = rand() / (float)RAND_MAX;
    if(chance < aux->chance){
        Scene *scene = aux->scene;
        Body *enemy = aux->enemy;
        Body *player = aux->player;
        Body *bullet = gen_bullet(100, scene, ENEMY_BULLET);

        body_set_centroid(bullet, body_get_centroid(enemy));
        Vector dir = vec_subtract(body_get_centroid(player), body_get_centroid(bullet));
        double angle = atan2(dir.y, dir.x);
        body_set_rotation(bullet, angle);
        body_set_velocity(bullet, vec_rotate(body_get_velocity(bullet), angle));

        add_bullet_forces(scene, bullet);
    }
}

void create_enemy_bullet(Scene *scene, double chance, Body *enemy, Body *player){
    ShootAux *shoot_aux = malloc(sizeof(ShootAux));
    shoot_aux->scene = scene;
    shoot_aux->enemy = enemy;
    shoot_aux->chance = chance;
    shoot_aux->player = player;
    List *bodies = list_init(0, NULL);
    list_add(bodies, enemy);

    scene_add_bodies_force_creator(scene, (ForceCreator)add_enemy_bullet, shoot_aux, bodies, free);
}

void add_player_movement(JumpAux *aux){
    Body *player = aux->player;
    int *key_data = aux->key_data;
    Vector speed = body_get_velocity(player);
    double mass = body_get_mass(player);
    BodyInfo *info = (BodyInfo*)body_get_info(player);
    int jump_count = info->num;

    double player_max_speed = aux->max_horiz_speed;
    double jump_impulse = aux->jump_impulse;

    if(*(key_data + LEFT_ARROW) == KEY_PRESSED && info->touch != TOUCHING_LEFT){
        body_add_impulse(player, (Vector){(-player_max_speed - speed.x)*mass, 0});
        info->touch = TOUCHING_NONE;
    }

    else if(*(key_data + RIGHT_ARROW) == KEY_PRESSED && info->touch != TOUCHING_RIGHT){
        body_add_impulse(player, (Vector){(player_max_speed - speed.x)*mass, 0});
        info->touch = TOUCHING_NONE;
    }

    if(*(key_data + UP_ARROW) == KEY_PRESSED){
        if(jump_count < 10){
            body_add_impulse(player, (Vector){0, jump_impulse/10});
            info->num++;
        }
    }
}

void create_player_movement(Scene *scene, double max_speed, double jump_impulse, Body *player){
    JumpAux *aux = malloc(sizeof(JumpAux));
    aux->max_horiz_speed = max_speed;
    aux->jump_impulse = jump_impulse;
    aux->player = player;
    aux->key_data = scene_key_data(scene);
    List *bodies = list_init(0, NULL);
    list_add(bodies, player);
    scene_add_bodies_force_creator(scene, (ForceCreator)add_player_movement, aux, bodies, free);
}
