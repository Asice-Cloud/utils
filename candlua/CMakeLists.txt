cmake_minimum_required(VERSION 3.5)
project(luatest C)

# Prefer CMake's FindLua; fall back to pkg-config (lua5.4, lua, lua54)
find_package(Lua 5.4 QUIET)

# Demo target that uses dynref
add_executable(luadyn ref_lua_demo.c dynref.c)

if(TARGET Lua::Lua)
	# Modern CMake target provided by some FindLua implementations
	target_link_libraries(luatest PRIVATE Lua::Lua m dl)
elseif(Lua_FOUND)
	# Older FindLua variables
	if(NOT DEFINED LUA_INCLUDE_DIR AND DEFINED LUA_INCLUDES)
		set(LUA_INCLUDE_DIR ${LUA_INCLUDES})
	endif()
	target_link_libraries(luadyn PRIVATE ${LUA_LIBRARIES} m dl)
else()
	find_package(PkgConfig REQUIRED)
	# Try several common pkg-config names; QUIET avoids noisy failures
	pkg_check_modules(LUA_PKG QUIET lua5.4 lua54 lua)
	if(NOT LUA_PKG_FOUND)
		message(FATAL_ERROR "Could not find Lua. Install Lua dev package or adjust pkg-config module name (e.g. lua5.4).")
	endif()
	target_include_directories(luatest PRIVATE ${LUA_PKG_INCLUDE_DIRS})
	target_link_libraries(luatest PRIVATE ${LUA_PKG_LIBRARIES} ${LUA_PKG_LDFLAGS} m dl)
	target_compile_options(luatest PRIVATE ${LUA_PKG_CFLAGS_OTHER})

	target_include_directories(luadyn PRIVATE ${LUA_PKG_INCLUDE_DIRS})
	target_link_libraries(luadyn PRIVATE ${LUA_PKG_LIBRARIES} ${LUA_PKG_LDFLAGS} m dl)
	target_compile_options(luadyn PRIVATE ${LUA_PKG_CFLAGS_OTHER})
endif()

# Helpful output
message(STATUS "Building luatest")
if(TARGET Lua::Lua)
	message(STATUS "Linking with Lua::Lua target")
elseif(Lua_FOUND)
	message(STATUS "Using FindLua: include=${LUA_INCLUDE_DIR} libs=${LUA_LIBRARIES}")
else()
	message(STATUS "Using pkg-config: ${LUA_PKG_INCLUDEDIR} ${LUA_PKG_LIBRARIES}")
endif()

