#include "motor_control.h"

// === 타이머 및 GPIO 외부 참조 ===
extern TIM_HandleTypeDef htim1; // DC 모터 PWM 타이머
extern TIM_HandleTypeDef htim2; // 서보 모터 PWM 타이머

// === private 매크로 ===
#define MOTOR_FORWARD()     do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); } while (0)
#define MOTOR_BACKWARD()    do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); } while (0)


void MotorControl_Init(void)
{
    // PWM 타이머 시작
    HAL_TIM_PWM_Start(&htim2, TIM_CHANNEL_3);
    HAL_TIM_PWM_Start(&htim1, TIM_CHANNEL_4);

    // 초기 방향 설정
    MOTOR_FORWARD();
}

void MotorControl_Update(const VehicleCommand_t* command)
{
	Update_MotorDirection((MotorDirection_t)command->direction);
    Control_Servo(command->roll);
    Control_DcMotor(command->accel_ms, command->brake_ms);
}

// 서보 모터 제어 함수
void Control_Servo(float roll)
{
    if (roll < -90.0f) roll = -90.0f;
    if (roll >  90.0f) roll =  90.0f;
    float angle = roll + 90.0f;
    uint16_t pwm_servo = (uint16_t)(1200 + (angle / 180.0f) * (1900 - 1200));
    if (pwm_servo < 1200) pwm_servo = 1200;
    if (pwm_servo > 1900) pwm_servo = 1900;
    __HAL_TIM_SET_COMPARE(&htim2, TIM_CHANNEL_3, pwm_servo);
}

// DC 모터 제어 함수
void Control_DcMotor(uint16_t accel_ms, uint16_t brake_ms)
{
    static int16_t duty = 0;
    const int16_t accel_step = 30;
    const int16_t brake_step = 100;
    const int16_t max_duty = 930;

    if (brake_ms > 0) {
        duty -= brake_step;
    } else if (accel_ms > 0) {
        if (duty == 0) duty = 300;
        duty += accel_step;
    } else if (accel_ms == 0) {
        duty -= accel_step;
    }

    if (duty < 0) duty = 0;
    if (duty > max_duty) duty = max_duty;
    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, duty);
}

// 모터 방향 제어 함수
void Update_MotorDirection(MotorDirection_t direction)
{
    static MotorDirection_t prev_direction = DIRECTION_FORWARD;
    if (direction != prev_direction) {
        prev_direction = direction;
        if (direction == DIRECTION_FORWARD) {
            MOTOR_FORWARD();
        } else {
            MOTOR_BACKWARD();
        }
    }
}
