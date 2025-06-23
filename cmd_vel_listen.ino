String serialBuffer = "";
float target_linear = 0.0;
float target_angular = 0.0;
void cmd() {
  while (Serial.available()) {
    char c = Serial.read();
    if (c == '\n') {
      serialBuffer.trim();
      handleVelocityCommand(serialBuffer);
      serialBuffer = "";
    } else if (c != '\r') {
      serialBuffer += c;
    }
  }
}

void handleVelocityCommand(String cmd) {
  if (!cmd.startsWith("vel:")) return;

  cmd.remove(0, 4); // bỏ "vel:"
  int commaIndex = cmd.indexOf(',');
  if (commaIndex == -1) return;

  float linear = cmd.substring(0, commaIndex).toFloat();
  float angular = cmd.substring(commaIndex + 1).toFloat();

  // Debug log
  Serial.print("[VEL CMD] linear="); Serial.print(linear);
  Serial.print(" angular="); Serial.println(angular);

  // Điều khiển theo giá trị cmd_vel
  if (abs(linear) > 0.01) {
    if (linear > 0.0) runForward();
    else runBackward();
  }

  if (abs(angular) > 0.01) {
    if (angular > 0.0) runLeft();
    else runRight();
  }

  // Nếu cả linear và angular đều nhỏ, thì dừng
  // if (abs(linear) <= 0.01 && abs(angular) <= 0.01) {
  //   stopbrake();
  // }
}
