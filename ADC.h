/*
 * ADC_HEAD.h
 *
 *  Created on: 2017��7��18��
 *      Author: w
 */

#ifndef ADC_H_
#define ADC_H_

void ADS1118_GPIO_Init(void);
void ADS1118_SPI_Init(void);
void ADS1118_ADS_Config(signed int temp_config_value);
int ADS1118_ADS_Read(void);
signed int ADS1118_WriteSPI(unsigned int config, unsigned char mode);
float ADC(void);


#endif /* ADC_H_ */
