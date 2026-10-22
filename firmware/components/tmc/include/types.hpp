#pragma once

#include <cstdint>

struct TmcIOIN_t {
  uint32_t raw;

  bool enn() const { return (raw >> 0) & 1; }
  bool ms1() const { return (raw >> 2) & 1; }
  bool ms2() const { return (raw >> 3) & 1; }
  bool diag() const { return (raw >> 4) & 1; }
  bool pdnUart() const { return (raw >> 6) & 1; }
  bool step() const { return (raw >> 7) & 1; }
  bool spreadEn() const { return (raw >> 8) & 1; }
  bool dir() const { return (raw >> 9) & 1; }
  uint8_t version() const { return static_cast<uint8_t>(raw >> 24); }
};

struct Gconf_t {
  bool i_scale_analog;
  bool internal_rsense;
  bool en_spread_cycle;
  bool shaft;
  bool index_otpw;
  bool index_step;
  bool pdn_disable;
  bool mstep_reg_select;
  bool multistep_filt;
  bool test_mode;

  operator uint32_t() const {
    uint32_t raw = 0;

    if (i_scale_analog) raw |= (1U << 0);
    if (internal_rsense) raw |= (1U << 1);
    if (en_spread_cycle) raw |= (1U << 2);
    if (shaft) raw |= (1U << 3);
    if (index_otpw) raw |= (1U << 4);
    if (index_step) raw |= (1U << 5);
    if (pdn_disable) raw |= (1U << 6);
    if (mstep_reg_select) raw |= (1U << 7);
    if (multistep_filt) raw |= (1U << 8);
    if (test_mode) raw |= (1U << 9);

    return raw;
  }
};

struct IHoldIRun_t {
  uint8_t i_hold;
  uint8_t i_run;
  uint8_t i_hold_delay;

  operator uint32_t() const {
    uint32_t raw = 0;

    raw |= (static_cast<uint32_t>(i_hold & 0x1F) << 0);
    raw |= (static_cast<uint32_t>(i_run & 0x1F) << 8);
    raw |= (static_cast<uint32_t>(i_hold_delay & 0x0F) << 16);

    return raw;
  }
};

struct ChopConf_t {
  bool diss2vs;
  bool diss2g;
  bool dedge;
  bool intpol;
  bool mres3;
  bool mres2;
  bool mres1;
  bool mres0;
  bool vsense;
  bool tbl1;
  bool tbl0;
  bool hend3;
  bool hend2;
  bool hend1;
  bool hend0;
  bool hstrt2;  // Wow great naming
  bool hstrt1;
  bool hstrt0;
  bool toff3;
  bool toff2;
  bool toff1;
  bool toff0;

  operator uint32_t() const {
    uint32_t raw = 0;

    if (diss2vs) raw |= (1U << 31);
    if (diss2g) raw |= (1U << 30);
    if (dedge) raw |= (1U << 29);
    if (intpol) raw |= (1U << 28);
    if (mres3) raw |= (1U << 27);
    if (mres2) raw |= (1U << 26);
    if (mres1) raw |= (1U << 25);
    if (mres0) raw |= (1U << 24);
    if (vsense) raw |= (1U << 17);
    if (tbl1) raw |= (1U << 16);
    if (tbl0) raw |= (1U << 15);
    if (hend3) raw |= (1U << 10);
    if (hend2) raw |= (1U << 9);
    if (hend1) raw |= (1U << 8);
    if (hend0) raw |= (1U << 7);
    if (hstrt2) raw |= (1U << 6);
    if (hstrt1) raw |= (1U << 5);
    if (hstrt0) raw |= (1U << 4);
    if (toff3) raw |= (1U << 3);
    if (toff2) raw |= (1U << 2);
    if (toff1) raw |= (1U << 1);
    if (toff0) raw |= (1U << 0);

    return raw;
  }
};
