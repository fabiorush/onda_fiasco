#include <l4/io/io.h>
#include <l4/irq/irq.h>
#include <l4/util/util.h>
#include <l4/sys/kdebug.h>
#include <l4/sys/ipc.h>
#include <stdio.h>
#include <string.h>
#include "omap3530.h"
/*#include <l4/re/c/util/cap_alloc.h>
#include <l4/re/c/namespace.h>
#include <l4/sys/ipc.h>
#include <l4/sys/ipc.h>
#include <l4/sys/capability>
#include <l4/sys/typeinfo_svr>
#include <l4/sys/ipc_gate>
#include <l4/sys/factory>
#include <l4/cxx/ipc_stream>
#include <l4/cxx/thread>
#include <l4/re/env>
#include <l4/re/util/cap_alloc>*/
#include <l4/re/env>
#include <l4/re/util/cap_alloc>
#include <l4/re/util/object_registry>
#include <l4/cxx/ipc_server>

#define in32(a)		*((unsigned int *)(a))
#define out32(a,b)	*((unsigned int *)(a)) = (b)

//L4_EXTERNAL_FUNC(l4x_srv_init);
//L4_EXTERNAL_FUNC(l4x_srv_setup_recv);

int pincount = 100;
int interval = 10;
int pwm_enable = 0;

l4_addr_t gpio5, sys, gpt3;

int t = 0;
//static L4::Cap<L4::Kobject> _rcv_cap;

static char page_to_map[L4_PAGESIZE] __attribute__((aligned(L4_PAGESIZE)));

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

  puts("\n\nRecebi algo!\n\n");
  // We're only talking the Map_example protocol
  //if (t.label() != Protocol::Map_example)
    //return -L4_EBADPROTO;

  L4::Opcode opcode;
  ios >> opcode;

  //switch (opcode)
    {
    //case Opcode::Do_map:
      l4_addr_t snd_base;
      ios >> snd_base;
      // put something into the page to read it out at the other side
      //snprintf(page_to_map, sizeof(page_to_map), "Hello from the server!");
      printf("Sending to client\n");
      // send page
      ios << 10;/*L4::Ipc::Snd_fpage::mem((l4_addr_t)page_to_map, L4_PAGESHIFT,
                                L4_FPAGE_RO, snd_base);*/
      return L4_EOK;
    /*default:
      return -L4_ENOSYS;*/
    }
}
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

/*void
l4x_srv_init(void)
{
        _rcv_cap = L4Re::Util::cap_alloc.alloc<L4::Kobject>();
        if (!_rcv_cap)
                printf("l4x_srv: rcv-cap alloc failed\n");
}

void
l4x_srv_setup_recv(l4_utcb_t *u)
{
        l4_utcb_br_u(u)->br[0] = _rcv_cap.cap() | L4_RCV_ITEM_SINGLE_CAP
                                 | L4_RCV_ITEM_LOCAL_ID;
        l4_utcb_br_u(u)->bdr = 0;
}*/

int
main(void)
{
	l4irq_t *irqdesc;
	l4_msgtag_t tag;
	l4_umword_t label;
	l4_cap_idx_t comm_cap;
	//l4_utcb_t *utcb;
	
	//l4x_srv_init();

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
  static Smap_server smap;
	
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
		

  // Register server
  if (!server.registry()->register_obj(&smap, "comm").is_valid())
    {
      printf("Could not register my service, read-only namespace?\n");
      return 1;
    }

  printf("Welcome to the memory map example server!\n");

  // Wait for client requests
	printf("Ready to awnser4\n");
  server.loop();
	
	/*utcb = l4_utcb();

	l4x_srv_setup_recv(utcb);

	/*comm_cap = l4re_get_env_cap("comm");

	if (l4_is_invalid_cap(comm_cap)) {
		printf("Did not find an comm\n");
		//mutex_unlock(&sysfs_lock);
		return 0;
    }

	tag = l4_ipc_wait(utcb, &label, L4_IPC_NEVER);
	//tag = l4_ipc_wait(comm_cap, &label, L4_IPC_NEVER);*/
	// Wait for client requests
	

#if 0		
	while (1) {
		//l4_msgtag_t tag;
		//l4_umword_t label;
		int ipc_error;
		long t;
		/* Check if we had any IPC failure, if yes, print the error code
		* and just wait again. */
		printf("Request received\n");

		ipc_error = l4_ipc_error(tag, l4_utcb());
		if (ipc_error) {
			fprintf(stderr, "IPC error: %x\n", ipc_error);
			tag = l4_ipc_wait(utcb, &label, L4_IPC_NEVER);
			//tag = l4_ipc_wait(comm_cap, &label, L4_IPC_NEVER);
			continue;
		}
		
		/* So, the IPC was ok, now take the value out of message register 0
		* of the UTCB and store the square of it back to it. */
		t = l4_msgtag_label(tag);
		switch (t) {
		case 1:
			printf("Eh um pincount\n");
			l4_utcb_mr()->mr[0] = pincount;
			break;
		case 2:
			printf("Eh um interval\n");
			l4_utcb_mr()->mr[0] = interval;
			break;
		case 3:
			printf("Eh um pwm_enable\n");
			l4_utcb_mr()->mr[0] = pwm_enable;
			break;
		default:
			printf("ao sei o q eh\n");
		}

		/* Send the reply and wait again for new messages.
		* The '1' in the msgtag indicated that we want to transfer 1 word in
		* the message registers (i.e. MR0) */
		//tag = l4_ipc_reply_and_wait(l4_utcb(), l4_msgtag(t, 1, 0, 0), &label, L4_IPC_NEVER);
		tag = l4_ipc_reply_and_wait(utcb, l4_msgtag(t, 1, 0, 0), &label, L4_IPC_NEVER);
    }
	
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
#endif

	return 0;
}
