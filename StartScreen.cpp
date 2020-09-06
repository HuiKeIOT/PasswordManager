#include "GenericScreen.h"

namespace espwv32 {

class StartScreen : public GenericScreen {
  public:
    StartScreen(String deviceId) {
      _deviceId = deviceId;
      reset();
    }
    
    ScreenType getType(){
      return START;
    }
  private:
    String _deviceId;
    void show() {
      M5.Lcd.fillScreen(BLACK);//背景颜色
      M5.Lcd.setTextColor(WHITE);//字体颜色
      M5.Lcd.setRotation(3);//显示角度
      M5.Lcd.setTextSize(3);//字体大小
      M5.Lcd.setCursor(8,30);//字体位置
      M5.Lcd.print("PassWord");//显示内容
    }
};
}
