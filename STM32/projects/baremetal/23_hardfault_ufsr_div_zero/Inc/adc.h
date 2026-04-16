#ifndef ADC_H_
#define ADC_H_

#define SR_EOC	(1U << 1)

void pa1_adc_single_init(void);
void pa1_adc_interrupt_init(void);
void pa1_adc_continuous_init(void);
void adc_start_single_conversion(void);
void adc_start_continuous_conversion(void);
uint32_t adc_read(void);

#endif /* ADC_H_ */
