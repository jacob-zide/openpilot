#define SUBARU_STEERING_LIMITS_GENERATOR(steer_max, rate_up, rate_down)          \
  {                                                                              \
    .max_steer = (steer_max),                                                    \
    .max_rt_delta = 940,                                                         \
    .max_rt_interval = 250000,                                                   \
    .max_rate_up = (rate_up),                                                    \
    .max_rate_down = (rate_down),                                                \
    .driver_torque_factor = 50,                                                  \
    .driver_torque_allowance = 60,                                               \
    .type = TorqueDriverLimited,                                                 \
  }                                                                              \

const SteeringLimits SUBARU_STEERING_LIMITS      = SUBARU_STEERING_LIMITS_GENERATOR(2047, 50, 70);
const SteeringLimits SUBARU_GEN2_STEERING_LIMITS = SUBARU_STEERING_LIMITS_GENERATOR(1000, 40, 40);


const LongitudinalLimits SUBARU_LONG_LIMITS = {
  .min_gas = 808,       // appears to be engine braking
  .max_gas = 3400,      // approx  2 m/s^2 when maxing cruise_rpm and cruise_throttle
  .inactive_gas = 1818, // this is zero acceleration
  .max_brake = 600,     // approx -3.5 m/s^2

  .min_transmission_rpm = 0,
  .max_transmission_rpm = 2400,
};

#define MSG_SUBARU_Brake_Status          0x13c
#define MSG_SUBARU_CruiseControl         0x240
#define MSG_SUBARU_Throttle              0x40
#define MSG_SUBARU_Steering_Torque       0x119
#define MSG_SUBARU_Wheel_Speeds          0x13a

#define MSG_SUBARU_ES_LKAS               0x122
#define MSG_SUBARU_ES_LKAS_ANGLE         0x124
#define MSG_SUBARU_ES_Brake              0x220
#define MSG_SUBARU_ES_Distance           0x221
#define MSG_SUBARU_ES_Status             0x222
#define MSG_SUBARU_ES_DashStatus         0x321
#define MSG_SUBARU_ES_LKAS_State         0x322
#define MSG_SUBARU_ES_Infotainment       0x323

#define SUBARU_MAIN_BUS 0
#define SUBARU_ALT_BUS  1
#define SUBARU_CAM_BUS  2

#define SUBARU_COMMON_TX_MSGS(alt_bus, lkas_msg)    \
  {MSG_SUBARU_ES_LKAS,         SUBARU_MAIN_BUS, 8}, \
  {MSG_SUBARU_ES_Distance,     alt_bus,         8}, \
  {MSG_SUBARU_ES_DashStatus,   SUBARU_MAIN_BUS, 8}, \
  {MSG_SUBARU_ES_LKAS_State,   SUBARU_MAIN_BUS, 8}, \
  {MSG_SUBARU_ES_Infotainment, SUBARU_MAIN_BUS, 8}, \

#define SUBARU_COMMON_LONG_TX_MSGS(alt_bus)         \
  {MSG_SUBARU_ES_Brake,        SUBARU_MAIN_BUS, 8}, \
  {MSG_SUBARU_ES_Status,       SUBARU_MAIN_BUS, 8}, \

#define SUBARU_COMMON_ADDR_CHECKS(alt_bus)                                                                                                            \
  {.msg = {{MSG_SUBARU_Throttle,        SUBARU_MAIN_BUS, 8, .check_checksum = true, .max_counter = 15U, .expected_timestep = 10000U}, { 0 }, { 0 }}}, \
  {.msg = {{MSG_SUBARU_Steering_Torque, SUBARU_MAIN_BUS, 8, .check_checksum = true, .max_counter = 15U, .expected_timestep = 20000U}, { 0 }, { 0 }}}, \
  {.msg = {{MSG_SUBARU_Wheel_Speeds,    alt_bus,         8, .check_checksum = true, .max_counter = 15U, .expected_timestep = 20000U}, { 0 }, { 0 }}}, \
  {.msg = {{MSG_SUBARU_Brake_Status,    alt_bus,         8, .check_checksum = true, .max_counter = 15U, .expected_timestep = 20000U}, { 0 }, { 0 }}}, \
  {.msg = {{MSG_SUBARU_CruiseControl,   alt_bus,         8, .check_checksum = true, .max_counter = 15U, .expected_timestep = 50000U}, { 0 }, { 0 }}}, \

