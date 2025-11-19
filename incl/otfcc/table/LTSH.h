#ifndef CARYLL_INCLUDE_TABLE_LTSH_H
#define CARYLL_INCLUDE_TABLE_LTSH_H

#include <otfcc/table/table-common.h>

typedef struct {
	uint16_t version;
	glyphid_t numGlyphs;
	OWNING uint8_t *yPels;
} table_LTSH;
extern caryll_RefElementInterface(table_LTSH) table_iLTSH;

#endif
