

#include "systick.h"
#include "mcal_gpio.h"
#include "mcal_macro.h"
#include "stm32f411xe.h"


int gpio_test_main(void)
{
	mcal_gpio_init(105);

    for (;;)
    {

        mcal_gpio_high(105);
        delay_ms(100);
        //for (volatile unsigned int i = 0; i < 12000000; i++) ;

        mcal_gpio_low(105);        
        delay_ms(100);        
        //for (volatile unsigned int i = 0; i < 12000000; i++) ;
    }

	return 0;
}