const CanMsg SUBARU_TX_MSGS[] = {
  SUBARU_COMMON_TX_MSGS(SUBARU_MAIN_BUS, MSG_SUBARU_ES_LKAS)
};
#define SUBARU_TX_MSGS_LEN (sizeof(SUBARU_TX_MSGS) / sizeof(SUBARU_TX_MSGS[0]))

const CanMsg SUBARU_LONG_TX_MSGS[] = {
  SUBARU_COMMON_TX_MSGS(SUBARU_MAIN_BUS, MSG_SUBARU_ES_LKAS)
  SUBARU_COMMON_LONG_TX_MSGS(SUBARU_MAIN_BUS)
};
#define SUBARU_LONG_TX_MSGS_LEN (sizeof(SUBARU_LONG_TX_MSGS) / sizeof(SUBARU_LONG_TX_MSGS[0]))

const CanMsg SUBARU_GEN2_TX_MSGS[] = {
  SUBARU_COMMON_TX_MSGS(SUBARU_ALT_BUS, MSG_SUBARU_ES_LKAS)
};
#define SUBARU_GEN2_TX_MSGS_LEN (sizeof(SUBARU_GEN2_TX_MSGS) / sizeof(SUBARU_GEN2_TX_MSGS[0]))

AddrCheckStruct subaru_addr_checks[] = {
  SUBARU_COMMON_ADDR_CHECKS(SUBARU_MAIN_BUS)
};
#define SUBARU_ADDR_CHECK_LEN (sizeof(subaru_addr_checks) / sizeof(subaru_addr_checks[0]))
addr_checks subaru_rx_checks = {subaru_addr_checks, SUBARU_ADDR_CHECK_LEN};

AddrCheckStruct subaru_gen2_addr_checks[] = {
  SUBARU_COMMON_ADDR_CHECKS(SUBARU_ALT_BUS)
};
#define SUBARU_GEN2_ADDR_CHECK_LEN (sizeof(subaru_gen2_addr_checks) / sizeof(subaru_gen2_addr_checks[0]))
addr_checks subaru_gen2_rx_checks = {subaru_gen2_addr_checks, SUBARU_GEN2_ADDR_CHECK_LEN};


const uint16_t SUBARU_PARAM_GEN2 = 1;
const uint16_t SUBARU_PARAM_LONGITUDINAL = 2;

bool subaru_gen2 = false;
bool subaru_longitudinal = false;


static uint32_t subaru_get_checksum(CANPacket_t *to_push) {
  return (uint8_t)GET_BYTE(to_push, 0);
}

static uint8_t subaru_get_counter(CANPacket_t *to_push) {
  return (uint8_t)(GET_BYTE(to_push, 1) & 0xFU);
}

static uint32_t subaru_compute_checksum(CANPacket_t *to_push) {
  int addr = GET_ADDR(to_push);
  int len = GET_LEN(to_push);
  uint8_t checksum = (uint8_t)(addr) + (uint8_t)((unsigned int)(addr) >> 8U);
  for (int i = 1; i < len; i++) {
    checksum += (uint8_t)GET_BYTE(to_push, i);
  }
  return checksum;
}

