1. Before setting item's value, please first use ADL2_Overdrive_Caps ADL call to check its capability. Only the visible item could be set.
2. The latest driver introduced Fan Curve feature and when it is available the legacy Fan controls will disabled.
3. When setting FAN Curve, please follow below rules:
    - OD8_FAN_CURVE_TEMPERATURE_n-1 <= OD8_FAN_CURVE_TEMPERATURE_n <= OD8_FAN_CURVE_TEMPERATURE_n+1
    - OD8_FAN_CURVE_SPEED_n-1 <= OD8_FAN_CURVE_SPEED_n <= OD8_FAN_CURVE_SPEED_n+1