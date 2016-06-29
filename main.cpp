#include "mbed.h"

#define DEBUG 1
#ifdef DEBUG
#define DBG(...) printf("" __VA_ARGS__)
#else
#define DBG(...)
#endif


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

bool hallstatus[3];
InterruptIn hall1(p15);
InterruptIn hall2(p16);
InterruptIn hall3(p17);

AnalogIn throttle(p19); // throttle is 0.0-1.0 float value. throttle value is read from some analog read pin.

Ticker tim;
InterruptIn SW1(p12);  // up frequency pin.
InterruptIn SW2(p14);  // down frequency pin.
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
    if (hallstatus[0]) HallVal += 1;
    if (hallstatus[1]) HallVal += 2;
    if (hallstatus[2]) HallVal += 4;
    DBG("bldcval = %d\r\n", HallVal);

    switch (HallVal) {
        // step-1
        case 5:
            DBG("Step-1\r\n");
            pwmu_1 = 0.0;
            pwmu_2 = 1.0;
            
            pwmv_1 = 1.0;
            pwmv_2 = 0.0;
            
            pwmw_1 = throttle;
            pwmw_2 = 0.0;
        break;

        // step-2
        case 1:
            DBG("Step-2\r\n");
            pwmu_1 = throttle;
            pwmu_2 = 0.0;
            
            pwmv_1 = 1.0;
            pwmv_2 = 0.0;
            
            pwmw_1 = 0.0;
            pwmw_2 = 1.0;
        break;

        // step-3
        case 3:
            DBG("Step-3\r\n");
            pwmu_1 = throttle;
            pwmu_2 = 0.0;
            
            pwmv_1 = 0.0;
            pwmv_2 = 1.0;
            
            pwmw_1 = 1.0;
            pwmw_2 = 0.0;
        break;

        // step-4
        case 2:
            DBG("Step-4\r\n");
            pwmu_1 = 0.0;
            pwmu_2 = 1.0;
            
            pwmv_1 = throttle;
            pwmv_2 = 0.0;
            
            pwmw_1 = 1.0;
            pwmw_2 = 0.0;
        break;

        // step-5
        case 6:
            DBG("Step-5\r\n");
            pwmu_1 = 1.0;
            pwmu_2 = 0.0;
            
            pwmv_1 = throttle;
            pwmv_2 = 0.0;
            
            pwmw_1 = 0.0;
            pwmw_2 = 1.0;
        break;

        // step-6
        case 4:
            DBG("Step-6\r\n");
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
    DBG("PushUp\r\n");
}

void pushDown(){
    freq -= FREQ_SIZE;
    if (freq <= MIN_FREQ) freq = MIN_FREQ;

    dx = 1.0/(DX_LEN * freq);
    tim.detach();
    tim.attach(&bldcval, dx);
    cled = 0;
    DBG("PushDown\r\n");
}

// hall sensor check interrupt functions.
void hall1interrupt_rise() {
    hallstatus[0] = true;
    DBG("hall 1 high\r\n");
}
void hall1interrupt_fall() {
    hallstatus[0] = false;
    DBG("hall 1 low\r\n");
}
void hall2interrupt_rise() {
    hallstatus[1] = true;
    DBG("hall 2 high\r\n");
}
void hall2interrupt_fall() {
    hallstatus[1] = false;
    DBG("hall 2 low\r\n");
}
void hall3interrupt_rise() {
    hallstatus[2] = true;
    DBG("hall 3 high\r\n");
}
void hall3interrupt_fall() {
    hallstatus[2] = false;
    DBG("hall 3 low\r\n");
}

int main() {
    // output pwm value
    pwmu_1.period_us(PWM_T_TIME);
    pwmv_1.period_us(PWM_T_TIME);
    pwmw_1.period_us(PWM_T_TIME);
    
    pwmu_2.period_us(PWM_T_TIME);
    pwmv_2.period_us(PWM_T_TIME);
    pwmw_2.period_us(PWM_T_TIME);
    
    // hall sensor status
    hallstatus[0] = hallstatus[1] = hallstatus[2] = false;
    hall1.rise(&hall1interrupt_rise);
    hall1.fall(&hall1interrupt_fall);
    hall2.rise(&hall2interrupt_rise);
    hall2.fall(&hall2interrupt_fall);
    hall3.rise(&hall3interrupt_rise);
    hall3.fall(&hall3interrupt_fall);

    // frequency controll
    dx = 1.0/(DX_LEN * freq);
    SW1.rise(&pushUp);
    SW2.rise(&pushDown);
    tim.attach(&bldcval, dx);
    while(1) {
        wait(1);
    }
}

