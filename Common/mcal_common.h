
#ifndef MCAL_COMMON_H
#define MCAL_COMMON_H

#include <stdint.h>
#include <stddef.h>   // NULL 등
#include <stdbool.h>  // bool 타입 (필요시)


/* =========================================================
 * Status
 * ========================================================= */
typedef enum

{

	MCAL_OK      = 0x00U,
	MCAL_ERROR   = 0x01U

} mcal_status_t;

#endif /* MCAL_COMMON_H */
