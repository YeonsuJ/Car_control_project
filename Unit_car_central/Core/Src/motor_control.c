#include "motor_control.h"

// === 타이머 및 GPIO 외부 참조 ===
extern TIM_HandleTypeDef htim1; // DC 모터 PWM 타이머
extern TIM_HandleTypeDef htim2; // 서보 모터 PWM 타이머

// === private 매크로 ===
#define MOTOR_FORWARD()     do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_RESET); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_SET); } while (0)
#define MOTOR_BACKWARD()    do { HAL_GPIO_WritePin(GPIOA, GPIO_PIN_0, GPIO_PIN_SET); HAL_GPIO_WritePin(GPIOA, GPIO_PIN_1, GPIO_PIN_RESET); } while (0)


// === DC 모터 제어를 위한 새로운 매크로 ===
#define MAX_DUTY            1000    // PWM 최대 듀티 값 (htim1의 ARR 값에 맞춰 설정)
#define MIN_DUTY_ON_ACCEL   400     // 가속 시작 시 최소 듀티 (모터가 돌기 시작하는 최소값)
#define ACCEL_SENSITIVITY   0.8f    // 값이 클수록 버튼 누르는 시간에 비해 속도가 빠르게 증가
#define COAST_DECREMENT     3      // 아무 버튼도 안 눌렀을 때 서서히 감속되는 값
#define BRAKE_STEP          500     // 브레이크 버튼 눌렀을 때 감속되는 값

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

// DC 모터 제어 함수 (수정됨)
void Control_DcMotor(uint16_t accel_ms, uint16_t brake_ms)
{
    static int16_t current_duty = 0;

    if (brake_ms > 0)
    {
        // 브레이크 버튼이 눌리면 빠르게 감속합니다.
        current_duty -= BRAKE_STEP;
    }
    else if (accel_ms > 0)
    {
        // 가속 버튼이 눌리면, 눌린 시간을 기반으로 목표 듀티를 계산합니다.
        // 오래 누를수록 target_duty 값이 커집니다.
        int16_t target_duty = (int16_t)(MIN_DUTY_ON_ACCEL + (accel_ms * ACCEL_SENSITIVITY));

        // 현재 듀티를 목표 듀티로 설정합니다.
        // (만약 더 부드러운 가속을 원한다면, current_duty를 target_duty까지 점진적으로 증가시키는 로직을 추가할 수 있습니다.)
        current_duty = target_duty;
    }
    else // 아무 버튼도 눌리지 않았을 때
    {
        // 서서히 속도를 줄여 관성 주행(Coasting)을 구현합니다.
        current_duty -= COAST_DECREMENT;
    }

    // 듀티 값이 유효한 범위를 벗어나지 않도록 제한합니다.
    if (current_duty > MAX_DUTY)
    {
        current_duty = MAX_DUTY;
    }
    if (current_duty < 0)
    {
        current_duty = 0;
    }

    __HAL_TIM_SET_COMPARE(&htim1, TIM_CHANNEL_4, current_duty);
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
