#include "gen_forces.h"
#include "helpers.h"

const double GRAV = 0.00009;
const double PLAYER_JUMP_IMPULSE = 7.5E4;
const double PLAYER_MAX_SPEED = 650;
const double BULLET_DAMAGE = .1;
const double SHOOT_CHANCE = .0008;

void add_platform_collision(Body *player, Body *platform, Vector axis, void *aux, CollisionEventType type){
    BodyInfo *info = (BodyInfo*)body_get_info(player);
    if(type == COLLISION_TOUCHING){
        if(get_body_type(platform) == MOVING_FLOOR){
    		Vector velocity_shift = vec_subtract(body_get_velocity(platform), body_get_velocity(player));
            body_add_impulse(player, vec_multiply(body_get_mass(player), velocity_shift));
        }
        //body_add_impulse(player, vec_multiply(body_get_mass(player), vec_multiply(-2, axis)));
        Vector centroid = body_get_centroid(player);
        //Vector shift = vec_multiply(-0.5, axis);
        Vector shift = (Vector){-2*axis.x, -0.5*axis.y};
        body_set_centroid(player, vec_add(centroid, shift));
        info->movement = TOUCHING_FLOOR;
        if(fabs(axis.x) > 0.5){
            if(axis.x > 0){
                info->touch = TOUCHING_RIGHT;
            } else {
                info->touch = TOUCHING_LEFT;
            }
        }
    } else if(type == COLLISION_START){
        if(axis.y < 0){
            double elasticity = 0.0;
    		add_bounce(player, platform, axis, &elasticity, COLLISION_START);
    		info->movement = TOUCHING_FLOOR;
            info->num = 0;
        }
        if(fabs(axis.x) > 0.5){
            if(info->type == PLAYER){
                double elasticity = 0.0;
                add_bounce(player, platform, axis, &elasticity, COLLISION_START);
                if(axis.x > 0){
                    info->touch = TOUCHING_RIGHT;
                } else {
                    info->touch = TOUCHING_LEFT;
                }
            }
        }
	} else if(type == COLLISION_END){
        if(info->type == PLAYER){
            info->movement = FALLING;
            if(fabs(axis.x) > 0.5){
                info->touch = TOUCHING_NONE;
            }
        }
        if(info->type == ENEMY){
            body_set_velocity(player, vec_negate(body_get_velocity(player)));
        }
    }
}

void add_enemy_collision(Body *player, Body *enemy, Vector axis, void *aux){
    Scene *scene = (Scene *)aux;
    scene_set_done(scene, true);
}

void create_enemy_collision(Scene *scene, Body *player, Body *enemy){
    create_collision(scene, player, enemy, (CollisionHandler)add_enemy_collision, scene, NULL);
}

void add_bullet_collision(Body *bullet, Body *enemy, Vector axis, void *aux){
    body_remove(enemy);
    body_remove(bullet);
}

void create_bullet_collision(Scene *scene, Body *bullet, Body *enemy){
    create_collision(scene, bullet, enemy, (CollisionHandler)add_bullet_collision, scene, NULL);
}

void add_boss_collision(Body *bullet, Body *boss, Vector axis, void *aux){
    BodyInfo *boss_info = body_get_info(boss);

    body_remove(bullet);
    boss_info->num = boss_info->num - BULLET_DAMAGE;
    if(boss_info->num < -1){
        body_remove(boss);
    }
}

void create_boss_collision(Scene *scene, Body *bullet, Body *enemy){
    create_collision(scene, bullet, enemy, (CollisionHandler)add_boss_collision, scene, NULL);
}

void add_spike_collision(Body *spike, Body *boss, Vector axis, void *aux){
    body_set_velocity(boss, vec_negate(body_get_velocity(boss)));
}

void create_spike_collision(Scene *scene, Body *spike, Body *enemy){
    create_collision(scene, spike, enemy, (CollisionHandler)add_spike_collision, scene, NULL);
}

void create_platform_collision(Scene *scene, Body *player, Body *platform){
	create_collision(scene, player, platform, (CollisionHandler)add_platform_collision, scene, NULL);
}

void add_player_forces(Scene *scene, Body *player){
	for(int i = 0; i < scene_bodies(scene); i++){
		Body *body = scene_get_body(scene, i);
		if(get_body_type(body) == FLOOR || get_body_type(body) == MOVING_FLOOR){
			create_platform_collision(scene, player, body);
		}

		if(get_body_type(body) == GRAVITY_BODY){
			create_platform_gravity(scene, GRAV, player, body);
		}
        if(get_body_type(body) == SPIKE){
            create_enemy_collision(scene, player, body);
        }
	}
	create_friction(scene, 1000.0, player);
    create_player_movement(scene, PLAYER_MAX_SPEED, PLAYER_JUMP_IMPULSE, player);
}

void add_enemy_forces(Scene *scene, Body *enemy, Body *player){
    for(int i = 0; i < scene_bodies(scene); i++){
		Body *body = scene_get_body(scene, i);
		if(get_body_type(body) == FLOOR || get_body_type(body) == MOVING_FLOOR){
			create_platform_collision(scene, enemy, body);
		}
        if(get_body_type(body) == PLAYER){
            create_enemy_collision(scene, enemy, body);
        }
        if(get_body_type(body) == GRAVITY_BODY){
            create_platform_gravity(scene, GRAV, enemy, body);
        }
	}
    create_enemy_bullet(scene, SHOOT_CHANCE, enemy, player);
}

void add_boss_forces(Scene *scene, Body *enemy){
    Body *player = get_first_body(scene, PLAYER);
    for(int i = 0; i < scene_bodies(scene); i++){
        Body *body = scene_get_body(scene, i);
        if(get_body_type(body) == FLOOR){
            create_platform_collision(scene, enemy, body);
        }
        if(get_body_type(body) == PLAYER){
            create_enemy_collision(scene, enemy, body);
        }
        if(get_body_type(body) == GRAVITY_BODY){
            create_platform_gravity(scene, GRAV, enemy, body);
        }
        if(get_body_type(body) == SPIKE){
            create_spike_collision(scene, enemy, body);
        }
    }
    create_enemy_bullet(scene, SHOOT_CHANCE * 2, enemy, player);
}

void add_bullet_forces(Scene *scene, Body *bullet){
    if(get_body_type(bullet) == BULLET){
        for(int i = 0; i < scene_bodies(scene); i++){
    		Body *body = scene_get_body(scene, i);
            if(get_body_type(body) == ENEMY){
                create_bullet_collision(scene, bullet, body);
            }
            if(get_body_type(body) == BOSS){
                create_boss_collision(scene, bullet, body);
            }
    	}
    }
    if(get_body_type(bullet) == ENEMY_BULLET){
        Body *player = get_first_body(scene, PLAYER);
        create_enemy_collision(scene, player, bullet);
    }
}

void gen_forces(Scene *scene){
    Body *player = get_first_body(scene, PLAYER);
	add_player_forces(scene, player);
    for(int i = 0; i < scene_bodies(scene); i++){
        Body *curr = scene_get_body(scene, i);
        if(get_body_type(curr) == ENEMY){
            add_enemy_forces(scene, curr, player);
        }
    }
}
