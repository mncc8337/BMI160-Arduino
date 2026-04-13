#include <Arduino.h>
#include <Wire.h>
#include <BMI160Gen.h>

void setup() {
    Serial.begin(115200);
    
    while(!Serial); 
    delay(1000);

    Serial.println("initializing BMI160");

    Wire.begin();

    if(!BMI160.begin(BMI160GenClass::I2C_MODE, 0x69)) {
        Serial.println("failed to initialize BMI160");
        while(1);
    }

    if(!BMI160.initializeMagnetometer(0x2C, 0x00, 0x80)) {
        Serial.println("failed to initialize qmc5883p");
    }
    // now we are at manual mode

    // my board is 90 degree counter-clockwise off from the bmi160
    BMI160.setMagRotation(BMI160_MAG_ROTATION_90);

    // specific board config like below is only available in manual mode

    // soft reset
    BMI160.writeMagRegister(0x0B, 0x80); // write 0x80 to 0x0B register
    delay(10);
    // set set/reset period
    BMI160.writeMagRegister(0x0B, 0x01); // ...
    // continuous mode, ODR 200hz, oversampling x4
    BMI160.writeMagRegister(0x0A, 0x1D);

    // see https://www.qstcorp.com/upload/pdf/202512/2C939E5AA0704285BC3BE71132B8629B.pdf
    // for full command list

    // after done configuring we can switch to data mode
    // which will automatically poll data for us

    // set QMC5883P ODR (only needed in auto mode)
    // ODR = 50/(2**(7 - raw_8bit_value))
    BMI160.setMagRate(0x09);

    // since QMC is on continuous mode, this settings does not effect the program
    // only need in auto mode
    BMI160.setMagDelay(0);

    // switch to data mode
    // set sensor data register to 0x01
    // since the QMC5883P is on continuous mode, we dont need to set the command write register
    // so we set it to a readonly register like 0x00 so it dont mess with the automatic
    // data poll process
    BMI160.magDataMo€ý.de(0x01, 0x00);
}

void loop() {
    // we dont need to trigger a read request, because the BMI is€ý. already do it for us
    // read 6 bytes. start from 0x01 register (first data byte)

    int16_t magX, magY, magZ;

    // we can either get the data by reading the register directly ...
    uint8_t xl = BMI160.getReadMagRegister(0); // get byte #0
    uint8_t xh = BMI160.getReadMagRegister(1); // get byte #1
    uint8_t yl = BMI160.getReadMagRegister(2); // ...
    uint8_t yh = BMI160.getReadMagRegister(3);
    uint8_t zl = BMI160.getReadMagRegister(4);
    uint8_t zh = BMI160.getReadMagRegister(5);

    magX = (int16_t)((xh << 8) | xl);
    magY = (int16_t)((yh << 8) | yl);
    magZ = (int16_t)((zh << 8) | zl);

    Serial.print("Mag direct X: "); Serial.print(magX);
    Serial.print(" | Y: "); Serial.print(magY);
    Serial.print(" | Z: "); Serial.println(magZ);
    // note that there is no axis switch
    // because you are reading the registers directly

    // ... or use the wrapper
    // this one actually account for the board disorientation
    BMI160.getMagneticField(&magX, &magY, &magZ);
    Serial.print("Mag rotated X: "); Serial.print(magX);
    Serial.print(" | Y: "); Serial.print(magY);
    Serial.print(" | Z: "); Serial.println(magZ);

    Serial.println();

    delay(100);
}

