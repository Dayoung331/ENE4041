#include "msp.h"
#include "Clock.h"
#include <stdio.h>

#define LED_RED 1
#define LED_GREEN (LED_RED << 1)
#define LED_BLUE  (LED_RED << 2)

void led_init()
{
    P2->SEL0 &= ~0x07;
    P2->SEL1 &= ~0X07;
    P2->DIR |= 0X07;
    P2->OUT &= ~0X07;
}
void turn_on_led(int color)
{
    P2->OUT |= ~0b00000001;
    P2->OUT |= color;
}

void turn_off_led()
{
    P2->OUT &= ~0b11111110;
}

void switch_init()
{
    P1->SEL0 &= ~0x12;
    P1->SEL1 &= ~0x12;

    P1->DIR &= ~0x12;

    P1->REN |= 0x12;

    P1->OUT |= 0x12;
}

void systick_init(void)
{
    SysTick->LOAD = 0x00FFFFFF;
    SysTick->CTRL = 0x00000005;
}

void systick_wait1ms()
{
    SysTick->LOAD = 47999;
    SysTick->VAL = 0;
    while ((SysTick->CTRL & 0x00010000) == 0)
    {
    };
}

void systick_wait_N_ms()
{
    int i;
    int count = 60;

    for (i = 0; i < count; i++)
    {
        systick_wait1ms();
    }
}
void systick_wait1s()
{
    int i;
    int count = 1000;

    for (i = 0; i < count; i++)
    {
        systick_wait1ms();
    }
}

void sensor_init()
{
    P5->SEL0 &= ~0x08;
    P5->SEL1 &= ~0x08;
    P5->DIR |= 0x08;
    P5->OUT &= ~0x08;

    P9->SEL0 &= ~0x04;
    P9->SEL1 &= ~0x04;
    P9->DIR |= 0x04;
    P9->OUT &= ~0x04;

    P7->SEL0 &= ~0xFF;
    P7->SEL1 &= ~0xFF;
    P7->DIR &= ~0xFF;
}

void pwm_init34(uint16_t period, uint16_t duty3, uint16_t duty4)
{
    TIMER_A0->CCR[0] = period;

    TIMER_A0->EX0 = 0x0000;

    TIMER_A0->CCTL[3] = 0x0040;
    TIMER_A0->CCR[3] = duty3;
    TIMER_A0->CCTL[4] = 0x0040;
    TIMER_A0->CCR[4] = duty4;

    TIMER_A0->CTL = 0x02F0;

    P2->DIR |= 0xC0;
    P2->SEL0 |= 0xC0;
    P2->SEL1 &= ~0xC0;
}

void motor_init(void)
{
    P3->SEL0 &= ~0xC0;
    P3->SEL1 &= ~0xC0;
    P3->DIR |= 0xC0;
    P3->OUT &= ~0xC0;

    P5->SEL0 &= ~0x30;
    P5->SEL1 &= ~0X30;
    P5->DIR |= 0x30;
    P5->OUT &= ~0x30;

    P2->SEL0 &= ~0xC0;
    P2->SEL1 &= ~0xC0;
    P2->DIR |= 0xC0;
    P2->OUT &= ~0xC0;

    pwm_init34(7500, 0, 0);
}

void move(uint16_t leftDuty, uint16_t rightDuty)
{
    P3->OUT |= 0xC0;
    TIMER_A0->CCR[3] = leftDuty;
    TIMER_A0->CCR[4] = rightDuty;
}

void left_forward()
{
    P5->OUT &= ~0x10;
}

void left_backward()
{
    P5->OUT |= 0x10;
}

void right_forward()
{
    P5->OUT &= ~0x20;
}

void right_backward()
{
    P5->OUT |= 0x20;
}

void (*TimerA2Task)(void);

void TimerA2_Init(void (*task)(void), uint16_t period)
{
    TimerA2Task = task;
    TIMER_A2->CTL = 0x0280;
    TIMER_A2->CCTL[0] = 0x0010;
    TIMER_A2->CCR[0] = (period - 1);
    TIMER_A2->EX0 = 0X0005;
    NVIC->IP[3] = (NVIC->IP[3] & 0xFFFFFF00) | 0x00000040;
    NVIC->ISER[0] = 0x00001000;
    TIMER_A2->CTL |= 0x0014;
}

void TA2_0_IRQHandler(void)
{
    TIMER_A2->CCTL[0] &= ~0x0001;
    (*TimerA2Task)();
}

