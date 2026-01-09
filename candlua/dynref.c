#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <lua.h>
#include <lauxlib.h>

typedef struct {
    const char *name;
    void *fn;
    const char *sig; /* e.g. "ii>i" args->ret, types: i=int, d=double, s=string */
} FuncEntry;

static FuncEntry *registry = NULL;
static size_t registry_count = 0;

static void registry_add(const char *name, void *fn, const char *sig) {
    registry = (FuncEntry*)realloc(registry, sizeof(FuncEntry) * (registry_count + 1));
    registry[registry_count].name = name;
    registry[registry_count].fn = fn;
    registry[registry_count].sig = sig;
    registry_count++;
}

static FuncEntry *registry_find(const char *name) {
    for (size_t i = 0; i < registry_count; ++i) {
        if (strcmp(registry[i].name, name) == 0) return &registry[i];
    }
    return NULL;
}

/* A tiny dispatcher supporting a few simple signatures for demo purposes. */
int dynref_call(lua_State *L) {
    const char *name = luaL_checkstring(L, 1);
    FuncEntry *e = registry_find(name);
    if (!e) return luaL_error(L, "unknown function '%s'", name);

    const char *sig = e->sig;
    const char *sep = strchr(sig, '>');
    if (!sep) return luaL_error(L, "invalid signature for %s", name);
    size_t nargs = sep - sig;
    const char *ret = sep + 1;

    /* copy args part */
    char argsig[16] = {0};
    if (nargs >= sizeof(argsig)) return luaL_error(L, "signature too long for %s", name);
    memcpy(argsig, sig, nargs);

    /* Support: ii>i, i>i, s>s */
    if (strcmp(argsig, "ii") == 0 && strcmp(ret, "i") == 0) {
        /* two ints -> int */
        int a = luaL_checkinteger(L, 2);
        int b = luaL_checkinteger(L, 3);
        typedef int (*fn_t)(int,int);
        fn_t f = (fn_t)e->fn;
        int r = f(a,b);
        lua_pushinteger(L, r);
        return 1;
    }
    if (strcmp(argsig, "i") == 0 && strcmp(ret, "i") == 0) {
        int a = luaL_checkinteger(L, 2);
        typedef int (*fn_t)(int);
        fn_t f = (fn_t)e->fn;
        int r = f(a);
        lua_pushinteger(L, r);
        return 1;
    }
    if (strcmp(argsig, "s") == 0 && strcmp(ret, "s") == 0) {
        const char *s = luaL_checkstring(L, 2);
        typedef const char *(*fn_t)(const char *);
        fn_t f = (fn_t)e->fn;
        const char *r = f(s);
        lua_pushstring(L, r);
        return 1;
    }

    return luaL_error(L, "unsupported signature '%s>%s' for %s", sig, ret, name);
}

/* Register the reflect.call function into Lua */
int luaopen_dynref(lua_State *L) {
    lua_newtable(L);
    lua_pushcfunction(L, dynref_call);
    lua_setfield(L, -2, "call");
    return 1;
}

/* Helper to register C functions from the host program */
void dynref_register_c(const char *name, void *fn, const char *sig) {
    registry_add(name, fn, sig);
}
