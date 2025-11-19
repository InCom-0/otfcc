#ifndef CARYLL_INCLUDE_TABLE_HDMX_H
#define CARYLL_INCLUDE_TABLE_HDMX_H

#include <otfcc/table/table-common.h>
#include <otfcc/table/maxp.h>

typedef struct {
	uint8_t pixelSize;
	uint8_t maxWidth;
	uint8_t *widths;
} device_record;

typedef struct {
	// Horizontal device metrics
	uint16_t version;
	uint16_t numRecords;
	uint32_t sizeDeviceRecord;
	OWNING device_record *records;
} table_hdmx;

extern caryll_RefElementInterface(table_hdmx) table_iHdmx;

#endif
