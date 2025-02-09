#define LED_B A0    // BLUE for standby
const int btns[3] = {11, 12, 13};
const int green_leds[3] = {A1, A2, A3};   // LED for green status
const int floor_leds[7] = {2, 3, 4, 5, 6, 7, 8};  // LEDs for each floor

unsigned long last_bounces[3];
bool called_floors[3] = {false, false, false};
unsigned int current_floor = 0;  // Start at floor 0
unsigned int target_floor = 0;  // No target initially
bool isMoving = false;
unsigned long last_move_time = 0;
const unsigned long moveInterval = 1000;
const unsigned long debounceInterval = 600;

class Queue {
  private:
    int front, rear, count;
    int queue[10];
  public:
    Queue() {
      front = 0;
      rear = 0;
      count = 0;
    }
    bool isEmpty() { return count == 0; }
    void enqueue(int floor) {
      if (count < 10) {
        queue[rear] = floor;
        rear = (rear + 1) % 10;
        count++;
      }
    }
    int dequeue() {
      if (isEmpty()) return -1;
      int floor = queue[front];
      front = (front + 1) % 10;
      count--;
      return floor;
    }
    int peek() {  // 다음 목표 층을 확인 (제거 X)
      return isEmpty() ? -1 : queue[front];
    }
    void remove(int floor){
      if (isEmpty()) return; 
      int temp[10], newCount =0, newRear = 0;
      while(!isEmpty()){
        int f = dequeue();
        if (f != floor) temp[newRear++] = f; 
      }
      for (int i=0; i<newRear; i++) enqueue(temp[i]);
    }
};

Queue lift_order;

void moveUp() {
  isMoving = true;
  digitalWrite(floor_leds[++current_floor], HIGH);
  digitalWrite(floor_leds[current_floor - 1], LOW);
}

void moveDown() {
  isMoving = true;
  digitalWrite(floor_leds[--current_floor], HIGH);
  digitalWrite(floor_leds[current_floor + 1], LOW);
}

void arrival(int target_F) {
  isMoving = false;
  digitalWrite(green_leds[target_F / 3], LOW); // 목표 층 LED 끄기
  called_floors[target_F / 3] = false;  // 호출 상태 초기화
}

void buttonDetect() {
  unsigned long current_millis = millis();
  for (int i = 0; i < 3; i++) {
    int floor = i * 3; // indices 0, 3, 6 -> 1~3F  

    if (digitalRead(btns[i]) == HIGH && current_millis - last_bounces[i] > debounceInterval) {
      digitalWrite(green_leds[i], HIGH);  
      
      if (!called_floors[i]) {   // enqueue only for not being called 
        lift_order.enqueue(floor);
        called_floors[i] = true;  
      } else{
        // 같은 층 버튼을 다시 누르면 호출 취소 
        lift_order.remove(floor);
        digitalWrite(green_leds[i], LOW);
        called_floors[i] = false;   
      }
      digitalWrite(LED_B, LOW);
      last_bounces[i] = current_millis; // update last calling time 
    }
  }
}

void setup() {
  Serial.begin(9600);
  pinMode(LED_B, OUTPUT);
  for (int i = 0; i < 7; i++) pinMode(floor_leds[i], OUTPUT);
  for (int i = 0; i < 3; i++) {
    pinMode(green_leds[i], OUTPUT);
    pinMode(btns[i], INPUT);
  }
  memset(&last_bounces[0], 0, sizeof(last_bounces));
  digitalWrite(floor_leds[0], HIGH);  // The lift starts at first floor.
}

void standby() {
  digitalWrite(LED_B, HIGH);
  isMoving = false;
}

void loop() {
  buttonDetect();  
  
  if (!isMoving && !lift_order.isEmpty()) {
    target_floor = lift_order.peek();  // 다음 목적지 확인
    unsigned long current_time = millis();
    if (current_time - last_move_time >= moveInterval) {
      if (current_floor < target_floor) {moveUp();}
      else if (current_floor > target_floor) {moveDown();}
      else {
        arrival(target_floor);
        lift_order.dequeue();  // 도착 후 큐에서 제거
      }
      last_move_time = current_time;
    }
  } else {
    standby();
  }
}
