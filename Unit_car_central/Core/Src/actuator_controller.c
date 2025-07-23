// actuator_controller.c

#include "actuator_controller.h"

extern TIM_HandleTypeDef htim1;
extern TIM_HandleTypeDef htim2;

// 이 파일 안에서만 사용할 것이므로 static으로 선언
static TIM_HandleTypeDef* dc_motor_htim = &htim1;  // DC 모터 타이머
static uint32_t dc_motor_channel = TIM_CHANNEL_4;   // DC 모터 채널

static TIM_HandleTypeDef* servo_htim = &htim2;     // 서보 타이머
static uint32_t servo_channel = TIM_CHANNEL_3;      // 서보 채널

// === private 매크로 및 열거형 ===
typedef enum {
    DIRECTION_BACKWARD = 0,
    DIRECTION_FORWARD = 1
} MotorDirection_t;

#define MOTOR_FORWARD()     do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); } while (0)
#define MOTOR_BACKWARD()    do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); } while (0)


// === private 함수 프로토타입 ===
static void s_ControlServo(float roll);
static void s_ControlDcMotor(uint16_t accel_ms, uint16_t brake_ms);
static void s_UpdateMotorDirection(MotorDirection_t direction);


// === 공개 함수 구현 ===
void ActuatorController_Init(void)
{
    // PWM 시작
    HAL_TIM_PWM_Start(servo_htim, servo_channel);
    HAL_TIM_PWM_Start(dc_motor_htim, dc_motor_channel);

    // 초기 방향 설정
    MOTOR_FORWARD();
}

void ActuatorController_Update(const VehicleCommand_t* command)
{
    s_UpdateMotorDirection((MotorDirection_t)command->direction);
    s_ControlServo(command->roll);
    s_ControlDcMotor(command->accel_ms, command->brake_ms);
}


// === private(내부용) 함수 구현 ===

// 서보 모터 제어 함수 (기존 Control_Servo)
static void s_ControlServo(float roll)
{
    if (roll < -90.0f) roll = -90.0f;
    if (roll >  90.0f) roll =  90.0f;
    float angle = roll + 90.0f;
    uint16_t pwm_servo = (uint16_t)(1200 + (angle / 180.0f) * (1900 - 1200));
    if (pwm_servo < 1200) pwm_servo = 1200;
    if (pwm_servo > 1900) pwm_servo = 1900;
    __HAL_TIM_SET_COMPARE(servo_htim, servo_channel, pwm_servo);
}

// DC 모터 제어 함수 (기존 Control_DC_Motor)
static void s_ControlDcMotor(uint16_t accel_ms, uint16_t brake_ms)
{
    static int16_t duty = 0;
    const int16_t accel_step = 50;
    const int16_t brake_step = 100;
    const int16_t max_duty = 1000;

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
    __HAL_TIM_SET_COMPARE(dc_motor_htim, dc_motor_channel, duty);
}

// 모터 방향 제어 함수 (기존 Update_Motor_Direction)
static void s_UpdateMotorDirection(MotorDirection_t direction)
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