void timer_A3_capture_init()
{
    P10->SEL0 |= 0x30;
    P10->SEL1 &= ~0x30;
    P10->DIR &= ~0x30;

    TIMER_A3->CTL &= ~0x0030;
    TIMER_A3->CTL = 0x0200;

    TIMER_A3->CCTL[0] = 0x4910;
    TIMER_A3->CCTL[1] = 0x4910;
    TIMER_A3->EX0 &= ~0x0007;

    NVIC->IP[3] = (NVIC->IP[3] & 0x0000FFFF) | 0x40400000;
    NVIC->ISER[0] = 0x0000C000;
    TIMER_A3->CTL |= 0x0024;
}

uint16_t first_left;
uint16_t first_right;

uint16_t period_left;
uint16_t period_right;

uint32_t left_count;

void TA3_0_IRQHandler(void)
{
    TIMER_A3->CCTL[0] &= ~0x0001;
    period_right = TIMER_A3->CCR[0] - first_right;
    first_right = TIMER_A3->CCR[0];
}

void TA3_N_IRQHandler(void)
{
    TIMER_A3->CCTL[1] &= ~0x0001;
    period_left = TIMER_A3->CCR[1] - first_left;
    first_left = TIMER_A3->CCR[1];
    left_count++;
}

uint32_t get_left_rpm()
{
    return 2000000 / period_left;
}

int sensor1;
int sensor2;
int sensor3;
int sensor4;
int sensor5;
int sensor6;
int sensor7;
int sensor8;

int goal1 = 1;
int goal2 = 0;
int goal3 = 0;
int goal4 = 0;
int goal5 = 0;
int goal6 = 0;
int goal7 = 0;

void IR_LED_charge()
{
    P5->OUT |= 0x08;
    P9->OUT |= 0x04;

    P7->DIR = 0xFF;
    P7->OUT = 0xFF;

    Clock_Delay1us(10);
    P7->DIR = 0x00;

    Clock_Delay1us(1200);

    sensor1 = P7->IN & 0x01;
    sensor2 = P7->IN & 0x02;
    sensor3 = P7->IN & 0x04;
    sensor4 = P7->IN & 0x08;
    sensor5 = P7->IN & 0x10;
    sensor6 = P7->IN & 0x20;
    sensor7 = P7->IN & 0x40;
    sensor8 = P7->IN & 0x80;
}

void little_bit_forward()
{
    left_count = 0;

    while (1)
    {
        left_forward();
        right_forward();
        move(1000, 1000);

        if (left_count > 5)
        {
            break;
        }
    }
}

void little_bit_backward()
{
    left_count = 0;

    while (1)
    {
        left_backward();
        right_backward();
        move(1000, 1000);

        if (left_count > 5)
        {
            break;
        }
    }
}

// move forward with speed in ncm
void move_ncm_forward(int n, int speed)
{
    left_count = 0;

    while (1)
    {
        left_forward();
        right_forward();
        move(speed, speed);

        // 8cm / (3.14 * 7cm) * 360 = 131
        int count = n / (3.14 * 7) * 360;
        if (left_count > count)
        {
            break;
        }
    }

}

// move backward with speed in ncm
void move_ncm_backward(int n, int speed)
{
    left_count = 0;

    while (1)
    {
        left_backward();
        right_backward();
        move(speed, speed);

        int count = n / (3.14 * 7) * 360;
        if (left_count > count)
        {
            break;
        }
    }
}

// turn left in n degrees
void rotate_left(int n)
{
    left_count = 0;
    while (1)
    {
        left_backward();
        right_forward();
        move(1000, 1000);

        if (left_count > 2 * n)
        {
            break;
        }
    }
}

// turn right in n degrees
void rotate_right(int n)
{
    left_count = 0;
    while (1)
    {
        left_forward();
        right_backward();
        move(1000, 1000);

        if (left_count > 2 * n)
        {
            break;
        }
    }
}

// code for adjusting position
void adjust_position()
{
    IR_LED_charge();
    if (sensor3)
    {
        while (1)
        {
            IR_LED_charge();

            left_forward();
            right_backward();

            move(1000, 1000);

            if (sensor4)
            {
                break;
            }
        }
    }

    else if (sensor6)
    {
        while (1)
        {
            IR_LED_charge();

            left_backward();
            right_forward();

            move(1000, 1000);

            if (sensor5)
            {
                break;
            }
        }
    }
}

