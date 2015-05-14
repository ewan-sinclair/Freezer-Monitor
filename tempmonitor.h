#define SET_TEMP -20

#define FUKT_TEMP 0
#define FUKT_TIME_MINS 1

#define WARN_TEMP -10
#define WARN_TIME_MINS 30

enum pins {
  PWR = 0,
  PWR_INTERRUPT = 2,
  WARN = 3,
  P_FUKD = 4,
  RESET_PWR_INTERRUPT = 5,
  TEMP_PROBE = 6
};
