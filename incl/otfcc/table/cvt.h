#ifndef CARYLL_INCLUDE_TABLE_CVT_H
#define CARYLL_INCLUDE_TABLE_CVT_H

#include <otfcc/table/table-common.h>

typedef struct {
	uint32_t length;
	OWNING uint16_t *words;
} table_cvt;
extern caryll_RefElementInterface(table_cvt) table_iCvt;

#endif
