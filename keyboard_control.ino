// keyboard_control.ino

String command = "";
String lastCommand ="";

unsigned long lastHandledTime = 0;
const unsigned long commandCooldown = 300; // 300 ms

void handleSerialInput() {
  while (Serial.available()) {
    char c = Serial.read();
    // Serial.print("[Serial char]: "); Serial.println(c);  // ⬅️ in từng ký tự nhận được


    // Nếu là ký tự kết thúc dòng (Enter từ Python), xử lý lệnh
    if (c == '\n') {
      command.trim();
      // Serial.print("[Command nhận được]: "); Serial.println(command);  // ⬅️ in lệnh hoàn chỉnh

      if (command.length() > 0 && (millis() - lastHandledTime > commandCooldown)){
            handleKeyCombo(command);
            lastHandledTime = millis();

            lastCommand = command;
      }
      command = "";
  
    } else if (c != '\r') {
      command += c;  // Thêm ký tự vào chuỗi lệnh
    }
  }
}

void handleKeyCombo(String combo) {
  // Serial.print("[Xử lý lệnh]: "); Serial.println(combo);  // ⬅️ in ra mỗi lần xử lý

  combo.toLowerCase();

  if (combo == "w") {
    runForward();
    Serial.print("t");
  } else if (combo == "s") {
    runBackward();
    Serial.print("l");
  } else if (combo == "a") {
    runLeft();
    Serial.println("tr");
  } else if (combo == "d") {
    runRight();
    Serial.println("p2");
  } else if (combo == "x") {
    stopbrake();
    Serial.println("p1");
  } else if (combo == "wa" || combo == "aw") {
    runForward();
    runLeft();
    Serial.println("wa1");
  } else if (combo == "wd" || combo == "dw") {
    runForward();
    runRight();
    Serial.println("wd1");
  } else if (combo == "sa" || combo == "as") {
    runBackward();
    runLeft();
    Serial.println("sa");
  } else if (combo == "sd" || combo == "ds") {
    runBackward();
    runRight();
    Serial.println("sd1");
  } else if (combo == "sx" || combo == "xs") {
    runBackward();
    stopbrake();
    Serial.println("sx");
  } else if (combo == "wx" || combo == "xw") {
    runForward();
    stopbrake();
    Serial.println("ws");
  } else {
    Serial.print("Lệnh không hợp lệ: ");
    Serial.println(combo);
  }
}
