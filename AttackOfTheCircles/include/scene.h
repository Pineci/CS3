#ifndef __SCENE_H__
#define __SCENE_H__

#include <stdbool.h>
#include "body.h"

/**
 * A collection of bodies and force creators.
 * The scene automatically resizes to store
 * arbitrarily many bodies and force creators.
 */
typedef struct scene Scene;

typedef struct force_handler ForceHandler;

/**
 * A function which adds some forces or impulses to bodies,
 * e.g. from collisions, gravity, or spring forces.
 * Takes in an auxiliary value that can store parameters or state.
 */
typedef void (*ForceCreator)(void *aux);

void *get_aux(ForceHandler *fh);

List *get_fh_bodies(ForceHandler *fh);

/**
 * Allocates memory for an empty scene.
 * Makes a reasonable guess of the number of bodies to allocate space for.
 * Asserts that the required memory is successfully allocated.
 *
 * @return the new scene
 */
Scene *scene_init(void);

/**
 * Releases memory allocated for a given scene
 * and all the bodies and force creators it contains.
 *
 * @param scene a pointer to a scene returned from scene_init()
 */
void scene_free(Scene *scene);

void scene_set_done(Scene* scene, bool done);

bool scene_is_done(Scene *scene);
int* scene_key_data(Scene *scene);
//void scene_set_jump_count(Scene *scene, int jump_count);
//void scene_add_one_jump(Scene *scene);
//int scene_jump_count(Scene *scene);
/**
 * Gets the number of bodies in a given scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @return the number of bodies added with scene_add_body()
 */
size_t scene_bodies(Scene *scene);

/**
 * Gets the body at a given index in a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 * @return a pointer to the body at the given index
 */
Body *scene_get_body(Scene *scene, size_t index);

/**
 * Adds a body to a scene.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param body a pointer to the body to add to the scene
 */
void scene_add_body(Scene *scene, Body *body);

/**
 * @deprecated Use body_remove() instead
 *
 * Removes and frees the body at a given index from a scene.
 * Asserts that the index is valid.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param index the index of the body in the scene (starting at 0)
 */
void scene_remove_body(Scene *scene, size_t index);

/**
 * @deprecated Use scene_add_bodies_force_creator() instead
 * so the scene knows which bodies the force creator depends on
 */
void scene_add_force_creator(
    Scene *scene, ForceCreator forcer, void *aux, FreeFunc freer
);

/**
 * Adds a force creator to a scene,
 * to be invoked every time scene_tick() is called.
 * The auxiliary value is passed to the force creator each time it is called.
 * The force creator is registered with a list of bodies it applies to,
 * so it can be removed when any one of the bodies is removed.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param forcer a force creator function
 * @param aux an auxiliary value to pass to forcer when it is called
 * @param bodies the list of bodies affected by the force creator.
 *   The force creator will be removed if any of these bodies are removed.
 *   This list does not own the bodies, so its freer should be NULL.
 * @param freer if non-NULL, a function to call in order to free aux
 */
void scene_add_bodies_force_creator(
    Scene *scene, ForceCreator forcer, void *aux, List *bodies, FreeFunc freer
);

/**
 * Executes a tick of a given scene over a small time interval.
 * This requires executing all the force creators
 * and then ticking each body (see body_tick()).
 * If any bodies are marked for removal, they should be removed from the scene
 * and freed, along with any force creators acting on them.
 *
 * @param scene a pointer to a scene returned from scene_init()
 * @param dt the time elapsed since the last tick, in seconds
 */

typedef Vector (*CameraFollower)(void *aux);

void scene_tick(Scene *scene, double dt);

void scene_set_camera(Scene *scene, Vector camera);

void scene_move_camera(Scene *scene, Vector shift);

void scene_set_camera_follower(Scene *scene, CameraFollower follower, void *aux, FreeFunc aux_freer);

void scene_set_camera_velocity(Scene *scene, Vector velocity);

Vector scene_get_camera(Scene *scene);

void scene_set_finished_level(Scene *scene);

bool scene_check_finished_level(Scene *scene);

double scene_get_time(Scene *scene);

bool scene_get_finished_title_screen(Scene *scene);

void scene_set_finished_title_screen(Scene *scene, bool setting);

#endif // #ifndef __SCENE_H__
