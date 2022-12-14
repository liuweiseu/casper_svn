module RAM512X18(
    RESET,
    RCLK, REN,
    PIPE, RW1, RW0,
    RADDR8, RADDR7, RADDR6, RADDR5, RADDR4, RADDR3, RADDR2, RADDR1, RADDR0,
    RD17, RD16, RD15, RD14, RD13, RD12, RD11, RD10, RD9, RD8, RD7,RD6, RD5, RD4, RD3, RD2, RD1, RD0,
    WCLK, WEN,
    WW1, WW0,
    WADDR8,WADDR7,WADDR6,WADDR5, WADDR4,WADDR3,WADDR2,WADDR1,WADDR0,
    WD17, WD16, WD15, WD14, WD13, WD12, WD11, WD10, WD9, WD8, WD7,WD6, WD5, WD4, WD3, WD2, WD1, WD0
    
  );
  input  RESET;
  input  RCLK, REN;
  input  PIPE, RW1, RW0;
  input  RADDR8, RADDR7, RADDR6, RADDR5, RADDR4, RADDR3, RADDR2, RADDR1, RADDR0;
  output RD17, RD16, RD15, RD14, RD13, RD12, RD11, RD10, RD9, RD8, RD7,RD6, RD5, RD4, RD3, RD2, RD1, RD0;
  input  WCLK, WEN;
  input  WW1, WW0;
  input  WADDR8,WADDR7,WADDR6,WADDR5, WADDR4,WADDR3,WADDR2,WADDR1,WADDR0;
  input  WD17, WD16, WD15, WD14, WD13, WD12, WD11, WD10, WD9, WD8, WD7,WD6, WD5, WD4, WD3, WD2, WD1, WD0;
endmodule
