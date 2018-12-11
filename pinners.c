#include <asm/io.h>
#include <mach/platform.h>
#include <linux/init.h>
#include <linux/module.h>

#include <linux/timer.h>
#include <linux/err.h>
#include <linux/jiffies.h>

struct GPIO_Regs
{
	uint32_t GPFSEL[6];
	uint32_t Reserved1;
	uint32_t GPSET[2];
	uint32_t Reserved2;
	uint32_t GPCLR[2];
};

struct GPIO_Regs *gpio_regs;

static struct timer_list s_BlinkTimer;
static int s_BlinkPeriod = 1000;


static void SetGPIOFunction(int GPIO, int functionCode)
{
    int registerIndex = GPIO / 10;
    int bit = (GPIO % 10) * 3;

    unsigned oldValue = gpio_regs ->GPFSEL[registerIndex];
    unsigned mask = 0b111 << bit;
    printk("Changing function of GPIO%d from %x to %x\n", GPIO, (oldValue >> bit) & 0b111, functionCode);
    gpio_regs ->GPFSEL[registerIndex] = (oldValue & ~mask) | ((functionCode << bit) & mask);
}

static void SetGPIOOutputValue(int GPIO, bool outputValue)
{
    if (outputValue)
        gpio_regs ->GPSET[GPIO / 32] = (1 << (GPIO % 32));
    else
        gpio_regs ->GPCLR[GPIO / 32] = (1 << (GPIO % 32));
}

// Funcion para hacer que el led cambie estado on->off, off->on, Se llamara
// de forma automatica tras las interrupciones del temporizador indicadas
static void BlinkTimerHandler(unsigned long unused) {
	static bool on = false;
	on = !on;

	SetGPIOOutputValue(27, on);
	mod_timer(&s_BlinkTimer, jiffies + msecs_to_jiffies(s_BlinkPeriod));
	
}

static int init_Module(void) {
	printk(KERN_ALERT "Accediendo al GPIO\n");
	gpio_regs = (struct GPIO_Regs *) __io_address(GPIO_BASE);

	//Configurar el pin como salida, recordar en que estado estaba
	SetGPIOFunction(22, 1);
	SetGPIOFunction(27, 1);
	SetGPIOFunction(23, 0);
	SetGPIOFunction(18, 0);


	setup_timer(&s_BlinkTimer, BlinkTimerHandler, 0);
	mod_timer(&s_BlinkTimer, jiffies + msecs_to_jiffies(s_BlinkPeriod));
	return 0;
	
}
	//Hacer una función que active o desactive el pin
static void close_Module(void) {
	// Cerrar el módulo y dejar el pin como estaba
	SetGPIOOutputValue(22, false);
	SetGPIOOutputValue(27, false);

	del_timer(&s_BlinkTimer);
	
}

module_init(init_Module);
module_exit(close_Module);