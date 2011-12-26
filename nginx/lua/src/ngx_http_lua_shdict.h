#ifndef NGX_HTTP_LUA_SHDICT_H
#define NGX_HTTP_LUA_SHDICT_H


#include "ngx_http_lua_common.h"


typedef struct {
    u_char                       color;
    u_char                       dummy;
    u_short                      key_len;
    ngx_queue_t                  queue;
    ngx_msec_t                   expires;
    uint8_t                      value_type;
    uint32_t                     value_len;
    u_char                       data[1];
} ngx_http_lua_shdict_node_t;


typedef struct {
    ngx_rbtree_t                  rbtree;
    ngx_rbtree_node_t             sentinel;
    ngx_queue_t                   queue;
} ngx_http_lua_shdict_shctx_t;


typedef struct {
    ngx_http_lua_shdict_shctx_t  *sh;
    ngx_slab_pool_t              *shpool;
    ngx_str_t                     name;
} ngx_http_lua_shdict_ctx_t;


ngx_int_t ngx_http_lua_shdict_init_zone(ngx_shm_zone_t *shm_zone, void *data);

void ngx_http_lua_shdict_rbtree_insert_value(ngx_rbtree_node_t *temp,
    ngx_rbtree_node_t *node, ngx_rbtree_node_t *sentinel);

void ngx_http_lua_inject_shdict_api(ngx_http_lua_main_conf_t *lmcf,
        lua_State *L);


#endif /* NGX_HTTP_LUA_SHDICT_H */

