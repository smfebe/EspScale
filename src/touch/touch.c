#include <zephyr/kernel.h>
#include <zephyr/drivers/gpio.h>
#include <zephyr/logging/log.h>
#include "scale.h"

LOG_MODULE_REGISTER(touch, LOG_LEVEL_INF);

struct touch_btn {
    struct gpio_dt_spec spec;
    struct gpio_callback cb;
    struct k_work_delayable work;
    void (*action)(void);
};

static struct touch_btn btns[] = {
    { .spec = GPIO_DT_SPEC_GET(DT_ALIAS(touch0), gpios), .action = scale_calc_offset },
    { .spec = GPIO_DT_SPEC_GET(DT_ALIAS(touch1), gpios), .action = scale_calibrate },
};

static void touch_work_handler(struct k_work *work)
{
    struct touch_btn *b = CONTAINER_OF(k_work_delayable_from_work(work),
                                       struct touch_btn, work);
    if (gpio_pin_get_dt(&b->spec) == 1) {
        LOG_INF("Touch on pin %d", b->spec.pin);
        b->action();
    }
}

static void touch_isr(const struct device *dev, struct gpio_callback *cb, uint32_t pins)
{
    struct touch_btn *b = CONTAINER_OF(cb, struct touch_btn, cb);
    k_work_reschedule(&b->work, K_MSEC(50));
}

int touch_init(void)
{
    for (size_t i = 0; i < ARRAY_SIZE(btns); i++) {
        struct touch_btn *b = &btns[i];

        if (!gpio_is_ready_dt(&b->spec)) {
            return -ENODEV;
        }
        int ret = gpio_pin_configure_dt(&b->spec, GPIO_INPUT);
        if (ret < 0) {
            return ret;
        }
        k_work_init_delayable(&b->work, touch_work_handler);
        gpio_init_callback(&b->cb, touch_isr, BIT(b->spec.pin));
        gpio_add_callback(b->spec.port, &b->cb);

        ret = gpio_pin_interrupt_configure_dt(&b->spec, GPIO_INT_EDGE_TO_ACTIVE);
        if (ret < 0) {
            return ret;
        }
    }
    return 0;
}