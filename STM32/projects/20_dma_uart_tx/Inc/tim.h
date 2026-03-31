#ifndef TIM_H_
#define TIM_H_

// Update interrupt flag
#define SR_UIF 		(1U << 0)

// Input captured flag
#define SR_CC1IF	(1U << 1)

void tim2_1hz_init(void);
void tim2_1hz_interrupt_init(void);
void tim2_pa5_output_compare(void);
void tim3_pa6_input_capture(void);

#endif /* TIM_H_ */
