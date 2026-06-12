#include <kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <device.h>             // API para obter e utilizar dispositivos do sistema
#include <drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)

// Define o valor do registrador MOD do TPM para configurar o período do PWM
#define TPM_MODULE 37500         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty  = TPM_MODULE/930;    

//variáveis globais para input capture
volatile uint16_t rise_time = 0;
volatile uint16_t fall_time = 0;
volatile uint16_t pulse_width = 0;

volatile uint8_t captured = 0;

void TPM1_IRQHandler(void)
{
    if (TPM1->STATUS & TPM_STATUS_CH0F_MASK)
    {
        TPM1->STATUS |= TPM_STATUS_CH0F_MASK;

        uint16_t current = TPM1->CONTROLS[0].CnV;

        static uint8_t state = 0;
        static uint16_t rise = 0;

        if (state == 0)
        {
            rise = current;
            state = 1;
        }
        else
        {
            pulse_width = current - rise;
            state = 0;
        }
    }
}

int main(void)
{
	SIM->SCGC5 |= SIM_SCGC5_PORTA_MASK;
	PORTA->PCR[12] &= ~PORT_PCR_MUX_MASK;
	PORTA->PCR[12] |= PORT_PCR_MUX(3);   // TPM1_CH0
    // Inicializa o módulo TPM2 com:
    // - base do TPMx
    // - fonte de clock PLL/FLL (TPM_CLK)
    // - valor do registrador MOD
    // - tipo de clock (TPM_CLK)
    // - prescaler de 1 a 128 (PS)
    // - modo de operação EDGE_PWM
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);

	//inicialização do input capture:


    // Inicializa o canal 0 do TPM2 para gerar sinal PWM na porta GPIOB_18
    // - modo TPM_PWM_H (nível alto durante o pulso)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18);

	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;

	SIM->SCGC6 |= SIM_SCGC6_TPM1_MASK;

	TPM1->SC = 0;
	TPM1->CNT = 0;
	TPM1->MOD = 0xFFFF;

	TPM1->SC = TPM_SC_PS(7) | TPM_SC_CMOD(1);

	TPM1->CONTROLS[0].CnSC =
    TPM_CnSC_ELSA_MASK |   // rising
    TPM_CnSC_ELSB_MASK |   // falling
    TPM_CnSC_CHIE_MASK;    // interrupt enable

	IRQ_CONNECT(TPM1_IRQn, 1, TPM1_IRQHandler, NULL, 0);
	irq_enable(TPM1_IRQn);

    // Define o valor do duty cycle: nesse caso, duty_100 (LED quase desligado)
    pwm_tpm_CnV(TPM2, 0, duty);
	

    // Loop infinito
    for (;;)
	{
	// //ticks:
    // printk("Pulse width (ticks): %u\n", pulse_width);
	// //tempo:
	// int time_us = pulse_width * 2.67f;
	// printk("Time (us): %d\n", time_us);
	// //distância
	// int distance_cm = time_us / 58;
	// printk("Distance (cm): %d\n", distance_cm);
    // k_msleep(500);
	}

    return 0;

}