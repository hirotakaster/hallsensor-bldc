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

bool hallstatus[3];
InterruptIn hall1(p15);
InterruptIn hall2(p16);
InterruptIn hall3(p17);

// AnalogIn throttle(p19); // throttle is 0.0-1.0 float value. throttle value is read from some analog read pin.
float throttle = 1.0;

Ticker tim;
InterruptIn SW1(p12);  // up frequency pin.
InterruptIn SW2(p14);  // down frequency pin.
InterruptIn SW3(p10);  // down frequency pin.
int freq = MIN_FREQ;   // running frequency value.
float dx = 0;
bool is_initialized;

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
    // DBG("bldcval = %d\r\n", HallVal);

    switch (HallVal) {
        // step-1
        case 1:
            pwmu_p = 0.0;
            pwmu_n = 1.0;
            
            pwmv_p = 1.0;
            pwmv_n = 0.0;
            
            pwmw_p = throttle;
            pwmw_n = 0.0;
            // DBG("step-1\r\n");
        break;

        // step-2
        case 3:
            pwmu_p = throttle;
            pwmu_n = 0.0;
            
            pwmv_p = 1.0;
            pwmv_n = 0.0;
            
            pwmw_p = 0.0;
            pwmw_n = 1.0;
            // DBG("step-2\r\n");
        break;

        // step-3
        case 7:
            pwmu_p = throttle;
            pwmu_n = 0.0;
            
            pwmv_p = 0.0;
            pwmv_n = 1.0;
            
            pwmw_p = 1.0;
            pwmw_n = 0.0;
            // DBG("step-3\r\n");
        break;

        // step-4
        case 6:
            pwmu_p = 0.0;
            pwmu_n = 1.0;
            
            pwmv_p = throttle;
            pwmv_n = 0.0;
            
            pwmw_p = 1.0;
            pwmw_n = 0.0;
            // DBG("step-4\r\n");
        break;

        // step-5
        case 4:
            pwmu_p = 1.0;
            pwmu_n = 0.0;
            
            pwmv_p = throttle;
            pwmv_n = 0.0;
            
            pwmw_p = 0.0;
            pwmw_n = 1.0;
            // DBG("step-5\r\n");
        break;

        // step-6
        case 0:
            pwmu_p = 1.0;
            pwmu_n = 0.0;
            
            pwmv_p = 0.0;
            pwmv_n = 1.0;
            
            pwmw_p = throttle;
            pwmw_n = 0.0;
            // DBG("step-6\r\n");
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
    DBG("now : %d\r\n", freq);
}

void pushDown(){
    freq -= FREQ_SIZE;
    if (freq <= MIN_FREQ) freq = MIN_FREQ;

    dx = 1.0/(DX_LEN * freq);
    tim.detach();
    tim.attach(&bldcval, dx);
    cled = 0;
    DBG("now : %d\r\n", freq);
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
        printf("already initialized\r\n");
    }
}
    
// hall sensor check interrupt functions.
void hall1interrupt_rise() {
    hallstatus[0] = true;
#ifdef HALL_LED_CHECK
    pwmu_p = 1.0;
    pwmu_n = 0.0;
    DBG("hall 1 high\r\n");
#endif
}
void hall1interrupt_fall() {
    hallstatus[0] = false;
#ifdef HALL_LED_CHECK
    pwmu_p = 0.0;
    pwmu_n = 1.0;
    DBG("hall 1 low\r\n");
#endif
}
void hall2interrupt_rise() {
    hallstatus[1] = true;
#ifdef HALL_LED_CHECK
    pwmv_p = 1.0;
    pwmv_n = 0.0;
    DBG("hall 2 high\r\n");
#endif
}
void hall2interrupt_fall() {
    hallstatus[1] = false;
#ifdef HALL_LED_CHECK
    pwmv_p = 0.0;
    pwmv_n = 1.0;
    DBG("hall 2 low\r\n");
#endif       
}
void hall3interrupt_rise() {
    hallstatus[2] = true;
#ifdef HALL_LED_CHECK
    pwmw_p = 1.0;
    pwmw_n = 0.0;
    DBG("hall 3 high\r\n");
#endif
}
void hall3interrupt_fall() {
    hallstatus[2] = false;
#ifdef HALL_LED_CHECK
    pwmw_p = 0.0;
    pwmw_n = 1.0;
    DBG("hall 3 low\r\n");
#endif
}

int main() {
    // output pwm value
    pwmu_p.period_us(PWM_T_TIME);
    pwmv_p.period_us(PWM_T_TIME);
    pwmw_p.period_us(PWM_T_TIME);
    
    pwmu_n.period_us(PWM_T_TIME);
    pwmv_n.period_us(PWM_T_TIME);
    pwmw_n.period_us(PWM_T_TIME);
    
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
    
    // initialize 
    is_initialized = false;
    SW3.rise(&initialize);
    
    while(1) {
        wait(1);
    }
}
