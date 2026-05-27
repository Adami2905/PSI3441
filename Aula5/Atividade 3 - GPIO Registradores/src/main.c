#define SIM_SCGC5 (*((volatile unsigned int*)0x40048038)) //habilita clock da porta B
#define GPIOB_PDDR (*((volatile unsigned int*)0x400FF054)) //define se pino é INPUT ou OUTPUT
#define PORTB_PCR19 (*((volatile unsigned int*)0x4004A04C)) // Para definir o PTB19 como GPIO

//registrador para alterar o nível do pino - opções: PDOR, PSOR, PCOR, PTOR

#define GPIOB_PDOR  (*((volatile unsigned int*)0x400FF040))
#define GPIOB_PSOR (*((volatile unsigned int*)0x400FF044))
#define GPIOB_PCOR (*((volatile unsigned int*)0x400FF048))
#define GPIOB_PTOR (*((volatile unsigned int*)0x400FF04C))


void delayMs (int n);

/* Função: Espera n milisegundos */
/* esta função depende do clock default do microcontrolador. Para o KL25Z a frequência é 21 MHz aproximadamente. 
O valor do contador deverá ser ajustado para se conseguir o tempo de espera desejado. */
void delayMs (int n) {
	volatile int i;
	volatile int j;
	for (i = 0; i < n; i++)
		for (j = 0; j < 7000; j++) {
        }
}

int main(){
    SIM_SCGC5 |= (1 << 10);

    PORTB_PCR19 = (1 << 8);
    GPIOB_PDDR |= (1 << 19);


    while(1){
        GPIOB_PTOR = (1 << 19);
        delayMs(1000);
    }
}

