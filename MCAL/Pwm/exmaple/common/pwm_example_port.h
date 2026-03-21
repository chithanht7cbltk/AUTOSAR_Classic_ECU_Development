/**********************************************************
 * @file    pwm_example_port.h
 * @brief   Khai báo hàm Port Init dùng chung cho PWM exmaple
 * @details Hàm này cấu hình chân GPIO cho PWM theo cấu hình hiện tại:
 *          - TIM2_CH1 -> PA0
 *          - TIM3_CH2 -> PA7
 * @version 1.0
 * @date    2026-03-21
 * @author  HALA Academy
 **********************************************************/

#ifndef PWM_EXAMPLE_PORT_H
#define PWM_EXAMPLE_PORT_H

void Pwm_Example_PortInit(void);

#endif /* PWM_EXAMPLE_PORT_H */
