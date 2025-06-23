#include <Wire.h>
#include <MPU6050.h>

MPU6050 mpu;

int16_t imu_ax, imu_ay, imu_az;
int16_t imu_gx, imu_gy, imu_gz;

void initIMU() {
  Wire.begin();
  mpu.initialize();
  if (!mpu.testConnection()) {
    Serial.println("MPU6050 connection failed");
    while (1);  // Dừng lại nếu lỗi
  } else {
    Serial.println("MPU6050 connected");
  }
}

void updateIMU() {
  mpu.getMotion6(&imu_ax, &imu_ay, &imu_az, &imu_gx, &imu_gy, &imu_gz);
}

void publishIMU() {
  Serial.print("IMU:");
  Serial.print(imu_ax); Serial.print(",");
  Serial.print(imu_ay); Serial.print(",");
  Serial.print(imu_az); Serial.print(",");
  Serial.print(imu_gx); Serial.print(",");
  Serial.print(imu_gy); Serial.print(",");
  Serial.println(imu_gz);
}
