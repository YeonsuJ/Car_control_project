# IMU 센서 오차 보정 트러블슈팅

## 문제점
자이로 센서는 시간이 지남에 따라 각도 오차가 점진적으로 누적되는 드리프트(drift) 현상으로 인해 부정확한 값을 출력하는 문제가 있으며, 여기에 더해 가속도 센서는 외부의 물리적 진동이나 충격에 민감하여 측정값에 노이즈가 많이 포함되는 특성을 가진다. 이 두 가지 문제가 복합적으로 작용하여 IMU 센서로부터 안정적인 조향 기준값(Roll 각도)을 확보하는 데 어려움이 있었다.

## 해결 방안
- MU 센서가 가진 두 가지 고질적인 문제, 즉 자이로 센서의 드리프트와 가속도 센서의 노이즈를 해결하기 위한 방안으로 칼만 필터를 도입했다. 이는 각기 다른 장단점을 가진 두 센서의 데이터를 융합하여 최적의 결과값을 추정하는 가장 효과적인 방법이라 판단했다.

- 칼만 필터의 핵심 아이디어는 두 센서의 장점을 취하고 단점을 상호 보완하는 것이다. 먼저, 급격한 움직임이나 동적인 변화는 자이로 센서의 빠른 반응성을 활용해 실시간으로 측정했다. 동시에, 이 값에 누적될 수 있는 오차(드리프트)는 중력을 기준으로 절대적인 기울기를 측정하는 가속도 센서의 장기적인 안정성을 이용해 지속적으로 바로잡아 주었다. 즉, 빠르게 변하는 순간적인 값은 자이로를 신뢰하고, 서서히 쌓이는 오차는 가속도계를 통해 보정하는 방식으로 두 센서의 완벽한 협업을 구현했다.

이러한 접근을 통해 자이로 센서의 드리프트와 가속도 센서의 노이즈를 효과적으로 상쇄할 수 있었다. 그 결과, 외부 환경 변화에도 흔들리지 않는 안정적인 Roll 각도를 실시간으로 도출하는 데 성공했고, 이를 바탕으로 목표했던 정확한 조향 제어를 실현할 수 있었다.

## 주요 코드
아래 코드는 MPU6050 센서로부터 데이터를 읽고 칼만 필터를 적용하는 핵심 로직이다.

### 1. 칼만 필터 파라미터 초기화
칼만 필터의 성능을 결정하는 주요 파라미터(Q_angle, Q_bias, R_measure)를 설정한다. 이 값들은 각각 **프로세스 노이즈(자이로)**와 **측정 노이즈(가속도계)**에 대한 신뢰도를 나타내며, 실험적으로 튜닝되었다.

```c
// Kalman_t 구조체 및 파라미터 초기화
Kalman_t KalmanX = {
    .Q_angle = 0.001f,   // 자이로 각속도 예측에 대한 불확실성
    .Q_bias = 0.003f,    // 자이로 바이어스 예측에 대한 불확실성
    .R_measure = 0.03f   // 가속도계 측정값에 대한 불확실성 (노이즈)
};

Kalman_t KalmanY = {
    .Q_angle = 0.001f,
    .Q_bias = 0.003f,
    .R_measure = 0.03f,
};
```

### 2. 센서 데이터 융합 및 각도 계산
MPU6050_Read_All 함수 내에서 가속도계와 자이로 센서 데이터를 모두 읽어와 칼만 필터를 통해 최종 각도를 계산한다.

```c
void MPU6050_Read_All(I2C_HandleTypeDef *I2Cx, MPU6050_t *DataStruct)
{
    // ... (14바이트 데이터 수신 및 파싱) ...

    DataStruct->Ax = DataStruct->Accel_X_RAW / 16384.0;
    DataStruct->Ay = DataStruct->Accel_Y_RAW / 16384.0;
    // ...
    DataStruct->Gx = DataStruct->Gyro_X_RAW / 131.0;
    DataStruct->Gy = DataStruct->Gyro_Y_RAW / 131.0;
    // ...

    // --- 칼만 필터 적용 부분 ---
    double dt = (double)(HAL_GetTick() - timer) / 1000; // 이전 호출과의 시간 간격 계산
    timer = HAL_GetTick();

    // 1. 가속도계 값으로 각도 계산 (노이즈가 있지만 드리프트는 없음)
    double roll = atan(DataStruct->Accel_Y_RAW / sqrt(DataStruct->Accel_X_RAW * DataStruct->Accel_X_RAW + DataStruct->Accel_Z_RAW * DataStruct->Accel_Z_RAW)) * RAD_TO_DEG;
    double pitch = atan2(-DataStruct->Accel_X_RAW, DataStruct->Accel_Z_RAW) * RAD_TO_DEG;

    // 2. 칼만 필터 함수 호출하여 최종 각도 도출
    //    입력: 가속도계 각도(newAngle), 자이로 각속도(newRate), 시간 변화량(dt)
    DataStruct->KalmanAngleY = Kalman_getAngle(&KalmanY, pitch, DataStruct->Gy, dt);
    DataStruct->KalmanAngleX = Kalman_getAngle(&KalmanX, roll, DataStruct->Gx, dt);
}
```

### 3. 칼만 필터 알고리즘 (Kalman_getAngle)
이 함수는 칼만 필터의 핵심인 **예측(Prediction)**과 갱신(Update) 단계를 수행한다.

```c
double Kalman_getAngle(Kalman_t *Kalman, double newAngle, double newRate, double dt)
{
    // --- 1. 예측 (Prediction) 단계 ---
    // 자이로 센서의 각속도(newRate)를 적분하여 다음 각도를 예측.
    // 드리프트가 누적될 수 있는 부분.
    double rate = newRate - Kalman->bias;
    Kalman->angle += dt * rate;

    // ... (오차 공분산 행렬 업데이트) ...

    // --- 2. 갱신 (Update) 단계 ---
    // 가속도계로 계산한 각도(newAngle)와 자이로로 예측한 각도의 차이(y)를 계산.
    double y = newAngle - Kalman->angle;

    // ... (칼만 이득 K 계산) ...

    // 칼만 이득을 사용하여 예측된 각도와 바이어스를 보정.
    // 이 과정을 통해 자이로의 드리프트가 제거됨.
    Kalman->angle += K[0] * y;
    Kalman->bias += K[1] * y;

    // ... (오차 공분산 행렬 재계산) ...

    return Kalman->angle; // 최종 보정된 각도 반환
};
```