#include <kernel.h>
#include <sys/printk.h>

volatile int saldo_vitrine = 0;

//mutex:
K_MUTEX_DEFINE(mutex_vitrine);
//semáforo:
K_SEM_DEFINE(paes_disponiveis, 0, 10);
K_SEM_DEFINE(espacos_livres, 10, 10);

void padeiro_thread(void)
{
    while (1) {
        k_sem_take(&espacos_livres, K_FOREVER);
		k_mutex_lock(&mutex_vitrine, K_FOREVER);
		saldo_vitrine++;
		k_mutex_unlock(&mutex_vitrine);
		k_sem_give(&paes_disponiveis);
		k_msleep(1000);
	}
}


void cliente_thread(void)
{
    while (1) {
		k_sem_take(&paes_disponiveis, K_FOREVER);
		k_mutex_lock(&mutex_vitrine, K_FOREVER);
		saldo_vitrine--;
		k_mutex_unlock(&mutex_vitrine);
		k_sem_give(&espacos_livres);
		k_msleep(1500);
    }
}

/* K_THREAD_DEFINE implementa:
- reserva uma pilha (stack)
- define uma struct
- inicializa junto com a inicialização do sistema*/

K_THREAD_DEFINE(tid1, 1024, padeiro_thread,
                NULL, NULL, NULL,
                5, 0, 0);

K_THREAD_DEFINE(tid2, 1024, cliente_thread,
                NULL, NULL, NULL,
                5, 0, 0);

int main(void)
{
    while (1) {
		k_mutex_lock(&mutex_vitrine, K_FOREVER);
		printk("saldo_vitrine = %d\n", saldo_vitrine);
        k_msleep(50);
		k_mutex_unlock(&mutex_vitrine);
    }
}