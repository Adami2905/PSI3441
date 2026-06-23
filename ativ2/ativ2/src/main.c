#include <kernel.h>             // Funções básicas do Zephyr (ex: k_msleep, k_thread, etc.)
#include <device.h>             // API para obter e utilizar dispositivos do sistema
#include <drivers/gpio.h>       // API para controle de pinos de entrada/saída (GPIO)
#include <pwm_z42.h>                // Biblioteca personalizada com funções de controle do TPM (Timer/PWM Module)
#include <drivers/uart.h>
#include <sys/printk.h>
#include <stdlib.h>

#define RX_BUF_SIZE 16

//A única opção que deu certo no lugar de scanf() é o uso da UART com interrupção
static char rx_buf[RX_BUF_SIZE];
static int rx_idx = 0;
static volatile int valor_pronto = 0;
static int valor = 0;

static void uart_cb(const struct device *dev, void *user_data)
{
    uint8_t c;

    while (uart_fifo_read(dev, &c, 1) == 1) //Enquanto houver dado presente na fila da UART
    {
        uart_poll_out(dev, c); // eco

        if (c == '\r' || c == '\n') //detectou newline ou enter?
        {
            rx_buf[rx_idx] = '\0';

            if (rx_idx > 0)
            {
                valor = atoi(rx_buf);
                valor_pronto = 1; //flag que indica quando o valor completo foi recebido (ex.: recebeu 45 e não apenas o 4)
            }

            rx_idx = 0;
        }
        else if (c >= '0' && c <= '9') //unindo o dígito ao futuro valor final
        {
            if (rx_idx < RX_BUF_SIZE - 1)
            {
                rx_buf[rx_idx++] = c;
            }
        }
    }
}

// Define o valor do registrador MOD do TPM para configurar o período do PWM
#define TPM_MODULE 1000         // Define a frequência do PWM fpwm = (TPM_CLK / (TPM_MODULE * PS))
// Valores de duty cycle correspondentes a diferentes larguras de pulso
uint16_t duty_50  = TPM_MODULE/2;      


int main(void)
{
    // Inicializa o módulo TPM2 com:
    // - base do TPMx
    // - fonte de clock PLL/FLL (TPM_CLK)
    // - valor do registrador MOD
    // - tipo de clock (TPM_CLK)
    // - prescaler de 1 a 128 (PS)
    // - modo de operação EDGE_PWM
    pwm_tpm_Init(TPM2, TPM_PLLFLL, TPM_MODULE, TPM_CLK, PS_128, EDGE_PWM);


    // - modo TPM_PWM_H (nível alto durante o pulso)
    pwm_tpm_Ch_Init(TPM2, 0, TPM_PWM_H, GPIOB, 18); //vermelho
    pwm_tpm_Ch_Init(TPM2, 1, TPM_PWM_H, GPIOB, 19); //verde

    //iniciando os dois LEDs em nível baixo
    pwm_tpm_CnV(TPM2, 0, (1000));
    pwm_tpm_CnV(TPM2, 1, (1000));


    float brightness = 0.0;

    //iniciando a UART:
    const struct device *uart = device_get_binding("UART_0");


    //ativando a interrupção:
    uart_irq_callback_user_data_set(uart, uart_cb, NULL);
    uart_irq_rx_enable(uart);

    printk("Digite um numero entre 0 e 100 e pressione ENTER:\n");

    while (1)
    {
        if (valor_pronto)
        {
            valor_pronto = 0;

            printk("\nPorcentagem definida para brilho do LED: %d \n", valor);
            brightness = valor / 100.0f;
            pwm_tpm_CnV(TPM2, 0, (1000 - brightness * 1000));
            pwm_tpm_CnV(TPM2, 1, (1000 - brightness * 400));
            k_msleep(20);

        }

        k_msleep(100);
    }

    return 0;
}