
#include "stm32f411xe.h"
#include "mcal_macro.h"

#define HCLK        96000000U
#define HSI         16000000U

#define PLLM 8U
#define PLLP 2U

#define PLLN    ((HCLK * PLLM * PLLP) / HSI)

void clock_init(void);
