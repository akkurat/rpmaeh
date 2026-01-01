
#include "hardware/clocks.h"
/**
 *  @brief Set frequency and duty cycle for any PWM slice and channel
 *  @param[in] slice_num  The slice number the GPIO is associated to
 *  @param[in] chan       The channel number the GPIO is associated to
 *  @param[in] freq       The required frequency to be set
 *  @param[in] duty_cycle The required duty cycle in percentage 1->100
 *
 *  @return 1: Success; <0: Error
 */
int32_t pwm_set_freq_duty(uint32_t slice_num, uint32_t chan, uint32_t freq,
                          int duty_cycle);

int32_t pwm_set_freq_duty(uint32_t slice_num, uint32_t chan, uint32_t freq,
                          int duty_cycle)
{

    uint8_t clk_divider = 0;
    uint32_t wrap = 0;
    uint32_t clock_div = 0;
    uint32_t clock = clock_get_hz(clk_sys);

    if (freq < 8 || freq > clock)
        /* This is the frequency range of generating a PWM
        in RP2040 at 125MHz */
        return -1;

    for (clk_divider = 1; clk_divider < UINT8_MAX; clk_divider++)
    {
        /* Find clock_division to fit current frequency */
        clock_div = clock / clk_divider;
        wrap = clock_div / freq;
        if ((clock_div / UINT16_MAX) <= freq && wrap <= UINT16_MAX)
        {
            break;
        }
    }
    if (clk_divider < UINT8_MAX)
    {
        /* Only considering whole number division */
        pwm_set_clkdiv_int_frac(slice_num, clk_divider, 0);
        pwm_set_enabled(slice_num, true);
        pwm_set_wrap(slice_num, (uint16_t)wrap);
        pwm_set_chan_level(slice_num, chan,
                           (uint16_t)((((uint16_t)(duty_cycle == 100 ? (wrap + 1) : wrap)) * duty_cycle) / 100));
    }
    else
        return -2;

    return 1;
}