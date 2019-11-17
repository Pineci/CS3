#include "gen_levels.h"
#include "scene.h"

List *get_bodies_type(Scene *scene, BODY_TYPE type){
    List *list = list_init(1, NULL);
    for(int i = 0; i < scene_bodies(scene); i++){
        Body *curr_body = scene_get_body(scene, i);
        BodyInfo *body_info = (BodyInfo*)body_get_info(curr_body);
        if(body_info->type == type){
            list_add(list, curr_body);
        }
    }

    return list;
}

Body *get_first_body(Scene *scene, BODY_TYPE type){
    List *list = get_bodies_type(scene, type);
    Body *body = list_get(list, 0);
    list_free(list);
    return body;
}