void main(void)
{
    Clock_Init48MHz();
    systick_init();
    led_init();
    switch_init();
    sensor_init();
    motor_init();
    timer_A3_capture_init();

    while (1)
    {
        IR_LED_charge();

        // goal1
        if (goal1 && sensor1 && !sensor2 && !sensor3 && sensor4 && sensor5
                && !sensor6 && !sensor7 && sensor8)
        {
            move(0, 0);
            Clock_Delay1ms(3000);

            // to get a right position
            move_ncm_forward(4, 1000);

            goal1 = 0;
            goal2 = 1;
            goal3 = 0;
            goal4 = 0;
            goal5 = 0;
            goal6 = 0;
            goal7 = 0;
        }

        //goal2
        else if (goal2 && sensor2 && sensor4 && sensor5 && sensor7 && !sensor1
                && !sensor3 && !sensor6 && !sensor8)
        {
            rotate_left(3);

            // move forward and backward for 8cm
            move_ncm_forward(8, 1000);
            move(0, 0);
            Clock_Delay1ms(300);

            move_ncm_backward(8, 1000);
            move(0, 0);
            Clock_Delay1ms(300);

            move_ncm_forward(8, 1000);
            move(0, 0);
            Clock_Delay1ms(300);

            move_ncm_backward(8, 1000);
            move(0, 0);
            Clock_Delay1ms(300);

            move_ncm_forward(8, 1000);

            // adjusting position
            move_ncm_forward(1, 1000);

            goal1 = 0;
            goal2 = 0;
            goal3 = 1;
            goal4 = 0;
            goal5 = 0;
            goal6 = 0;
            goal7 = 0;
        }

        //goal3
        else if (goal3 && sensor1 && !sensor2 && sensor4 && sensor5 && !sensor7
                && !sensor8)
        {
            rotate_right(9);

            int i = 6;

            for (i = 6; i > 0; i--)
            {
                move_ncm_forward(1, 800);
                adjust_position();
            }

            for (i = 7; i > 0; i--)
            {
                move_ncm_forward(1, 650);
                adjust_position();
            }

            for (i = 6; i > 0; i--)
            {
                move_ncm_forward(1, 500);
                adjust_position();
            }

            move(1000, 1000);
            Clock_Delay1ms(10);

            goal1 = 0;
            goal2 = 0;
            goal3 = 0;
            goal4 = 1;
            goal5 = 0;
            goal6 = 0;
            goal7 = 0;
        }

        //goal4
        else if (goal4 && !sensor1 && !sensor2 && sensor4 && sensor5 && !sensor7
                && sensor8)
        {
            rotate_right(20);

            int i = 0;

            for (i = 8; i > 0; i--)
            {
                move_ncm_forward(1, 1500);
                adjust_position();
            }

            for (i = 8; i > 0; i--)
            {
                move_ncm_forward(1, 2000);
                adjust_position();
            }
            for (i = 8; i > 0; i--)
            {
                move_ncm_forward(1, 1500);
                adjust_position();
            }

            move(1000, 1000);
            Clock_Delay1ms(10);

            goal1 = 0;
            goal2 = 0;
            goal3 = 0;
            goal4 = 0;
            goal5 = 1;
            goal6 = 0;
            goal7 = 0;
        }

        //goal5
        if (goal5 && !sensor1 && !sensor2 && !sensor3 && sensor4 && sensor5
                && !sensor6 && sensor7 && !sensor8)
        {
            //turn left in 360 degrees
            rotate_left(340);
            while (1)
            {
                IR_LED_charge();

                rotate_left(1);

                if (!sensor1 && !sensor2 && !sensor3 && sensor4 && sensor5
                        && !sensor6 && !sensor8)
                {
                    break;
                }
            }
            rotate_left(5);

            //move a little bit
            move_ncm_forward(3, 700);

            move(0, 0);
            Clock_Delay1ms(100);

            //turn right in 360 degrees
            rotate_right(340);
            while (1)
            {
                IR_LED_charge();

                rotate_right(1);

                if (!sensor1 && !sensor2 && !sensor3 && sensor4 && sensor5
                        && !sensor6 && sensor7 && !sensor8)
                {
                    break;
                }
            }

            move(0, 0);
            Clock_Delay1ms(100);

            //turn left in 360 degrees
            rotate_left(340);
            while (1)
            {
                IR_LED_charge();

                rotate_left(1);

                if (!sensor1 && !sensor2 && !sensor3 && sensor4 && sensor5
                        && !sensor6 && sensor7 && !sensor8)
                {
                    break;
                }
            }
            rotate_left(9);

            goal1 = 0;
            goal2 = 0;
            goal3 = 0;
            goal4 = 0;
            goal5 = 0;
            goal6 = 1;
            goal7 = 0;

        }

        // goal6
        else if (goal6 && !sensor1 && sensor2 && !sensor3 && sensor4 && sensor5
                && !sensor6 && !sensor7 && !sensor8)
        {
            move_ncm_forward(13, 1000);

            rotate_left(175);

            move_ncm_forward(8, 1000);

            rotate_right(160);
            while (1)
            {
                IR_LED_charge();

                rotate_right(1);

                if (!sensor1 && sensor2 && !sensor3 && sensor4 && sensor5
                        && !sensor6 && !sensor7 && !sensor8)
                {
                    break;
                }
            }
            rotate_right(5);

            goal1 = 0;
            goal2 = 0;
            goal3 = 0;
            goal4 = 0;
            goal5 = 0;
            goal6 = 0;
            goal7 = 1;
        }

        // goal7
        else if ((goal7 && !sensor1 && sensor3 && sensor4 && sensor5 && sensor6
                && !sensor8)
                || (goal7 && !sensor1 && sensor4 && sensor5 && sensor6
                        && !sensor8)
                || (goal7 && !sensor1 && sensor3 && sensor4 && sensor5
                        && !sensor8))
        {
            //stop
            P2->OUT &= ~0xC0;
            // motor stop
            move(0, 0);
            break;
        }

        /* code for finding track */
        // go straight
        else if (sensor4 && sensor5 && (!sensor3) && (!sensor6))
        {
            left_forward();
            right_forward();
            move(1000, 1000);
        }

        // left 90 degree
        else if (sensor7 && sensor8 && !sensor1)
        {
            left_forward();
            right_forward();
            move(1000, 1000);

            //90 degree turn
            left_count = 0;
            move(1000, 1000);
            while (1)
            {
                IR_LED_charge();

                right_forward();
                left_backward();

                if (sensor3)
                {
                    int i;
                    for (i = left_count; i == 0; i--)
                    {
                        right_backward();
                        left_forward();
                        move(1000, 1000);
                    }
                    left_count = 0;
                    break;
                }
            }

        }

        // right 90 degree
        else if (sensor1 && sensor2 && !sensor8)
        {
            left_forward();
            right_forward();
            move(1000, 1000);

            int i;
            int count = 100;
            for (i = 0; i < count; i++)
            {
                systick_wait1ms();
            }

            //90 degree turn
            left_count = 0;
            move(1000, 1000);
            while (1)
            {
                IR_LED_charge();

                left_forward();
                right_backward();

                if (sensor6)
                {
                    for (i = left_count; i == 0; i--)
                    {
                        right_forward();
                        left_backward();
                        move(1000, 1000);

                    }
                    left_count = 0;
                    break;
                }
            }
        }

        //being skewed right -> turn left a little bit
        else if (sensor4)
        {
            if (sensor3)
            {
                int count = 0;
                while (1)
                {
                    IR_LED_charge();

                    left_forward();
                    right_backward();

                    move(1000, 1000);

                    if (!sensor1 && !sensor2 && sensor3 && sensor4 && sensor5
                            && sensor6 && !sensor7 && !sensor8)
                    {
                        left_backward();
                        right_forward();

                        int i;
                        for (i = count; i == 0; i--)
                        {
                        }
                        little_bit_forward();

                        break;
                    }
                    else if (!sensor3)
                    {
                        break;
                    }
                    count++;
                }
            }
        }

        //being skewed left -> turn right a little bit
        else if (sensor5)
        {
            if (sensor6)
            {
                int count = 0;
                while (1)
                {
                    IR_LED_charge();

                    left_backward();
                    right_forward();

                    move(1000, 1000);

                    if (!sensor1 && !sensor2 && sensor3 && sensor4 && sensor5
                            && sensor6 && !sensor7 && !sensor8)
                    {
                        left_forward();
                        right_backward();

                        int i;
                        for (i = count; i == 0; i--)
                        {
                        }
                        little_bit_forward();
                        break;
                    }
                    else if (!sensor6)
                    {
                        break;
                    }
                    count++;
                }
            }
        }

        //tuning for goal3 or goal4
        else if (sensor3)
        {
            while (1)
            {
                IR_LED_charge();

                left_forward();
                right_backward();

                move(1000, 1000);

                if (sensor4)
                {
                    break;
                }
            }
        }

        else if (sensor6)
        {
            while (1)
            {
                IR_LED_charge();

                left_backward();
                right_forward();

                move(1000, 1000);

                if (sensor5)
                {
                    break;
                }
            }
        }

        //no signal
        else
        {
            // motor stop
            move(0, 0);
        }
    }
}
