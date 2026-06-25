#include <zephyr/kernel.h>
#include <zephyr/device.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/drivers/adc.h>
#include <zephyr/drivers/sensor.h>
#include <zephyr/sys/printk.h>
#include <zephyr/sys/atomic.h>
#include <stdlib.h>

/* ADC */
#define ADC_RESOLUTION       12
#define ADC_GAIN             ADC_GAIN_1
#define ADC_REFERENCE        ADC_REF_INTERNAL
#define ADC_ACQUISITION_TIME ADC_ACQ_TIME_DEFAULT
#define ADC_CHANNEL_ID       0
#define ADC_VREF_MV          3300

static const struct device *const adc_dev = DEVICE_DT_GET(DT_NODELABEL(adc0));
static int16_t sample_buffer;

/* Acelerômetro */
static const struct device *const accel = DEVICE_DT_GET(DT_NODELABEL(mma8451q));

/* LED e botão */
#define LED_C_NODE   DT_ALIAS(led1)
#define BUTTON_NODE  DT_NODELABEL(user_button_0)

static const struct gpio_dt_spec ledc = GPIO_DT_SPEC_GET(LED_C_NODE, gpios);
static const struct gpio_dt_spec button = GPIO_DT_SPEC_GET(BUTTON_NODE, gpios);
static struct gpio_callback button_cb_data;

/* 0 = modo ADC; 1 = modo completo */
static atomic_t modo_completo = ATOMIC_INIT(0);

/* ISR do botão */
void button_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    gpio_pin_toggle_dt(&ledc);
    atomic_xor(&modo_completo, 1);
}

/* Thread do ADC */
void adc_thread(void)
{
    struct adc_channel_cfg channel_cfg = {
        .gain = ADC_GAIN,
        .reference = ADC_REFERENCE,
        .acquisition_time = ADC_ACQUISITION_TIME,
        .channel_id = ADC_CHANNEL_ID,
        .differential = 0,
    };

    struct adc_sequence sequence = {
        .channels = BIT(ADC_CHANNEL_ID),
        .buffer = &sample_buffer,
        .buffer_size = sizeof(sample_buffer),
        .resolution = ADC_RESOLUTION,
    };

    while (1) {
        int err = adc_read(adc_dev, &sequence);

        if (err == 0) {
            int32_t mv = sample_buffer;
            adc_raw_to_millivolts(ADC_VREF_MV, ADC_GAIN, ADC_RESOLUTION, &mv);

            printk("[ADC] raw = %d, tensao = %d mV\n", sample_buffer, mv);
        } 
        k_sleep(K_MSEC(500));
    }
}

/* Thread do acelerômetro */
void accel_thread(void)
{
    struct sensor_value ax, ay, az;


    while (1) {
        if (atomic_get(&modo_completo)) {
            int ret = sensor_sample_fetch(accel);

            if (ret == 0) {
                sensor_channel_get(accel, SENSOR_CHAN_ACCEL_X, &ax);
                sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Y, &ay);
                sensor_channel_get(accel, SENSOR_CHAN_ACCEL_Z, &az);

                printk("[ACC] X=%d.%06d, Y=%d.%06d, Z=%d.%06d\n",
                       ax.val1, abs(ax.val2),
                       ay.val1, abs(ay.val2),
                       az.val1, abs(az.val2));
            } else {
                printk("[ACC] erro de leitura: %d\n", ret);
            }
        }

        k_sleep(K_MSEC(1000));
    }
}


void main(void)
{
    printk( "Iniciando programa...\n\n\n" );

    gpio_pin_configure_dt(&ledc, GPIO_OUTPUT_INACTIVE);
    gpio_pin_configure_dt(&button, GPIO_INPUT | GPIO_PULL_UP);

    gpio_pin_interrupt_configure_dt(&button, GPIO_INT_EDGE_FALLING);
    gpio_init_callback(&button_cb_data, button_isr, BIT(button.pin));
    gpio_add_callback(button.port, &button_cb_data);

    printk("Botao em %s, pino %d\n", button.port->name, button.pin);
    printk("Modo inicial: ADC\n");

    while (1) {
        k_sleep(K_FOREVER);
    }
}

/* Threads */
K_THREAD_DEFINE(adc_tid, 1024, adc_thread, NULL, NULL, NULL, 6, 0, 0);
K_THREAD_DEFINE(accel_tid, 1024, accel_thread, NULL, NULL, NULL, 4, 0, 0);