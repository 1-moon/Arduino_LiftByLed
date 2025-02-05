#define LED_B A0    // BLUE for standby
const int btns[3] = {11,12,13};
const int green_leds[3] = {A1, A2, A3};   // LED for green status
const int floor_leds[7] = {2, 3, 4, 5, 6, 7, 8};  // LEDs for each floor
unsigned long last_bounces[3];
bool called_floors[3];
unsigned int current_floor = 0;  // Start at floor 0
unsigned int target_floor = 0;  // No target initially
bool isMoving = false;
unsigned long last_move_time = 0;
const unsigned long moveInterval = 650;
const unsigned long debounceInterval = 500;
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
};
Queue lift_order;
void moveUp(){
  isMoving = true;
  digitalWrite(floor_leds[++current_floor], HIGH);
  digitalWrite(floor_leds[current_floor - 1], LOW);
}
void moveDown(){
  isMoving = true;
  digitalWrite(floor_leds[--current_floor], HIGH);
  digitalWrite(floor_leds[current_floor + 1], LOW);
}
void arrival(int target_F){
  isMoving = false;
  digitalWrite(green_leds[target_F / 3], LOW);
}
void moveLift(int target) {
  while (current_floor != target) {
    unsigned long current_time = millis();
    if (current_time - last_move_time >= moveInterval) {
      if (current_floor < target)      { moveUp(); }
      else if (current_floor > target) { moveDown(); }
      last_move_time = current_time;
    }
  }
  arrival(target);
}
void buttonDetect() {
  unsigned long current_millis = millis();
  for (int i = 0; i < 3; i++) {
    if (digitalRead(btns[i]) == HIGH && current_millis - last_bounces[i] > debounceInterval) {
      digitalWrite(green_leds[i], HIGH);
      digitalWrite(LED_B, LOW);
      lift_order.enqueue(i * 3);
      last_bounces[i] = current_millis;
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
void standby(){
  digitalWrite(LED_B, HIGH);
  isMoving = false;
}
void loop() {
  buttonDetect();
  if (!lift_order.isEmpty()) {
    target_floor = lift_order.dequeue();
    moveLift(target_floor);
  } else {standby();}
}