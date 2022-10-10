#include "edtinc.h"

#include "lib_adt7461.h"


#define DEBUG_PRINT 

/**/
/* IO to chip pins*/
/**/

/* int temp_clock = 0, temp_data = 1, temp_den = 0;*/

void
edt_adt7461_set_clock(EdtDev *edt_p, int clock)
{
	int val = edt_reg_read(edt_p, EDT_ADT7461_REG) & ~EDT_ADT7461_CLK;
	edt_reg_write(edt_p, EDT_ADT7461_REG, (val | ((clock)?EDT_ADT7461_CLK:0)));
}

void
edt_adt7461_set_data(EdtDev *edt_p, int data)
{
	int val = edt_reg_read(edt_p, EDT_ADT7461_REG) & ~EDT_ADT7461_DATA;
	edt_reg_write(edt_p, EDT_ADT7461_REG, (val | ((data)?EDT_ADT7461_DATA:0)));
}

void
edt_adt7461_set_den(EdtDev *edt_p, int enable)
{
	int val = edt_reg_read(edt_p, EDT_ADT7461_REG) & ~EDT_ADT7461_TRISTATE;
	edt_reg_write(edt_p, EDT_ADT7461_REG, (val | ((!enable)?EDT_ADT7461_TRISTATE:0)));
}

void
edt_adt7461_show_bits(EdtDev *edt_p)

{
	int val = edt_reg_read(edt_p, EDT_ADT7461_REG);

	printf("%s %s %s\n", 
		(val & EDT_ADT7461_TRISTATE)? "DEN":"   ",
		(val & EDT_ADT7461_DATA)?"DATA":"    ",
		(val & EDT_ADT7461_CLK)?"CLK":"   ");

}

void
edt_adt7461_set(EdtDev *edt_p, int enable, int data, int clock )
{
	int rval = edt_reg_read(edt_p, EDT_ADT7461_REG) & 
		~(EDT_ADT7461_TRISTATE|EDT_ADT7461_CLK|EDT_ADT7461_DATA) ;

	int val  = ((clock)  ? EDT_ADT7461_CLK : 0)
		| ((data) ? EDT_ADT7461_DATA : 0) | ((enable) ? 0 : EDT_ADT7461_TRISTATE);


	edt_reg_write(edt_p, EDT_ADT7461_REG, val | rval);


/*	edt_adt7461_show_bits(edt_p);*/


}


int
edt_adt7461_get_therm_reset(EdtDev *edt_p)

{
	int val;
	val = edt_reg_read(edt_p, EDT_ADT7461_REG) & EDT_ADT7461_THERM_RESET;
	return (val != 0);
}

void
edt_adt7461_set_therm_reset(EdtDev *edt_p, int enable)

{
	int val = edt_reg_read(edt_p, EDT_ADT7461_REG) & ~EDT_ADT7461_THERM_RESET;
	edt_reg_write(edt_p, EDT_ADT7461_REG, val | (enable)?EDT_ADT7461_THERM_RESET:0);
}

int
edt_adt7461_read_alert_pin(EdtDev *edt_p)
{
	if ((edt_reg_read(edt_p, EDT_ADT7461_REG) & EDT_ADT7461_ALERT) != 0) {
		return 1;
	} else {
		return 0;
	}
}


int
edt_adt7461_read_data_pin(EdtDev *edt_p)
{
	int val;
	val = edt_reg_read(edt_p, EDT_ADT7461_REG) & EDT_ADT7461_DATA;
	DEBUG_PRINT("read_data_pin %x\n", val);
	return (val != 0);
}

int
edt_adt7461_read_therm_pin(EdtDev *edt_p)
{
	int val;
	val = edt_reg_read(edt_p, EDT_ADT7461_REG) & EDT_ADT7461_THERM;
	DEBUG_PRINT("read_data_pin %x\n", val);
	return (val != 0);
}

/**/
/* Basic communication parts*/
/**/

void
edt_adt7461_send_bus_reset(EdtDev *edt_p) 
{
	/*send low to hi data transition while clock is hi*/
	DEBUG_PRINT("Send Bus Reset\n");
 	edt_adt7461_set_den(edt_p, 1);
	edt_adt7461_set_clock(edt_p,1);
	edt_adt7461_set_data(edt_p,1);
	edt_adt7461_set_data(edt_p,0);



}

void
edt_adt7461_send_bus_start(EdtDev *edt_p)

{
	/*assuming clock starts hi*/
	/*send hi to low data transition while clock is hi*/
	DEBUG_PRINT("Send Bus Start\n");


	edt_adt7461_set_den(edt_p,1);
	edt_adt7461_set_data(edt_p,1);

	edt_adt7461_set_data(edt_p,0);

	edt_adt7461_set_clock(edt_p,0);



}

void
edt_adt7461_send_bus_stop(EdtDev *edt_p)

{
	/*assuming clock starts low*/
	/*send low to hi data transition while clock is hi*/
	DEBUG_PRINT("Send Bus Stop\n");
 	edt_adt7461_set_clock(edt_p,1);

	edt_adt7461_set_data(edt_p,0);
	edt_adt7461_set_data(edt_p,1);


}

