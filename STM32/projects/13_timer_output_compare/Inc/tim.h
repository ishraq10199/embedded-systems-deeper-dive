#ifndef TIM_H_
#define TIM_H_

// Update interrupt flag
#define SR_UIF (1U << 0)

void tim2_1hz_init(void);
void tim2_output_compare(void);

#endif /* TIM_H_ */
