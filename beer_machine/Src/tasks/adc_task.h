
#define  ADC_TASK_ADC_VALUE_MAX                (4095)
#define  ADC_TASK_ADC_VALUE_MIN                (0)

#define  ADC_TASK_ADC_ERR_MAX                  (100)
#define  ADC_TASK_ADC_ERR_VALUE                (0xFFFF)


#define  ADC_TASK_ADC_SAMPLE_MAX               48
#define  ADC_TASK_TEMPERATURE_IDX              0
#define  ADC_TASK_PRESSURE_IDX                 1

#define  ADC_TASK_INTERVAL                     10
#define  ADC_TASK_ADC_TIMEOUT                  5  

#define  ADC_TASK_ADC_COMPLETED_SIGNAL         (1<<0)
#define  ADC_TASK_ADC_ERROR_SIGNAL             (1<<1)
#define  ADC_TASK_ALL_SIGNALS                  ((1<<2)-1)