#include "mbed.h"

#define DX_LEN 20
#define PWM_T_TIME 100

#define FREQ_SIZE 10
#define MIN_FREQ 10
#define MAX_FREQ 1000

PwmOut pwmu_1(p21);
PwmOut pwmu_2(p22);

PwmOut pwmv_1(p23);
PwmOut pwmv_2(p24);

PwmOut pwmw_1(p25);
PwmOut pwmw_2(p26);

bool holestatus[3];
InterruptIn hole1(p5);
InterruptIn hole2(p6);
InterruptIn hole3(p7);

AnalogIn throttle(p19); // throttle is 0.0-1.0 float value. throttle value is read from some analog read pin.

Ticker tim;
InterruptIn SW1(p13);  // up frequency pin.
InterruptIn SW2(p15);  // down frequency pin.
int freq = MIN_FREQ;   // running frequency value.
float dx = 0;

DigitalOut cled(LED1); // test led pin.

// for float value mapping function
float valuetomap(float x, float in_min, float in_max, float out_min ,float out_max) {
    return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

void bldcval(){
    // check hall sensor value
    int HallVal = 0;
    if (holestatus[0]) HallVal += 1;
    if (holestatus[1]) HallVal += 2;
    if (holestatus[2]) HallVal += 4;
    
    switch (HallVal) {
        // step-1
        case 5:
            pwmu_1 = 0.0;
            pwmu_2 = 1.0;
            
            pwmv_1 = 1.0;
            pwmv_2 = 0.0;
            
            pwmw_1 = throttle;
            pwmw_2 = 0.0;
        break;

        // step-2
        case 1:
            pwmu_1 = throttle;
            pwmu_2 = 0.0;
            
            pwmv_1 = 1.0;
            pwmv_2 = 0.0;
            
            pwmw_1 = 0.0;
            pwmw_2 = 1.0;
        break;

        // step-3
        case 3:
            pwmu_1 = throttle;
            pwmu_2 = 0.0;
            
            pwmv_1 = 0.0;
            pwmv_2 = 1.0;
            
            pwmw_1 = 1.0;
            pwmw_2 = 0.0;
        break;

        // step-4
        case 2:
            pwmu_1 = 0.0;
            pwmu_2 = 1.0;
            
            pwmv_1 = throttle;
            pwmv_2 = 0.0;
            
            pwmw_1 = 1.0;
            pwmw_2 = 0.0;
        break;

        // step-5
        case 6:
            pwmu_1 = 1.0;
            pwmu_2 = 0.0;
            
            pwmv_1 = throttle;
            pwmv_2 = 0.0;
            
            pwmw_1 = 0.0;
            pwmw_2 = 1.0;
        break;

        // step-6
        case 4:
            pwmu_1 = 1.0;
            pwmu_2 = 0.0;
            
            pwmv_1 = 0.0;
            pwmv_2 = 1.0;
            
            pwmw_1 = throttle;
            pwmw_2 = 0.0;
        break;
    }
}

void pushUp(){
    freq += FREQ_SIZE;
    if (freq >= MAX_FREQ) freq = MAX_FREQ;

    dx = 1.0/(DX_LEN * freq);
    tim.detach();
    tim.attach(&bldcval, dx);
    cled = 1;
}

void pushDown(){
    freq -= FREQ_SIZE;
    if (freq <= MIN_FREQ) freq = MIN_FREQ;

    dx = 1.0/(DX_LEN * freq);
    tim.detach();
    tim.attach(&bldcval, dx);
    cled = 0;
}

void hole1interrupt_rise() {
    holestatus[0] = true;
}
void hole1interrupt_fall() {
    holestatus[0] = false;
}
void hole2interrupt_rise() {
    holestatus[1] = true;
}
void hole2interrupt_fall() {
    holestatus[1] = false;
}
void hole3interrupt_rise() {
    holestatus[2] = true;
}
void hole3interrupt_fall() {
    holestatus[2] = false;
}

int main() {
    // output pwm value
    pwmu_1.period_us(PWM_T_TIME);
    pwmv_1.period_us(PWM_T_TIME);
    pwmw_1.period_us(PWM_T_TIME);
    
    pwmu_2.period_us(PWM_T_TIME);
    pwmv_2.period_us(PWM_T_TIME);
    pwmw_2.period_us(PWM_T_TIME);
    
    // hole sensor status
    holestatus[0] = holestatus[1] = holestatus[2] = false;
    hole1.rise(&hole1interrupt_rise);
    hole1.fall(&hole1interrupt_fall);
    hole2.rise(&hole2interrupt_rise);
    hole2.fall(&hole2interrupt_fall);
    hole3.rise(&hole3interrupt_rise);
    hole3.fall(&hole2interrupt_fall);

    // frequency controll
    dx = 1.0/(DX_LEN * freq);
    SW1.rise(&pushUp);
    SW2.rise(&pushDown);
    tim.attach(&bldcval, dx);
    while(1) {
        wait(1);
    }
}