static int subaru_rx_hook(CANPacket_t *to_push) {

  bool valid = addr_safety_check(to_push, &subaru_rx_checks,
                                 subaru_get_checksum, subaru_compute_checksum, subaru_get_counter, NULL);

  if (valid) {
    const int bus = GET_BUS(to_push);
    const int alt_main_bus = subaru_gen2 ? SUBARU_ALT_BUS : SUBARU_MAIN_BUS;

    int addr = GET_ADDR(to_push);
    if ((addr == MSG_SUBARU_Steering_Torque) && (bus == SUBARU_MAIN_BUS)) {
      int torque_driver_new;
      torque_driver_new = ((GET_BYTES(to_push, 0, 4) >> 16) & 0x7FFU);
      torque_driver_new = -1 * to_signed(torque_driver_new, 11);
      update_sample(&torque_driver, torque_driver_new);

      int angle_meas_new = (GET_BYTES(to_push, 4, 2) & 0xFFFFU);
      // convert Steering_Torque -> Steering_Angle to centidegrees, to match the ES_LKAS_ANGLE angle request units
      angle_meas_new = ROUND(to_signed(angle_meas_new, 16) * 2.17);
      update_sample(&angle_meas, angle_meas_new);
    }

    // enter controls on rising edge of ACC, exit controls on ACC off
    if ((addr == MSG_SUBARU_CruiseControl) && (bus == alt_main_bus)) {
      // PFEIFER - AOL {{
      bool cruise_available = GET_BIT(to_push, 40U) != 0U;
      if (!cruise_available) {
        lateral_controls_allowed = 0;
      }
      if(alternative_experience & ALT_EXP_AOL_ENABLE_ON_MAIN) {
        lateral_controls_allowed = cruise_available;
      }
      // }} PFEIFER - AOL
      bool cruise_engaged = GET_BIT(to_push, 41U) != 0U;
      pcm_cruise_check(cruise_engaged);
    }

    // update vehicle moving with any non-zero wheel speed
    if ((addr == MSG_SUBARU_Wheel_Speeds) && (bus == alt_main_bus)) {
      uint32_t fr = (GET_BYTES(to_push, 1, 3) >> 4) & 0x1FFFU;
      uint32_t rr = (GET_BYTES(to_push, 3, 3) >> 1) & 0x1FFFU;
      uint32_t rl = (GET_BYTES(to_push, 4, 3) >> 6) & 0x1FFFU;
      uint32_t fl = (GET_BYTES(to_push, 6, 2) >> 3) & 0x1FFFU;

      vehicle_moving = (fr > 0U) || (rr > 0U) || (rl > 0U) || (fl > 0U);

      float speed = (fr + rr + rl + fl) / 4U * 0.057;
      update_sample(&vehicle_speed, ROUND(speed * VEHICLE_SPEED_FACTOR));
    }

    if ((addr == MSG_SUBARU_Brake_Status) && (bus == alt_main_bus)) {
      brake_pressed = GET_BIT(to_push, 62U) != 0U;
    }

    if ((addr == MSG_SUBARU_Throttle) && (bus == SUBARU_MAIN_BUS)) {
      gas_pressed = GET_BYTE(to_push, 4) != 0U;
    }

    generic_rx_checks((addr == MSG_SUBARU_ES_LKAS) && (bus == SUBARU_MAIN_BUS));
  }
  return valid;
}

