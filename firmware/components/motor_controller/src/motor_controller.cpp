#include "motor_controller.hpp"

#include "tmc.hpp"

constexpr const char* TAG = "MotorController";
MotorController::MotorController(TmcBus& bus, MotorIO m_io0, MotorIO m_io1,
                                 MotorIO m_io2, MotorIO m_io3,
                                 gpio_num_t enn_pin)
    : enn_pin(enn_pin),
      bus(bus),
      motors{
          Tmc(bus, m_io0.address, m_io0.step, m_io0.dir),
          Tmc(bus, m_io1.address, m_io1.step, m_io1.dir),
          Tmc(bus, m_io2.address, m_io2.step, m_io2.dir),
          Tmc(bus, m_io3.address, m_io3.step, m_io3.dir),
      } {
  gpio_set_direction(enn_pin, GPIO_MODE_OUTPUT);
  gpio_set_level(enn_pin, UINT32_MAX);
};

esp_err_t MotorController::init() {
  for (Tmc& motor : motors) {
    ESP_RETURN_ON_ERROR(motor.init(), TAG, "motor init");

    ESP_RETURN_ON_ERROR(motor.setNodeConf(3), TAG, "motor node conf");

    Gconf_t gconf{
        .i_scale_analog = false,
        .internal_rsense = false,
        .en_spread_cycle = false,
        .shaft = false,
        .index_otpw = false,
        .index_step = false,
        .pdn_disable = true,
        .mstep_reg_select = true,
        .multistep_filt = true,
        .test_mode = false,
    };
    ESP_RETURN_ON_ERROR(motor.setGconf(gconf), TAG, "motor gconf");

    IHoldIRun_t iconf{
        .i_hold = 10,
        .i_run = 28,
        .i_hold_delay = 12,
    };
    ESP_RETURN_ON_ERROR(motor.setIHoldIRun(iconf), TAG, "motor iconf");

    ChopConf_t chop_conf{
        .diss2vs = false,
        .diss2g = false,

        .dedge = false,
        .intpol = true,

        .mres3 = false,
        .mres2 = true,
        .mres1 = false,
        .mres0 = false,

        .vsense = true,

        .tbl1 = false,
        .tbl0 = false,

        .hend3 = false,
        .hend2 = false,
        .hend1 = false,
        .hend0 = false,

        .hstrt2 = true,
        .hstrt1 = false,
        .hstrt0 = true,

        .toff3 = false,
        .toff2 = false,
        .toff1 = true,
        .toff0 = true,
    };
    ESP_RETURN_ON_ERROR(motor.setChopConf(chop_conf), TAG, "motor chop conf");
    ESP_LOGI(TAG, "GCONF=0x%08lX CHOP=0x%08lX", (uint32_t)gconf,
             (uint32_t)chop_conf);
  }

  all_intialised = true;
  return ESP_OK;
}

esp_err_t MotorController::setENN(bool level) {
  if (!level && !all_intialised) {
    ESP_LOGE(TAG, "Can't lift ENN line if not all motors are initialised");
    return ESP_ERR_INVALID_STATE;
  }
  uint32_t pin_level = 0;
  if (level) {
    pin_level = UINT32_MAX;
  } else {
    pin_level = 0;
  }
  return gpio_set_level(enn_pin, pin_level);
}

Tmc& MotorController::get_motor(uint8_t num) {
  if (num > 3) {
    ESP_LOGE(TAG, "Can not get motor num: % as there are only 0-3", num);
    abort();
  }
  if (!all_intialised) {
    ESP_LOGW(TAG, "Not all motors initalised yet when trying to get motor");
  }

  return motors.at(num);
}
