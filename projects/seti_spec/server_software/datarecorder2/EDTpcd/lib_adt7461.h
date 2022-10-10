
/*******************************************************8
 * Definitions for accessing the ADT 7461 temperature chip
 * Functions are in lib_adt7461.c
 *
 * Created : 10/24/2004 jsc
 * Copyright 2004 EDT
 *
 *******************************************************/
#ifndef LIB_ADT7461_H
#define LIB_ADT7461_H

#define EDT_ADT7461_REG		0x01000088
/* bit definitions for EDT_ADT7461_REG */

#define EDT_ADT7461_CLK		0x1
#define EDT_ADT7461_DATA		0x2
#define EDT_ADT7461_TRISTATE	0x4
#define EDT_ADT7461_THERM_RESET	0x8
#define EDT_ADT7461_ALERT		0x10
#define EDT_ADT7461_THERM		0x20

/* register addresses on the ADT7461 */

#define EDT_ADT7461_DEVICE_ADDR			0x4C
#define EDT_ADT7461_ALERT_RESPONSE_ADDR	0x0C
#define EDT_ADT7461_INTERN_TEMP_R		0x00
#define EDT_ADT7461_EXT_TEMP_R		0x01
#define EDT_ADT7461_EXT_TEMP_LOW_BYTE_R		0x10
#define EDT_ADT7461_STATUS_REG_R			0x02
#define EDT_ADT7461_CFG_R                0x03
#define EDT_ADT7461_CFG_W                0x09

#define EDT_ADT7461_EXT_TEMP_HIGH_R	0x07
#define EDT_ADT7461_EXT_TEMP_LOW_R	0x08

#define EDT_ADT7461_EXT_TEMP_HIGH_W	0x0D
#define EDT_ADT7461_EXT_TEMP_LOW_W	0x0E

#define EDT_ADT7461_EXT_THERM_R    0x19
#define EDT_ADT7461_EXT_THERM_W    0x19

#define EDT_ADT7461_THERM_HYSTERESIS 0x21


EDTAPI void edt_adt7461_set_clock(EdtDev *edt_p, int clock);

EDTAPI void edt_adt7461_set_data(EdtDev *edt_p, int data);

EDTAPI void edt_adt7461_set_den(EdtDev *edt_p, int enable);

EDTAPI void edt_adt7461_show_bits(EdtDev *edt_p);

EDTAPI void edt_adt7461_set(EdtDev *edt_p, int enable, int data, int clock );

EDTAPI int edt_adt7461_get_therm_reset(EdtDev *edt_p);

EDTAPI void edt_adt7461_set_therm_reset(EdtDev *edt_p, int enable);

EDTAPI int edt_adt7461_read_alert_pin(EdtDev *edt_p);

EDTAPI int edt_adt7461_read_data_pin(EdtDev *edt_p);

EDTAPI int edt_adt7461_read_therm_pin(EdtDev *edt_p);

EDTAPI void edt_adt7461_send_bus_reset(EdtDev *edt_p) ;

EDTAPI void edt_adt7461_send_bus_start(EdtDev *edt_p);

EDTAPI void edt_adt7461_send_bus_stop(EdtDev *edt_p);

EDTAPI int edt_adt7461_get_ack(EdtDev *edt_p);

EDTAPI int edt_adt7461_send_data_byte(EdtDev *edt_p, int data);

EDTAPI int edt_adt7461_get_data_byte(EdtDev *edt_p, int doAcknowledge);

EDTAPI int edt_adt7461_reg_read(EdtDev *edt_p, int address);

EDTAPI int edt_adt7461_reg_write(EdtDev *edt_p, int address, int data);

EDTAPI int edt_adt7461_read_alert_address(EdtDev *edt_p);


EDTAPI int edt_adt7461_read_extern(EdtDev *edt_p, int tenbits);

EDTAPI int edt_adt7461_read_intern(EdtDev *edt_p);

EDTAPI void edt_adt7461_set_alert_high(EdtDev *edt_p, int alert_high);


EDTAPI int edt_adt7461_get_alert_high(EdtDev *edt_p);

EDTAPI void edt_adt7461_set_alert_low(EdtDev *edt_p, int alert_low);

EDTAPI int edt_adt7461_get_alert_low(EdtDev *edt_p);

EDTAPI void edt_adt7461_set_therm(EdtDev *edt_p, int therm_high);


EDTAPI int edt_adt7461_get_therm(EdtDev *edt_p);


EDTAPI void edt_adt7461_set_therm_hysteresis(EdtDev *edt_p, int hysteresis);


EDTAPI int edt_adt7461_get_therm_hysteresis(EdtDev *edt_p);


EDTAPI int edt_adt7461_get_alert(EdtDev *edt_p);

EDTAPI void edt_adt4761_print_status(EdtDev *edt_p, int full);


#endif
