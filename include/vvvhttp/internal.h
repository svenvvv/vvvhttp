#pragma once

#include "vvvhttp.h"
#include "vvvlog.h"

#include <assert.h>
#define __ASSERT(expr) assert(expr)

#define VVVHTTP_RESPONSE_SERVER_ID "vvvhttp"

#define VVVHTTP_ROUTE_DEFS_START    __start_vvvhttp_routes
#define VVVHTTP_ROUTE_DEFS_END      __stop_vvvhttp_routes

extern struct vvvhttp_route const * const VVVHTTP_ROUTE_DEFS_START;
extern struct vvvhttp_route const * const VVVHTTP_ROUTE_DEFS_END;
