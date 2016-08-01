#include "mbed.h"

#define DEBUG
Serial pcout(SERIAL_TX, SERIAL_RX);
Serial cycon(PC_10, PC_11);

#ifdef DEBUG
#define DBG(...) pcout.printf("" __VA_ARGS__)
#else
#define DBG(...)
#endif

#define DX_LEN 1
#define PWM_PERIOD_TIME 15
#define PWM_PULSE_WIDTH 10

#define PWM_DEAD_UTIME 5

#define FREQ_SIZE 100
#define MIN_FREQ 500
#define MAX_FREQ 10000

PwmOut pwmu_p(PA_10);
PwmOut pwmu_n(PB_3);

PwmOut pwmv_p(PB_5);
PwmOut pwmv_n(PB_4);

PwmOut pwmw_p(PB_10);
PwmOut pwmw_n(PA_8);

DigitalIn hall1(PC_7);
DigitalIn hall2(PB_6);
DigitalIn hall3(PA_7);

InterruptIn startup(PA_6);
InterruptIn frequp(PA_5);
InterruptIn freqdow(PB_9);

DigitalOut powerselector(PC_8);

Ticker monitoring;
Ticker controller;

int freq = MIN_FREQ;   // running frequency value.
int rpmcounter;
float dx;
bool is_initialized;
bool start_motor;

bool hallonoff = false;

int HallVal = 0;
int lastHallVal = 0;

void bldcval(){
    
    if (start_motor) {

        HallVal = 0;
        if (hall1) { HallVal += 1; rpmcounter++;}
        if (hall2) HallVal += 2;
        if (hall3) HallVal += 4;
        // DBG("bldcval = %d\r\n", HallVal);
        
        // save hall val
        if (HallVal != lastHallVal) {
            lastHallVal = HallVal;
            // for dead time.
            pwmu_p = pwmu_n = pwmv_p = pwmv_n = pwmw_p = pwmw_n = 0.0;
            wait_us(PWM_DEAD_UTIME);
        }
        
        switch (HallVal) {
            // step-1
            case 5:
                pwmu_p = 0.0;
                pwmu_n = 0.0;
                
                pwmv_p = 0.0;
                pwmv_n = 1.0;
                
                pwmw_p = 1.0;
                pwmw_n = 0.0;
            break;
    
            // step-2
            case 1:
                pwmu_p = 1.0;
                pwmu_n = 0.0;
                
                pwmv_p = 0.0;
                pwmv_n = 1.0;
                
                pwmw_p = 0.0;
                pwmw_n = 0.0;
            break;
    
            // step-3
            case 3:
                pwmu_p = 1.0;
                pwmu_n = 0.0;
                
                pwmv_p = 0.0;
                pwmv_n = 0.0;
                
                pwmw_p = 0.0;
                pwmw_n = 1.0;
            break;
    
            // step-4
            case 2:
                pwmu_p = 0.0;
                pwmu_n = 0.0;
                
                pwmv_p = 1.0;
                pwmv_n = 0.0;
                
                pwmw_p = 0.0;
                pwmw_n = 1.0;
            break;
    
            // step-5
            case 6:
                pwmu_p = 0.0;
                pwmu_n = 1.0;
                
                pwmv_p = 1.0;
                pwmv_n = 0.0;
                
                pwmw_p = 0.0;
                pwmw_n = 0.0;
            break;
    
            // step-6
            case 4:
                pwmu_p = 0.0;
                pwmu_n = 1.0;
                
                pwmv_p = 0.0;
                pwmv_n = 0.0;
                
                pwmw_p = 1.0;
                pwmw_n = 0.0;
            break;
        }

    } else {
        pwmu_p = 0.0;
        pwmu_n = 0.0;
        
        pwmv_p = 0.0;
        pwmv_n = 0.0;
        
        pwmw_p = 0.0;
        pwmw_n = 0.0;
    }
}

void initialize() {
    if (!is_initialized) {
        pwmu_p = 0.0;
        pwmv_p = 0.0;
        pwmw_p = 0.0;
        
        pwmu_n = 1.0;
        pwmv_n = 1.0;
        pwmw_n = 1.0;
        is_initialized = true;
        wait(0.5);
        DBG("initialize\r\n");
    
    } else if (start_motor == false) {
        pwmu_p = 0.0;
        pwmv_p = 0.0;
        pwmw_p = 0.0;
        
        pwmu_n = 0.0;
        pwmv_n = 0.0;
        pwmw_n = 0.0;
        start_motor = true;
        powerselector = 1;
        wait(0.5);

        DBG("start motor\r\n");
    } else if (start_motor == true) {
        pwmu_p = 0.0;
        pwmv_p = 0.0;
        pwmw_p = 0.0;

        pwmu_n = 0.0;
        pwmv_n = 0.0;
        pwmw_n = 0.0;
        start_motor = false;
        powerselector = 0;
        wait(0.5);

        DBG("stop motor\r\n");
    }
}

void pushUp(){
    freq += FREQ_SIZE;
    if (freq >= MAX_FREQ) freq = MAX_FREQ;

    dx = 1.0/(DX_LEN * freq);
    controller.detach();
    controller.attach(&bldcval, dx);

    DBG("freq = %d\r\n", freq);
}

int main() {
    DBG("start main !!\r\n");
    
    // output pwm value
    pwmu_p.period_us(PWM_PERIOD_TIME);
    pwmv_p.period_us(PWM_PERIOD_TIME);
    pwmw_p.period_us(PWM_PERIOD_TIME);
    
    pwmu_n.period_us(PWM_PERIOD_TIME);
    pwmv_n.period_us(PWM_PERIOD_TIME);
    pwmw_n.period_us(PWM_PERIOD_TIME);    

    // initialize 
    is_initialized = false;
    start_motor = false;
    hallonoff = false;
    dx = 1.0/(DX_LEN * freq);
    
    startup.rise(&initialize);
    frequp.rise(&pushUp);
    controller.attach(&bldcval, dx);

    int timecnt = 0;
    while(1) {
        wait(1);
        
        // write logging   
        DBG("rpm = %d\r\n", (rpmcounter/20) * 60);
        cycon.printf("%d\r\n", (rpmcounter/20) * 60);
        rpmcounter = 0;
    }
}
