# 비상 처리와 이동 제어 개선


## 1. 비상 확인을 메인 루프에서 분리

이전 코드에서는 `ap_main()`의 루프가 `elevator_task()`를 호출하고, 그 내부에서 비상 버튼과 상태를 확인했습니다. 같은 루프에서 수행하는 DHT 측정·LCD 통신·버튼 디바운스의 대기가 다음 확인 시점에 영향을 줄 수 있었습니다.

최종 코드에서는 `HAL_TIM_PeriodElapsedCallback()`이 TIM5에서 `elevator_emergencyISR()`, TIM2에서 `elevator_timerISR()`를 호출합니다. 초기화 시 두 타이머를 인터럽트 모드로 시작하며, 타이머 설정은 각각 1ms·2ms입니다.

이로써 비상 버튼 확인과 이동 제어의 실행 계기를 메인 루프 순회에서 분리했습니다.

## 2. 다음 감지 층 정지를 명시적으로 표현

이전 `floor_detect()`는 목표 층 센서와 `secondFloor_passed`를 사용했습니다. 비상 발생 시 층 차이가 1보다 크고 2층을 통과하지 않았으면 목표를 2층으로 변경했습니다.

최종 코드는 이동 방향을 `moveDirection`에 저장하고, 진행 방향의 층 센서를 확인하며 `current_floor`를 갱신합니다. 비상이 발생하면 `stopAtNextFloor`를 설정하고, 다음 감지 층에서 `EMERGENCY`로 전환합니다.

예를 들어 1층에서 3층으로 이동할 때, 2층 감지 전에 정지를 예약하면 2층에서, 2층을 지난 뒤 예약하면 3층에서 정지하는 흐름입니다.

## 3. 비상 버튼 전용 디바운스

일반 `button_getState()`는 상태 변화 시 `HAL_Delay(5)`를 호출합니다. 최종 비상 ISR에서는 이 함수를 호출하지 않고 GPIO를 직접 읽습니다. 입력 후보와 반복 횟수를 저장하고 동일 입력이 5회 관찰되면 안정 상태로 반영합니다. 눌림에서 해제로 바뀔 때 수동 비상 플래그를 토글합니다.

일반 층 버튼은 메인 루프에서 기존 함수를 사용합니다. 따라서 개선 범위는 비상 버튼의 ISR 처리이며 전체 버튼 드라이버의 비차단화가 아닙니다.

## 4. 사운드 처리를 메인으로 전달

최종 `floor_detect()`는 `pendingFloorSound`에 층 번호를 저장합니다. `elevator_task()`는 인터럽트 상태를 보존하는 임계 구역에서 요청을 읽고 비운 뒤, 메인 문맥에서 사운드를 처리합니다. 이 변수는 큐가 아닌 단일 요청 슬롯입니다.

## 관련 코드

- [타이머 콜백과 초기화](../firmware/Core/Src/ap/ap_main.c)
- [이동·비상·요청 전달](../firmware/Core/Src/ap/service/elevator/elevator.c)
- [일반 버튼](../firmware/Core/Src/driver/button/button.c)
- [사운드 상태 처리](../firmware/Core/Src/ap/service/sound/sound.c)
