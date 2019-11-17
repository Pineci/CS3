#include "scene.h"
#include "sdl_wrapper.h"

struct scene{
  List *bodies;
  List *force_handlers;
  int key_presses[5];
  bool finished_level;
  bool done;
  Vector camera;
  Vector camera_velocity;
  CameraFollower follower;
  void *follower_aux;
  FreeFunc follower_freer;
  double total_time;
  bool finished_title_screen;
};

struct force_handler{
    ForceCreator force;
    void* aux;
    List *bodies;
    FreeFunc freer;
};

void force_handler_free(ForceHandler *fh){
    if(fh->freer != NULL){
        fh->freer(fh->aux);
    }
    list_free(fh->bodies);
    free(fh);
}

void *get_aux(ForceHandler *fh){
  return fh->aux;
}

List *get_fh_bodies(ForceHandler *fh){
  return fh->bodies;
}

Scene *scene_init(void){
    Scene *scene = malloc(sizeof(Scene));
    scene->bodies = list_init(0, (FreeFunc) body_free);
    scene->force_handlers = list_init(0, (FreeFunc) force_handler_free);
    scene->camera = VEC_ZERO;
    scene->camera_velocity = VEC_ZERO;
    scene->follower = NULL;
    scene->follower_aux = NULL;
    scene->follower_freer = NULL;
    scene->finished_level = false;
    //scene->jump_count = 0;
    for(int i = 0; i < 4; i++){
        scene->key_presses[i] = KEY_RELEASED;
    }
    scene->total_time = 0;
    return scene;
}

void scene_free(Scene *scene){
    list_free(scene->force_handlers);
    list_free(scene->bodies);
    if(scene->follower_freer != NULL){
        scene->follower_freer(scene->follower_aux);
    }
    free(scene);
}

void scene_set_done(Scene *scene, bool done){
    scene->done = done;
}

/*void scene_set_jump_count(Scene *scene, int jump_count){
    scene->jump_count = jump_count;
}*/

/*void scene_add_one_jump(Scene *scene){
    int jc = scene->jump_count;
    scene_set_jump_count(scene, jc+1);
}*/

/*int scene_jump_count(Scene *scene){
    return scene->jump_count;
}*/

bool scene_is_done(Scene *scene){
    return scene->done;
}

void scene_set_finished_level(Scene *scene){
    scene->finished_level = true;
}

bool scene_check_finished_level(Scene *scene){
    return scene->finished_level;
}

int* scene_key_data(Scene *scene){
    return scene->key_presses;
}

size_t scene_bodies(Scene *scene){
    return list_size(scene->bodies);
}

Body *scene_get_body(Scene *scene, size_t index){
    return list_get(scene->bodies, index);
}

void scene_add_body(Scene *scene, Body *body){
    list_add(scene->bodies, body);
}

void scene_remove_body(Scene *scene, size_t index){
    body_remove(scene_get_body(scene, index));
}

bool scene_get_finished_title_screen(Scene *scene){
    return scene->finished_title_screen;
}

void scene_set_finished_title_screen(Scene *scene, bool setting){
    scene->finished_title_screen = setting;
}

double scene_get_time(Scene *scene){
    return scene->total_time;
}

void scene_add_bodies_force_creator(Scene *scene, ForceCreator forcer, void *aux,
  List *bodies, FreeFunc freer){
    ForceHandler *fh = malloc(sizeof(ForceHandler));
    fh->force = forcer;
    fh->aux = aux;
    fh->bodies = bodies;
    fh->freer = freer;
    list_add(scene->force_handlers, fh);
}

void scene_add_force_creator(Scene *scene, ForceCreator forcer, void *aux, FreeFunc freer){
    scene_add_bodies_force_creator(scene, forcer, aux, list_init(0, free), freer);
}

bool contains_removed_body(ForceHandler *fh){
  bool flag = false;
  for (size_t i = 0; i < list_size(fh->bodies); i++){
    Body *body = list_get(fh->bodies, i);
    if (body_is_removed(body)){
      flag = true;
    }
  }
  return flag;
}

void scene_tick(Scene *scene, double dt){
    scene->total_time += dt;
    size_t last_index_fh = list_size(scene->force_handlers) - 1;
    if(scene->follower != NULL){
        scene_set_camera(scene, scene->follower(scene->follower_aux));
    } else {
        scene_move_camera(scene, vec_multiply(dt, scene->camera_velocity));
    }

    for(size_t i = 0; i < last_index_fh + 1; i++){
      ForceHandler* fh = list_get(scene->force_handlers, last_index_fh - i);
      bool contains_removed_body = false;

      for (size_t j = 0; j < list_size(fh->bodies); j++){
        Body *body = list_get(fh->bodies, j);
        if (body_is_removed(body)){
          contains_removed_body = true;
        }
      }
      if (!contains_removed_body){
        fh->force(fh->aux);
      }
      else{
        list_remove(scene->force_handlers, last_index_fh - i);
        force_handler_free(fh);
      }
    }

    size_t last_index_bodies = scene_bodies(scene) - 1;
    for (size_t i = 0; i < last_index_bodies + 1; i++){
        Body *curr = scene_get_body(scene, last_index_bodies - i);
        if (body_is_removed(curr)){
          list_remove(scene->bodies, last_index_bodies - i);
        }
        else{
          body_tick(curr, dt);
        }
    }
}

void scene_set_camera(Scene *scene, Vector camera){
    scene->camera = camera;
}

void scene_move_camera(Scene *scene, Vector shift){
    scene->camera = vec_add(scene->camera, shift);
}

void scene_set_camera_velocity(Scene *scene, Vector velocity){
    scene->camera_velocity = velocity;
}

Vector scene_get_camera(Scene *scene){
    return scene->camera;
}

void scene_set_camera_follower(Scene *scene, CameraFollower follower, void *aux, FreeFunc freer){
    scene->follower = follower;
    scene->follower_aux = aux;
    scene->follower_freer = freer;
}