static int subaru_tx_hook(CANPacket_t *to_send) {

  int tx = 1;
  int addr = GET_ADDR(to_send);
  bool violation = false;

  if (subaru_gen2) {
    tx = msg_allowed(to_send, SUBARU_GEN2_TX_MSGS, SUBARU_GEN2_TX_MSGS_LEN);
  } else if (subaru_longitudinal) {
    tx = msg_allowed(to_send, SUBARU_LONG_TX_MSGS, SUBARU_LONG_TX_MSGS_LEN);
  } else {
    tx = msg_allowed(to_send, SUBARU_TX_MSGS, SUBARU_TX_MSGS_LEN);
  }

  // steer cmd checks
  if (addr == MSG_SUBARU_ES_LKAS) {
    int desired_torque = ((GET_BYTES(to_send, 0, 4) >> 16) & 0x1FFFU);
    desired_torque = -1 * to_signed(desired_torque, 13);

    bool steer_req = GET_BIT(to_send, 29U) != 0U;

    const SteeringLimits limits = subaru_gen2 ? SUBARU_GEN2_STEERING_LIMITS : SUBARU_STEERING_LIMITS;
    violation |= steer_torque_cmd_checks(desired_torque, steer_req, limits);
  }

  // check es_brake brake_pressure limits
  if (addr == MSG_SUBARU_ES_Brake) {
    int es_brake_pressure = GET_BYTES(to_send, 2, 2);
    violation |= longitudinal_brake_checks(es_brake_pressure, SUBARU_LONG_LIMITS);
  }

  // check es_distance cruise_throttle limits
  if (addr == MSG_SUBARU_ES_Distance) {
    int cruise_throttle = (GET_BYTES(to_send, 2, 2) & 0xFFFU);
    bool cruise_cancel = GET_BIT(to_send, 56U) != 0U;

    if (subaru_longitudinal) {
      violation |= longitudinal_gas_checks(cruise_throttle, SUBARU_LONG_LIMITS);
    } else {
      // If openpilot is not controlling long, only allow ES_Distance for cruise cancel requests,
      // (when Cruise_Cancel is true, and Cruise_Throttle is inactive)
      violation |= (cruise_throttle != SUBARU_LONG_LIMITS.inactive_gas);
      violation |= (!cruise_cancel);
    }
  }

  // check es_status transmission_rpm limits
  if (addr == MSG_SUBARU_ES_Status) {
    int transmission_rpm = (GET_BYTES(to_send, 2, 2) & 0xFFFU);
    violation |= longitudinal_transmission_rpm_checks(transmission_rpm, SUBARU_LONG_LIMITS);
  }

  if (violation){
    tx = 0;
  }
  return tx;
}

static int subaru_fwd_hook(int bus_num, int addr) {
  int bus_fwd = -1;

  if (bus_num == SUBARU_MAIN_BUS) {
    bus_fwd = SUBARU_CAM_BUS;  // to the eyesight camera
  }

  if (bus_num == SUBARU_CAM_BUS) {
    // Global platform
    bool block_lkas = ((addr == MSG_SUBARU_ES_LKAS) ||
                       (addr == MSG_SUBARU_ES_DashStatus) ||
                       (addr == MSG_SUBARU_ES_LKAS_State) ||
                       (addr == MSG_SUBARU_ES_Infotainment));

    bool block_long = ((addr == MSG_SUBARU_ES_Brake) ||
                       (addr == MSG_SUBARU_ES_Distance) ||
                       (addr == MSG_SUBARU_ES_Status));

    bool block_msg = block_lkas || (subaru_longitudinal && block_long);
    if (!block_msg) {
      bus_fwd = SUBARU_MAIN_BUS;  // Main CAN
    }
  }

  return bus_fwd;
}

static const addr_checks* subaru_init(uint16_t param) {
  subaru_gen2 = GET_FLAG(param, SUBARU_PARAM_GEN2);

#ifdef ALLOW_DEBUG
  subaru_longitudinal = GET_FLAG(param, SUBARU_PARAM_LONGITUDINAL) && !subaru_gen2;
#endif

  if (subaru_gen2) {
    subaru_rx_checks = (addr_checks){subaru_gen2_addr_checks, SUBARU_GEN2_ADDR_CHECK_LEN};
  } else {
    subaru_rx_checks = (addr_checks){subaru_addr_checks, SUBARU_ADDR_CHECK_LEN};
  }

  return &subaru_rx_checks;
}

const safety_hooks subaru_hooks = {
  .init = subaru_init,
  .rx = subaru_rx_hook,
  .tx = subaru_tx_hook,
  .tx_lin = nooutput_tx_lin_hook,
  .fwd = subaru_fwd_hook,
};
