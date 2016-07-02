#include "mbed.h"

#define DEBUG
#ifdef DEBUG
#define DBG(...) printf("" __VA_ARGS__)
#else
#define DBG(...)
#endif

// #define HALL_LED_CHECK

#define DX_LEN 20
#define PWM_T_TIME 10

#define FREQ_SIZE 1
#define MIN_FREQ 1
#define MAX_FREQ 1000

PwmOut pwmu_p(p21);
PwmOut pwmu_n(p22);

PwmOut pwmv_p(p23);
PwmOut pwmv_n(p24);

PwmOut pwmw_p(p25);
PwmOut pwmw_n(p26);

// AnalogIn throttle(p19); // throttle is 0.0-1.0 float value. throttle value is read from some analog read pin.
float throttle = 1.0;

Ticker tim;
InterruptIn SW1(p12);  // up frequency pin.
InterruptIn SW2(p14);  // down frequency pin.
InterruptIn SW3(p10);  // down frequency pin.
int freq = MIN_FREQ;   // running frequency value.
float dx = 100;
bool is_initialized;
bool start_motor = false;

DigitalOut cled(LED1); // test led pin.

// for float value mapping function
float valuetomap(float x, float in_min, float in_max, float out_min ,float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

int HallVal = 1;

void bldcval(){
if (start_motor) {
    HallVal = HallVal + 1;
    if (HallVal >= 7) {
        HallVal = 1;
    }

    
    switch (HallVal) {
        // step-1
        case 1:
            pwmu_p = 0.0;
            pwmu_n = 0.0;
            
            pwmv_p = 0.0;
            pwmv_n = 1.0;
            
            pwmw_p = throttle;
            pwmw_n = 0.0;
            printf("step-1\r\n");
        break;

        // step-2
        case 2:
            pwmu_p = throttle;
            pwmu_n = 0.0;
            
            pwmv_p = 0.0;
            pwmv_n = 1.0;
            
            pwmw_p = 0.0;
            pwmw_n = 0.0;
            printf("step-2\r\n");
        break;

        // step-3
        case 3:
            pwmu_p = throttle;
            pwmu_n = 0.0;
            
            pwmv_p = 0.0;
            pwmv_n = 0.0;
            
            pwmw_p = 0.0;
            pwmw_n = 1.0;
            printf("step-3\r\n");
        break;

        // step-4
        case 4:
            pwmu_p = 0.0;
            pwmu_n = 0.0;
            
            pwmv_p = throttle;
            pwmv_n = 0.0;
            
            pwmw_p = 0.0;
            pwmw_n = 1.0;
            printf("step-4\r\n");
        break;

        // step-5
        case 5:
            pwmu_p = 0.0;
            pwmu_n = 1.0;
            
            pwmv_p = throttle;
            pwmv_n = 0.0;
            
            pwmw_p = 0.0;
            pwmw_n = 0.0;
            printf("step-5\r\n");
        break;

        // step-6
        case 6:
            pwmu_p = 0.0;
            pwmu_n = 1.0;
            
            pwmv_p = 0.0;
            pwmv_n = 0.0;
            
            pwmw_p = throttle;
            pwmw_n = 0.0;
            printf("step-6\r\n");
        break;
    }
}
}

void initialize() {
    if (!is_initialized) {
        pwmu_n = 1.0;
        pwmv_n = 1.0;
        pwmw_n = 1.0;
        is_initialized = true;
        printf("initialize\r\n");
    } else {
        pwmu_n = 0.0;
        pwmv_n = 0.0;
        pwmw_n = 0.0;
        start_motor = true;
        printf("already initialized\r\n");
    }
}

int main() {
    // output pwm value
    pwmu_p.period_us(PWM_T_TIME);
    pwmv_p.period_us(PWM_T_TIME);
    pwmw_p.period_us(PWM_T_TIME);
    
    pwmu_n.period_us(PWM_T_TIME);
    pwmv_n.period_us(PWM_T_TIME);
    pwmw_n.period_us(PWM_T_TIME);
    

    // initialize 
    is_initialized = false;
    SW3.rise(&initialize);
    
    while(1) {
        bldcval();
        wait(1);
    }
}
