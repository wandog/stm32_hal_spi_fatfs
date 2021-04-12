#include "main.h"

int main(void)
{
	uint64_t CardSize = 0;
    SystemInit();
    NVIC_PriorityGroupConfig(NVIC_PriorityGroup_2);
    delay_init();
    uart_init(115200);

	while(SD_Initialize())
	{
		printf("##[Warning]: sd card init failed!\r\n");
		delay_ms(1000);
	}
	
	CardSize = SD_GetSectorCount();
	CardSize = CardSize * 512 / 1024 / 1024;
	printf("# SD Card Size: %lldMB\r\n", CardSize);
    while(1)
    {

    }
}
