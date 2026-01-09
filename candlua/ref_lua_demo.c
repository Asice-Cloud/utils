#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <lua.h>
#include <lauxlib.h>
#include <lualib.h>

/* forward from dynref.c */
void dynref_register_c(const char *name, void *fn, const char *sig);
int luaopen_dynref(lua_State *L);

/* Example C functions to register */
static int add_ints(int a, int b) {
    return a + b;
}

static const char *greet(const char *name) {
    static char buf[128];
    snprintf(buf, sizeof(buf), "Hello, %s!", name);
    return buf;
}

int main(int argc, char **argv) {
    lua_State *L = luaL_newstate();
    luaL_openlibs(L);

    /* register dynref module */
    luaL_requiref(L, "dynref", luaopen_dynref, 1);
    lua_pop(L, 1);

    /* register C functions in the dynref registry */
    dynref_register_c("add", (void*)add_ints, "ii>i");
    dynref_register_c("greet", (void*)greet, "s>s");

    const char *script = "script.lua";
    if (argc > 1) script = argv[1];

    if (luaL_dofile(L, script) != LUA_OK) {
        fprintf(stderr, "lua error: %s\n", lua_tostring(L, -1));
        lua_pop(L,1);
    }

    lua_close(L);
    return 0;
}
