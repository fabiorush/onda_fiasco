#include <l4/io/io.h>
#include <l4/irq/irq.h>
#include <l4/util/util.h>
#include <l4/sys/kdebug.h>
#include <stdio.h>
#include <string.h>
#include "omap3530.h"

#define in32(a)		*((unsigned int *)(a))
#define out32(a,b)	*((unsigned int *)(a)) = (b)

int pincount = 100;
int interval = 10;
int pwm_enable = 0;

l4_addr_t gpio5, sys, gpt3;


int t = 0;

static void isr_handler(void *data)
{
	(void)data;
	if (t)
		out32(gpio5 + OMAP2420_GPIO_CLEARDATAOUT, (1 << 11));
	else
		out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));

	t = (~t) & 1;
	out32(gpt3 + OMAP3530_GPT_TISR, 2);
}

int
main(void)
{
	l4irq_t *irqdesc;

	/*l4io_device_handle_t devhandle = l4io_get_root_device();
	l4io_device_t dev;
	l4io_resource_handle_t reshandle;
	l4io_resource_t res;

	if (l4io_lookup_device("GPIO", &devhandle, &dev, &reshandle)) {
		puts("Erro procurando dispositivos!");
		return 1;
	}
	
	if (!l4io_iterate_devices(&devhandle, &dev, &reshandle)) {
		puts("Erro procurando dispositivos!");
		return;
	}
	
	reshandle += 5;
	
	if (l4io_lookup_resource(devhandle, L4IO_RESOURCE_ANY,
									&reshandle, &res)) {
		puts("Erro procurando handle!");
		return 1;
	}*/

	unsigned int l;
	
	/* attach timer interrupt */
	if (!(irqdesc = l4irq_request(39, isr_handler, 0, 0xff, 0))) {
		printf("Requesting IRQ %d failed\n", 39);
		return 1;
    }

	if (l4io_request_iomem(OMAP3530_GPIO5_BASE, OMAP3530_GPIO_SIZE, 0, &gpio5))
		return 1;
		
	if (l4io_request_iomem(OMAP3530_SYSCTL_BASE, OMAP3530_SYSCTL_SIZE, 0, &sys))
		return 1;

	//GPT9 used for PWM
	//if (l4io_request_iomem(OMAP3530_GPT3_BASE, OMAP3530_GPT_SIZE, 0, &gpt3))
	if (l4io_request_iomem(OMAP3530_GPT9_BASE, OMAP3530_GPT_SIZE, 0, &gpt3))
		return 1;
		
	/* selecting pullup and mode 4 function - GPIO 139 */
	l = (in32(sys + 0x174) & ~(7<<16) ) | (((1<<3) | 4) << 16);
	//l = (in32(sys + 0x168) & ~(7<<16) ) | (4 << 16);
	out32(sys + 0x168, l);

	/* setting mode 2 - PWM */
	l = (in32(sys + 0x174) & ~7 ) | 2;
	out32(sys + 0x174, l);

	/* setting the PIN 139 to output */
	l = in32(gpio5 + OMAP2420_GPIO_OE) & ~(1 << 11);
	out32(gpio5 + OMAP2420_GPIO_OE, l);
	

	out32(gpt3 + OMAP3530_GPT_TCLR, 0);

	sleep(1);

	/* setting the initial timer counter value
	 * cada tick é 80ns
	 */
	out32(gpt3 + OMAP3530_GPT_TLDR, 0xffffff80);
	out32(gpt3 + OMAP3530_GPT_TCRR, 0xffffff80);

	/* enabling the interrupt */
	//out32(gpt3 + OMAP3530_GPT_TIER, 2);

	sleep(1);

	/* starting timer with PWM */
	out32(gpt3 + OMAP3530_GPT_TCLR, 3 | (1<<12) | (1<<10));
	//out32(gpt3 + OMAP3530_GPT_TCLR, 3);
	
	/*for (;;)
	{
		//puts("Onda!");
		//printf("   resource: %d %x %lx-%lx\n",
			//res.type, res.flags, res.start, res.end);
		//dump_vbus();
		//unsigned int *rev = (unsigned int *) (sys + 0x168);
		//printf("rev: %08x\n", *rev);
		out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));
		sleep(1);
		out32(gpio5 + OMAP2420_GPIO_CLEARDATAOUT, (1 << 11));
		sleep(1);
	}*/
	
	printf("Esperando interrupção\n");
	sleep(10);
	out32(gpt3 + OMAP3530_GPT_TCLR, 0);
	out32(gpt3 + OMAP3530_GPT_TIER, 0);

	if (l4irq_release(irqdesc)) {
		printf("Failed to release IRQ\n");
		return 1;
    }

	printf("Bye\n");

	return 0;
}
