//inspired by https://github.com/T-vK/ESP32-BLE-Keyboard

#include "ble.h"
#include <Arduino.h>

using namespace ble;

BLEKeyboard::BLEKeyboard(std::string deviceName, std::string deviceManufacturer) {
  this->deviceName = deviceName;
  this->deviceManufacturer = deviceManufacturer;

  BLEDevice::init(this->deviceName);
  BLEDevice::setEncryptionLevel(ESP_BLE_SEC_ENCRYPT_MITM);
  this->pServer = BLEDevice::createServer();
  pServer->setCallbacks(this);
  BLEDevice::setSecurityCallbacks(this);

  BLESecurity *pSecurity = new BLESecurity();
  pSecurity->setAuthenticationMode(ESP_LE_AUTH_REQ_SC_MITM_BOND);
  pSecurity->setCapability(ESP_IO_CAP_OUT);
  pSecurity->setKeySize(16);
  pSecurity->setInitEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);
  pSecurity->setRespEncryptionKey(ESP_BLE_ENC_KEY_MASK | ESP_BLE_ID_KEY_MASK);

  this->hid = new BLEHIDDevice(pServer);
  hid->manufacturer()->setValue(this->deviceManufacturer);
  this->inputKeyboard = hid->inputReport(0x01);
  hid->outputReport(0x01);
  hid->inputReport(0x02);
  hid->pnp(SIG, VENDOR_ID, PRODUCT_ID, PRODUCT_VERSION);
  hid->hidInfo(0x00, 0x01);
  hid->reportMap((uint8_t*)_hidReportDescriptor, sizeof(_hidReportDescriptor));
  hid->startServices();

  BLEAdvertising *pAdvertising = pServer->getAdvertising();
  pAdvertising->setScanResponse(true);
  pAdvertising->setMinPreferred(0x06);  // functions that help with iPhone connections issue
  pAdvertising->setMinPreferred(0x12);
  pAdvertising->setAppearance(HID_KEYBOARD);
  pAdvertising->addServiceUUID(hid->hidService()->getUUID());
  pAdvertising->start();
  Serial.println("Advertising started!");

  releaseAll();
};

BLEKeyboard::~BLEKeyboard() {
  //TODO: disable Bluetooth
  delete &deviceName;
  delete &deviceManufacturer;
}

void BLEKeyboard::onConnect(BLEServer* pServer)
{
  Serial.println("TryConnect");//尝试蓝牙连接
  BLEDescriptor *desc = inputKeyboard->getDescriptorByUUID(BLEUUID((uint16_t)0x2902));
  uint8_t val[] = {0x01, 0x00};
  desc->setValue(val, 2);
  //TODO: Notify of incomming connection
}

void BLEKeyboard::onDisconnect(BLEServer* pServer)
{
  this->connected = false;
  callbacks->disconnected();
}

uint32_t BLEKeyboard::onPassKeyRequest()
{
  //TODO: Error + disconenct
  return UINT32_MAX;
}

void BLEKeyboard::onPassKeyNotify(uint32_t pass_key)
{
  //TODO notify we need to display the PIN
  if (callbacks != NULL) {
    Serial.println("Show PIN");
    callbacks->authenticationInfo(pass_key);//显示蓝牙密钥
  } else {
    Serial.println("No callback to notify");
  }
}

bool BLEKeyboard::onSecurityRequest()
{
  return true;
}

void BLEKeyboard::onAuthenticationComplete(esp_ble_auth_cmpl_t cmpl)
{
  if (cmpl.success)
  {
    //TODO: Add device to the whitelist: esp_ble_gap_update_whitelist
    //TODO: Notify connection succeeded
    this->connected = true;
    callbacks->connected();
  } else {

    disconnect();
    //TODO: Notify connection failed
  }
}

bool BLEKeyboard::onConfirmPIN(unsigned int v)
{
  //TODO: Error
  disconnect();
  return false;
}

void BLEKeyboard::disconnect() {
  BLEDevice::removePeerDevice(pServer->getConnId(), true);
  pServer->disconnect(pServer->getConnId());
}

bool BLEKeyboard::isConnected() {
  return this->connected;
}

void BLEKeyboard::sendReport(KeyReport* keys)
{
  if (this->isConnected())
  {
    this->inputKeyboard->setValue((uint8_t*)keys, sizeof(KeyReport));
    this->inputKeyboard->notify();
    Serial.printf("Sending %d | %d \n", _keyReport.modifiers, _keyReport.keys[0]);
  } else {
    Serial.println("NOT sending, NOT connected");
  }
}

size_t BLEKeyboard::press(uint8_t k)
{
  if (k & SHIFT) {            // it's a capital letter or other character reached with shift
    _keyReport.modifiers |= 0x02; // the left shift modifier
    k &= 0x7F;
  }
  _keyReport.keys[0] = k;

  sendReport(&_keyReport);
  return 1;
}

size_t BLEKeyboard::release(uint8_t k)
{
  if (k & SHIFT) {              // it's a capital letter or other character reached with shift
    _keyReport.modifiers &= ~(0x02);  // the left shift modifier
    k &= 0x7F;
  }
  // Test the key report to see if k is present.  Clear it if it exists.
  // Check all positions in case the key is present more than once (which it shouldn't be)
  for (uint8_t i = 0; i < 6; i++) {
    if (0 != k && _keyReport.keys[i] == k) {
      _keyReport.keys[i] = 0x00;
    }
  }

  sendReport(&_keyReport);
  return 1;
}
void BLEKeyboard::releaseAll(void)
{
  _keyReport.keys[0] = 0;
  _keyReport.keys[1] = 0;
  _keyReport.keys[2] = 0;
  _keyReport.keys[3] = 0;
  _keyReport.keys[4] = 0;
  _keyReport.keys[5] = 0;
  _keyReport.modifiers = 0;
  sendReport(&_keyReport);
}


size_t BLEKeyboard::write(uint8_t c)
{
  uint8_t p = press(_asciimap[c]);  // Keydown
  release(_asciimap[c]);            // Keyup
  return p;              // just return the result of press() since release() almost always returns 1
}

size_t BLEKeyboard::write(const uint8_t *buffer, size_t size) {
  size_t n = 0;
  while (size--) {
    if (*buffer != '\r') {
      if (write(*buffer)) {
        n++;
      } else {
        break;
      }
    }
    buffer++;
  }
  return n;
}

void BLEKeyboard::setBatteryLevel(uint8_t level) {
  this->hid->setBatteryLevel(level);
}

void BLEKeyboard::setCallbacks(BLEKeyboardCallbacks* callbacks) {
  this->callbacks = callbacks;
}