int
edt_adt7461_get_ack(EdtDev *edt_p)

{
    int rc = 1;
	DEBUG_PRINT("Get Ack\n");
	/*check for acknowledge from slave (pulls data line low)*/

	edt_adt7461_set_den(edt_p,0);
	edt_adt7461_set_clock(edt_p, 0);
	edt_adt7461_set_clock(edt_p, 1);

/*    edt_adt7461_set(edt_p,0,0,0); //disable data output*/

   /* edt_adt7461_set(edt_p,0,0,1); //clock high*/
	if (edt_adt7461_read_data_pin(edt_p)) {
		rc = 0; /*data is hi - no acknowledge*/
	}

	edt_adt7461_set_clock(edt_p, 0);

 /*   edt_adt7461_set(edt_p,0,0,0); //clock low*/

    return rc;
}


int
edt_adt7461_send_data_byte(EdtDev *edt_p, int data)

{
	/*assuming clock starts low*/
	int i;
	/*write out one bit x8*/
    int dbit;
	DEBUG_PRINT("Send Data Byte %02x\n", data);

	edt_adt7461_set_den(edt_p, 1);
	edt_adt7461_set_clock(edt_p, 0);

	for (i = 0; i < 8; i++) {
		dbit  = data & 0x80;
        edt_adt7461_set_data(edt_p, dbit);;
        edt_adt7461_set_clock(edt_p, 1);
		edt_adt7461_set_clock(edt_p, 0);

		data = data << 1;
	}

    if (!edt_adt7461_get_ack(edt_p))
        return -1;

	return 0;
}

int
edt_adt7461_get_data_byte(EdtDev *edt_p, int doAcknowledge)

{
	/*assuming clock starts low*/
	int i;
	int data = 0;
	/*read in one bit x8*/
	DEBUG_PRINT("Get Data Byte %s\n", (doAcknowledge)?"Ack" : "");

	edt_adt7461_set_den(edt_p,0);
	edt_adt7461_set_clock(edt_p, 0);

	for (i = 0; i < 8; i++) {
		edt_adt7461_set_clock(edt_p,1); /*clock goes high*/
		data = (data << 1) | ( edt_adt7461_read_data_pin(edt_p) ? 0x01 : 0x00 ); /*data bit is read*/
		edt_adt7461_set_clock(edt_p,0); /*clock goes high*/
	}
	if (doAcknowledge) {

        if (!edt_adt7461_get_ack(edt_p))
            return -1;
        else
		    return data;
	} else {
		edt_adt7461_set_den(edt_p,1);
		edt_adt7461_set_data(edt_p, 1);
		/*send no acknowledge to slave (pull data line hi)*/
		edt_adt7461_set_clock(edt_p,1); /*clock goes high*/
		edt_adt7461_set_clock(edt_p,0); /*clock goes low*/

		return data;
	}
}

/**/
/* Full register read/write commands*/
/**/

int
edt_adt7461_reg_read(EdtDev *edt_p, int address)

{
	int status;
	int data;
	switch (0) {
		case 0:
		/*send start condition*/
		edt_adt7461_send_bus_start(edt_p);
		/*send address of temperature controller*/
		status = edt_adt7461_send_data_byte(edt_p, (EDT_ADT7461_DEVICE_ADDR << 1) | 0x00);
		if (status == -1) break;
		/*send register address for read*/
		status = edt_adt7461_send_data_byte(edt_p, address);
		if (status == -1) break;
		/*send stop condition*/
		edt_adt7461_send_bus_stop(edt_p);
		/*send start condition*/
		edt_adt7461_send_bus_start(edt_p);
		/*send address of temperature controller*/
		status = edt_adt7461_send_data_byte(edt_p, (EDT_ADT7461_DEVICE_ADDR << 1) | 0x01);
		if (status == -1) break;
		/*read data for read*/
		data = edt_adt7461_get_data_byte(edt_p, 0);
		if (data == -1) break;
		/*send stop condition*/
		edt_adt7461_send_bus_stop(edt_p);
		return data;
	}
	/*reset bus*/
	edt_adt7461_send_bus_reset(edt_p);
	return -1;	/*transaction failed*/
}

int
edt_adt7461_reg_write(EdtDev *edt_p, int address, int data)

{
	int status;
	switch (0) {
		case 0:
		/*send start condition*/
		edt_adt7461_send_bus_start(edt_p);
		/*send address of temperature controller*/
		status = edt_adt7461_send_data_byte(edt_p, (EDT_ADT7461_DEVICE_ADDR << 1) | 0x00);
		if (status == -1) break;
		/*send register address for write*/
		status = edt_adt7461_send_data_byte(edt_p, address);
		if (status == -1) break;
		/*send data for write*/
		status = edt_adt7461_send_data_byte(edt_p, data);
		if (status == -1) break;
		/*send stop condition*/
		edt_adt7461_send_bus_stop(edt_p);
		return 0;
	}
	/*reset bus*/
	edt_adt7461_send_bus_reset(edt_p);
	return -1;	/*transaction failed*/
}

