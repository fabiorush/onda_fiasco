#include <l4/io/io.h>
#include <l4/irq/irq.h>
#include <l4/util/util.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/ipc.h>
#include <stdio.h>
#include <string.h>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>
#include "omap3530.h"

#define in32(a)		*((unsigned int *)(a))
#define out32(a,b)	*((unsigned int *)(a)) = (b)

int pincount = 100;
int interval = 10;
int pwm_enable = 0;

l4_addr_t gpio5, sys, gpt3, gpt9;

int t = 0;

//static char page_to_map[L4_PAGESIZE] __attribute__((aligned(L4_PAGESIZE)));

static L4Re::Util::Registry_server<> server;

class Smap_server : public L4::Server_object
{
public:
  int dispatch(l4_umword_t obj, L4::Ipc::Iostream &ios);
};

int
Smap_server::dispatch(l4_umword_t, L4::Ipc::Iostream &ios)
{
  l4_msgtag_t t;
  ios >> t;

  int opcode;
  ios >> opcode;
  
  switch (opcode) {
    case 0x55:
		printf("Eh um pincount read\n");
		ios << pincount;
		return L4_EOK;
	case 0x56:
		printf("Eh um interval read\n");
		ios << interval;
		return L4_EOK;
	case 0x66:
		printf("Eh um interval write\n");
		ios >> interval;
		ios << interval;
		return L4_EOK;
	case 0x57:
		printf("Eh um pwm_enable read\n");
		ios << pwm_enable;
		return L4_EOK;
	case 0x67:
		printf("Eh um pwm_enable write\n");
		ios >> pwm_enable;
		ios << pwm_enable;
		return L4_EOK;
	default:
		printf("ao sei o q eh\n");
		return -L4_ENOSYS;
    }
}

static void gpio_isr_handler(void *data)
{
	(void)data;
	if (in32(gpio5 + OMAP2420_GPIO_DATAIN) & (1 << 10)) {
		//out32(gpt3 + OMAP3530_GPT_TCLR, 0);
		out32(gpt9 + OMAP3530_GPT_TCLR, 0);
		//out32(gpt3 + OMAP3530_GPT_TISR, 2);
		out32(gpt9 + OMAP3530_GPT_TISR, 2);
		
		/* set the pin 139 */
		out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));
	} else {
		/* clear the pin 139*/
		out32(gpio5 + OMAP2420_GPIO_CLEARDATAOUT, (1 << 11));
		
		/* setting the initial timer counter value
		 * cada tick é 80ns */
		unsigned int t = 0xffffffff - ((interval*1000)/79);
		
		out32(gpt9 + OMAP3530_GPT_TLDR, t);
		out32(gpt9 + OMAP3530_GPT_TCRR, t);
		//out32(gpt3 + OMAP3530_GPT_TLDR, t);
		//out32(gpt3 + OMAP3530_GPT_TCRR, t);

		/* starting timer with PWM */
		out32(gpt9 + OMAP3530_GPT_TCLR, 3 | (1<<12) | (1<<10)); //-- PWM
		//out32(gpt3 + OMAP3530_GPT_TCLR, 3);

		t = 0;
	}
	out32(gpio5 + OMAP2420_GPIO_IRQSTATUS1, 1 << 10);
}

static void timer_isr_handler(void *data)
{
	(void)data;
	if (t)
		out32(gpio5 + OMAP2420_GPIO_CLEARDATAOUT, (1 << 11));
	else
		out32(gpio5 + OMAP2420_GPIO_SETDATAOUT, (1 << 11));

	t = (~t) & 1;
	//out32(gpt3 + OMAP3530_GPT_TISR, 2);
	out32(gpt9 + OMAP3530_GPT_TISR, 2);
}

int
main(void)
{
	l4irq_t *irqdesc, *irqdesc2;
	unsigned int l;
	static Smap_server smap;
	
	/* attach timer interrupt */
	//if (!(irqdesc = l4irq_request(39, timer_isr_handler, 0, 0xff, 0))) {
	if (!(irqdesc = l4irq_request(45, timer_isr_handler, 0, 0xff, 0))) {
		printf("Requesting IRQ %d failed\n", 45);
		return 1;
    }

	/* attach GPIO interrupt */
	if (!(irqdesc2 = l4irq_request(33, gpio_isr_handler, 0, 0xff, 0))) {
		printf("Requesting IRQ %d failed\n", 33);
		return 1;
    }

	if (l4io_request_iomem(OMAP3530_GPIO5_BASE, OMAP3530_GPIO_SIZE, 0, &gpio5))
		return 1;
		
	if (l4io_request_iomem(OMAP3530_SYSCTL_BASE, OMAP3530_SYSCTL_SIZE, 0, &sys))
		return 1;

	//if (l4io_request_iomem(OMAP3530_GPT3_BASE, OMAP3530_GPT_SIZE, 0, &gpt3))
	//	return 1;

	/* GPT9 used for PWM */
	if (l4io_request_iomem(OMAP3530_GPT9_BASE, OMAP3530_GPT_SIZE, 0, &gpt9))
		return 1;

	/* Register server */
	if (!server.registry()->register_obj(&smap, "comm").is_valid()) {
		printf("Could not register my service, read-only namespace?\n");
		return 1;
	}

	/* selecting pullup and mode 4 function - GPIO 139
	 * selecting mode 4 function - GPIO 138 */
	//l = (in32(sys + 0x168) & 0 ) | (((1<<3) | 4) << 16);
	//l = (in32(sys + 0x168) & ~(7<<16) ) | (4 << 16);
	out32(sys + 0x168, ((1<<3 | 4) << 16) | (1<<3) | 4);

	/* setting mode 2 - PWM */
	l = (in32(sys + 0x174) & ~7 ) | 2;
	out32(sys + 0x174, l);

	/* setting the PIN 138 to input
	 * setting the PIN 139 to output */
	l = (in32(gpio5 + OMAP2420_GPIO_OE) & ~(1 << 11)) | 1 << 10;
	out32(gpio5 + OMAP2420_GPIO_OE, l);
	
	/* enabling interrupt on both levels on GPIO 139 */
	out32(gpio5 + OMAP2420_GPIO_RISINGDETECT, l << 10);
	out32(gpio5 + OMAP2420_GPIO_FALLINGDETECT, l << 10);
	out32(gpio5 + OMAP2420_GPIO_SETIRQENABLE1, l << 10);

	/* make sure timer has stop */
	//out32(gpt3 + OMAP3530_GPT_TCLR, 0);
	out32(gpt9 + OMAP3530_GPT_TCLR, 0);

	/* enabling the interrupt */
	out32(gpt9 + OMAP3530_GPT_TIER, 2); //comentar se PWM
	
	/* Wait for client requests */
	printf("Ready to anwser\n");
	server.loop();

	if (l4irq_release(irqdesc)) {
		printf("Failed to release IRQ\n");
		return 1;
    }

	if (l4irq_release(irqdesc2)) {
		printf("Failed to release IRQ\n");
		return 1;
    }
	printf("Bye\n");

	return 0;
}
