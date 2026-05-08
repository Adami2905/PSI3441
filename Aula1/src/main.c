#include <zephyr.h>
#include <device.h>
#include <drivers/gpio.h>
#include <devicetree.h>

#define GPIOB_NODE DT_NODELABEL(gpiob)
#define GREEN_PIN 19
#define RED_PIN 18

void changeLED(const struct device *gpiob, int state){
	if (state == 0){ //verde
		gpio_pin_set(gpiob, RED_PIN, 1);   // vermelho OFF
		gpio_pin_set(gpiob, GREEN_PIN, 0); // verde ON
		k_msleep(1500);
	}

	else if (state == 1){ //amarelo
		gpio_pin_set(gpiob, RED_PIN, 0);   // vermelho ON
		gpio_pin_set(gpiob, GREEN_PIN, 0); // verde ON
		k_msleep(750);
	}

	else if (state == 2){ //vermelho
		gpio_pin_set(gpiob, RED_PIN, 0);   // vermelho ON
		gpio_pin_set(gpiob, GREEN_PIN, 1); // verde OFF
		k_msleep(1500);
	}
}

void main(void)
{
    const struct device *gpiob;
    
    gpiob = device_get_binding(DT_LABEL(GPIOB_NODE));
    if (!gpiob) return;

    gpio_pin_configure(gpiob, GREEN_PIN, GPIO_OUTPUT);
	gpio_pin_configure(gpiob, RED_PIN, GPIO_OUTPUT);


    int currentState = 2; //para começar no estado zero após a primeira execução do while abaixo
    while (1) {
		//estados:
		//0 - verde
		//1 - amarelo
		//2 - vermelho

		currentState++; 
		if (currentState > 2){ 
			currentState = 0;
		}
		changeLED(gpiob, currentState);
		
	}


}

