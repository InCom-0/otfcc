#ifndef CARYLL_SUPPORT_JSON_IDENT_H
#define CARYLL_SUPPORT_JSON_IDENT_H

#include "json.h"
#include "uthash.h"
#include <inttypes.h>
#include <stdbool.h>
#include <stdint.h>

bool json_ident(const json_value *a, const json_value *b);

#endif
