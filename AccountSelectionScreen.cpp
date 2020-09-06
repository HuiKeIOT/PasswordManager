#include "GenericScreen.h"
#include "Storage.h"
#include "ble.h"


namespace espwv32 {

class AccountSelectionScreen: public GenericScreen {
  public:
    AccountSelectionScreen(ble::BLEKeyboard* keyboard, uint8_t *userPin) {
      _storage = new Storage();
      _keyboard = keyboard;
      _userPin = userPin;
      Serial.printf("Account Selection with pin %d%d%d%d\n", _userPin[0], _userPin[1], _userPin[2], _userPin[3]);
      _userPinSize = sizeof(_userPin) / sizeof(uint8_t);
      reset();
    }
    ~AccountSelectionScreen() {
      delete _storage;
    }

    void reset() {
      _accountIndex = 0;
      GenericScreen::reset();
    }

    virtual void buttonPressedA() {//A键短按
      _accountIndex++;
      if (_accountIndex >= NUM_ACCOUNTS)//要与设置的账户组数量匹配
        _accountIndex = 0;
      Serial.printf("Next %d \n", _accountIndex);
      show();//账户页面，递增
    
    switch (_accountIndex)
    {
    case 0://QQ
    _dataToSend = PASSWORD;//和初始默认值保持一致
    break;
    case 1://Tongji GM
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 2://Taobao
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 3://School Email
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 4://Ubuntu
    _dataToSend = PASSWORD;
    break;
    case 5://Windows 10
    _dataToSend = PASSWORD;
    break;
    case 6://Google
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 7://Bilibili
    _dataToSend = USERNAME;
    break;
    default://Others
   break;
    }
    showDataToSendControls();
    }

    virtual void buttonMediumPressedA() {//A按键中按,发送信息
      M5.Lcd.fillScreen(WHITE);
      Serial.println("sending");
      switch (_dataToSend) {
        case USERNAME_PASSWORD:
          _keyboard->print(_currentAccount.username);
          _keyboard->print("\t");
          _keyboard->println(_currentAccount.password);
          Serial.print(_currentAccount.username);
          Serial.print("\t");
          Serial.println(_currentAccount.password);
          break;
        case USERNAME:
          _keyboard->print(_currentAccount.username);
          Serial.print(_currentAccount.username);
          break;
        case PASSWORD:
          _keyboard->print(_currentAccount.password);
          Serial.print(_currentAccount.password);
          break;
      }
      show();
    }

    virtual void buttonPressedB() {//B按键短按
    if (_accountIndex == 0)
    {
      _accountIndex = NUM_ACCOUNTS-1;
    }
    else
    {
      _accountIndex--;
    }
      Serial.printf("Previous %d \n", _accountIndex);
      show();//账户页面，递减

   switch (_accountIndex)
    {
    case 0://Windows 10
    _dataToSend = PASSWORD;//和初始默认值保持一致
    break;
    case 1://Tongji GM
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 2://Taobao
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 3://School Email
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 4://Ubuntu
    _dataToSend = PASSWORD;
    break;
    case 5://QQ
    _dataToSend = PASSWORD;
    break;
    case 6://Google
    _dataToSend = USERNAME_PASSWORD;
    break;
    case 7://Bilibili
    _dataToSend = USERNAME;
    break;
    default://Others
   break;
    }
    showDataToSendControls();
    }

    virtual void buttonMediumPressedB() {//B按键中按,切换发送类型
      switch (_dataToSend) {
      case USERNAME_PASSWORD:
        _dataToSend = USERNAME;//只发送用户名
        break;
      case USERNAME:
        _dataToSend = PASSWORD;//只发送密码
        break;
      case PASSWORD:
        _dataToSend = USERNAME_PASSWORD;//用户名以及密码都发送
        break;
      }
      showDataToSendControls();
    }

    ScreenType getType() {
      return ACCOUNT_SELECTION;
    }

    void show() {
      M5.Lcd.fillScreen(BLACK);
      M5.Lcd.setRotation(3);

      _currentAccount = _storage->read(_accountIndex, _userPin);

      M5.Lcd.setCursor(2, 2);
      M5.Lcd.setTextSize(3);
      M5.Lcd.setTextColor(BLUE);
      M5.Lcd.printf("%02d", _accountIndex);

      M5.Lcd.setCursor(2, 40);
      M5.Lcd.setTextSize(2);
      M5.Lcd.setTextColor(WHITE);
      M5.Lcd.print(_currentAccount.name);

      showDataToSendControls();

      M5.Lcd.setTextSize(2);
      M5.Lcd.setCursor(-1, 25);
      M5.Lcd.printf("%c", 175);
    }
    
    void showDataToSendControls() {
      M5.Lcd.fillRect(56,0,61,15, BLACK);
      M5.Lcd.setCursor(60, 4);
      M5.Lcd.setTextSize(1);
      M5.Lcd.setTextColor(RED);
      M5.Lcd.printf("U%cP  U  P", 175); //https://votreportail.com/codes-ascii.htm

      switch (_dataToSend) {
        case USERNAME_PASSWORD:
          M5.Lcd.drawRect(56, 0, 24, 15, RED );
          break;
        case USERNAME:
          M5.Lcd.drawRect(86, 0, 14, 15, RED );
          break;
        case PASSWORD:
          M5.Lcd.drawRect(103, 0, 14, 15, RED );
          break;
      }
    }
    
  private:
    uint8_t* _userPin;
    uint8_t _userPinSize;
    const uint8_t NUM_ACCOUNTS =8; // TODO: move to storage
    Storage* _storage;
    uint8_t _accountIndex = 0;
    enum DataToSend {
      USERNAME_PASSWORD,
      USERNAME,
      PASSWORD
    };
    DataToSend _dataToSend = PASSWORD;
    Credentials _currentAccount;
    ble::BLEKeyboard* _keyboard;
};
}
