#include <BLEPeripheral.h>
#include <SparkFunTMP102.h>

#include <nrf_nvic.h>

BLEPeripheral ble;

BLEService tempService("babe");
BLEFloatCharacteristic tempCharacteristic("2a6e", BLERead | BLENotify);
BLEDescriptor tempDescriptor("2901", "Temperature");

static unsigned long then = millis();
static int _connected = 0;

void setup(void)
{
	ble.setDeviceName("BLE/TMP102");
	ble.setAdvertisedServiceUuid(tempService.uuid());
	ble.addAttribute(tempService);
	ble.addAttribute(tempCharacteristic);
	ble.addAttribute(tempDescriptor);
	ble.setEventHandler(BLEConnected, [=] (BLECentral &central) {
		_connected++;
	});
	ble.setEventHandler(BLEDisconnected, [=] (BLECentral &central) {
		_connected--;
		sd_nvic_SystemReset();
	});
	ble.begin();
}

void loop(void)
{
	ble.poll();
	unsigned long now = millis();
	if (_connected > 0) {
		if (now - then >= 25 * 1000) {
			TMP102 tmp102(0x48);
			tmp102.begin();
			tmp102.setFault(0);
			tmp102.setAlertPolarity(1);
			tmp102.setConversionRate(2);
			tmp102.setExtendedMode(0);
			tmp102.setHighTempC(60);
			tmp102.setLowTempC(-30);
			tmp102.wakeup();
			tempCharacteristic.setValue(tmp102.readTempC());
			tmp102.sleep();
			tmp102.end();
			then = now;
		}
	}
	else {		
		sd_power_dcdc_mode_set(NRF_POWER_DCDC_ENABLE);
		sd_power_mode_set(NRF_POWER_MODE_LOWPWR);
		sd_app_evt_wait();
	}
}