int
edt_adt7461_read_alert_address(EdtDev *edt_p)

{
	int status;
	int deviceAddress;
	switch (0) {
		case 0:
		/*send start condition*/
		edt_adt7461_send_bus_start(edt_p);
		/*send address of temperature controller*/
		status = edt_adt7461_send_data_byte(edt_p, (EDT_ADT7461_ALERT_RESPONSE_ADDR << 1) | 0x01);
		if (status == -1) break;
		/*read address of alert-causing device*/
		deviceAddress = edt_adt7461_get_data_byte(edt_p, 0);
		if (deviceAddress == -1) break;
		/*send stop condition*/
		edt_adt7461_send_bus_stop(edt_p);
		return deviceAddress & 0xFE;
	}
	/*reset bus*/
	edt_adt7461_send_bus_reset(edt_p);
	return -1;	/*transaction failed*/
}
/**/
/* Main*/
/**/


/* returns 10-bit temperature*/
/* with two bits for fractional degree*/
int
edt_adt7461_read_extern(EdtDev *edt_p, int tenbits)

{

    int low, high;

    low = edt_adt7461_reg_read(edt_p,EDT_ADT7461_EXT_TEMP_LOW_R);
    high = edt_adt7461_reg_read(edt_p,EDT_ADT7461_EXT_TEMP_R);

    if (tenbits)
		return (high << 2) | (low >> 6);
	else
		return high;
}

int
edt_adt7461_read_intern(EdtDev *edt_p)

{
    return edt_adt7461_reg_read(edt_p,EDT_ADT7461_INTERN_TEMP_R);
}

void
edt_adt7461_set_alert_high(EdtDev *edt_p, int alert_high)

{	
	edt_adt7461_reg_write(edt_p, EDT_ADT7461_EXT_TEMP_HIGH_W, alert_high);
}

int
edt_adt7461_get_alert_high(EdtDev *edt_p)

{	
	return edt_adt7461_reg_read(edt_p, EDT_ADT7461_EXT_TEMP_HIGH_R);
}
void
edt_adt7461_set_alert_low(EdtDev *edt_p, int alert_low)

{	
	edt_adt7461_reg_write(edt_p, EDT_ADT7461_EXT_TEMP_LOW_W, alert_low);
}

int
edt_adt7461_get_alert_low(EdtDev *edt_p)

{	
	return edt_adt7461_reg_read(edt_p, EDT_ADT7461_EXT_TEMP_LOW_R);
}

void
edt_adt7461_set_therm(EdtDev *edt_p, int therm_high)

{
		
	edt_adt7461_reg_write(edt_p, EDT_ADT7461_EXT_THERM_W, therm_high);


}

int
edt_adt7461_get_therm(EdtDev *edt_p)

{
		
	return edt_adt7461_reg_read(edt_p, EDT_ADT7461_EXT_THERM_W);


}

void
edt_adt7461_set_therm_hysteresis(EdtDev *edt_p, int hysteresis)

{

		edt_adt7461_reg_write(edt_p, EDT_ADT7461_THERM_HYSTERESIS, hysteresis);

}

int
edt_adt7461_get_therm_hysteresis(EdtDev *edt_p)

{

	return	edt_adt7461_reg_read(edt_p, EDT_ADT7461_THERM_HYSTERESIS);

}

int
edt_adt7461_get_alert(EdtDev *edt_p)

{
	int alert;

	alert = edt_adt7461_read_alert_pin(edt_p);
		
	/* clear */

	if (alert)
		edt_adt7461_read_alert_address(edt_p);

	return alert;
}

void
edt_adt4761_print_status(EdtDev *edt_p, int full)

{

	int alert;
	int external;
	int internal;
	int therm;
	

	alert = edt_adt7461_get_alert(edt_p);
	therm = edt_adt7461_read_therm_pin(edt_p);
	external = edt_adt7461_read_extern(edt_p, TRUE);
	internal = edt_adt7461_read_intern(edt_p);
	printf("Ext %d.%02d Int %i", external >> 2, (external & 3) * 25, internal);

	if (full)
	{


		printf(" Alert min %i max %i",
			edt_adt7461_get_alert_low(edt_p),
			edt_adt7461_get_alert_high(edt_p));
		
		printf(" Therm %d Hyster %d", 
			edt_adt7461_get_therm(edt_p),
			edt_adt7461_get_therm_hysteresis(edt_p));

		printf(" Stat 0x%02x T Reset %s", 
			edt_adt7461_reg_read(edt_p, EDT_ADT7461_STATUS_REG_R),
			(edt_adt7461_get_therm_reset(edt_p)) ? "On ":"Off"
			);
	
	
		
	}
	if (alert) {
		printf(" ALERT");			
	}
	if (therm) {
		printf(" THERM");			
	}

	printf("\n");
}

