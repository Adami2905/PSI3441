#include "MKL25Z4.h"


int main()
{
    SIM->SCGC5 |= SIM_SCGC5_PORTB_MASK | SIM_SCGC5_PORTD_MASK;
    SIM->SCGC6 |= SIM_SCGC6_ADC0_MASK;

    PORTB->PCR[19] = PORT_PCR_MUX(1);
    PORTD->PCR[1]  = PORT_PCR_MUX(1);

    PTB->PDDR |= (1 << 19);
    PTD->PDDR |= (1 << 1);

    ADC0->CFG1 =
        ADC_CFG1_MODE(3) |
        ADC_CFG1_ADICLK(0) |
        ADC_CFG1_ADIV(2);

    ADC0->SC2 = 0;
    PORTB->PCR[0] = PORT_PCR_MUX(0);

    while (1)
    {
        ADC0->SC1[0] = 8;


        while (!(ADC0->SC1[0] & ADC_SC1_COCO_MASK));

        uint16_t valor = ADC0->R[0];

        if (valor > 3800)
        {
            PTD->PCOR = (1 << 1);
            PTB->PSOR = (1 << 19);
        }
        else if (valor < 200)
        {
            PTB->PCOR = (1 << 19);
            PTD->PSOR = (1 << 1);
        }
        else
        {
            PTB->PSOR = (1 << 19);
            PTD->PSOR = (1 << 1);
        }
    }
}