onerror {resume}
quietly WaveActivateNextPane {} 0
add wave -noupdate -divider {System Level Ports}
add wave -noupdate -format Logic /bfm_system/sys_clk
add wave -noupdate -format Logic /bfm_system/sys_reset
add wave -noupdate -divider {PLB Bus Master Signals}
add wave -noupdate -format Logic /bfm_system/plb_bus_M_abort
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_M_ABus
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_M_BE
add wave -noupdate -format Logic /bfm_system/plb_bus_M_busLock
add wave -noupdate -format Logic /bfm_system/plb_bus_M_compress
add wave -noupdate -format Logic /bfm_system/plb_bus_M_guarded
add wave -noupdate -format Logic /bfm_system/plb_bus_M_lockErr
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_M_MSize
add wave -noupdate -format Logic /bfm_system/plb_bus_M_ordered
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_M_priority
add wave -noupdate -format Logic /bfm_system/plb_bus_M_rdBurst
add wave -noupdate -format Logic /bfm_system/plb_bus_M_request
add wave -noupdate -format Logic /bfm_system/plb_bus_M_RNW
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_M_size
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_M_type
add wave -noupdate -format Logic /bfm_system/plb_bus_M_wrBurst
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_M_wrDBus
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MBusy
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MErr
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MWrBTerm
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MWrDAck
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MAddrAck
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MRdBTerm
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MRdDAck
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_MRdDBus
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_MRdWdAddr
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_MRearbitrate
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_MSSize
add wave -noupdate -divider {PLB Bus Slave Signals}
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_addrAck
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_Sl_MBusy
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_Sl_MErr
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_rdBTerm
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_rdComp
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_rdDAck
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_Sl_rdDBus
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_Sl_rdWdAddr
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_rearbitrate
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_Sl_SSize
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_wait
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_wrBTerm
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_wrComp
add wave -noupdate -format Logic /bfm_system/plb_bus_Sl_wrDAck
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_abort
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_ABus
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_BE
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_busLock
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_compress
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_guarded
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_lockErr
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_masterID
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_MSize
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_ordered
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_PAValid
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_pendPri
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_pendReq
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_rdBurst
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_rdPrim
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_reqPri
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_RNW
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_SAValid
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_size
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_type
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_wrBurst
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/plb_bus_PLB_wrDBus
add wave -noupdate -format Logic /bfm_system/plb_bus_PLB_wrPrim
add wave -noupdate -divider {BFM Synch Bus Signals}
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/synch_bus/synch_bus/FROM_SYNCH_OUT
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/synch_bus/synch_bus/TO_SYNCH_IN
add wave -noupdate -divider {plb_sram Interface Signals}
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_Clk
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_Rst
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_addrAck
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/Sl_MBusy
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/Sl_MErr
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_rdBTerm
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_rdComp
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_rdDAck
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/Sl_rdDBus
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/Sl_rdWdAddr
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_rearbitrate
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/Sl_SSize
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_wait
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_wrBTerm
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_wrComp
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/Sl_wrDAck
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_abort
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_ABus
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_BE
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_busLock
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_compress
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_guarded
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_lockErr
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_masterID
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_MSize
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_ordered
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_PAValid
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_pendPri
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_pendReq
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_rdBurst
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_rdPrim
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_reqPri
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_RNW
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_SAValid
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_size
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_type
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_wrBurst
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/PLB_wrDBus
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/PLB_wrPrim
add wave -noupdate -divider {User Logic Interface Signals}
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_Clk
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_Reset
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_Addr
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_Burst
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_RNW
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_RdReq
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_WrReq
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_Retry
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_Error
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_ToutSup
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_AddrAck
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_Busy
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_RdAck
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_WrAck
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_ArData
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_ArBE
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/Bus2IP_ArCS
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/IP2Bus_ArData
add wave -noupdate -divider {User Logic Internal BRAM Signals}
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/ar_data_out
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/ar_address
add wave -noupdate -format Literal -radix hexadecimal /bfm_system/my_core/my_core/uut/user_logic_i/ar_select
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/ar_read_enable
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/ar_read_ack_dly1
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/ar_read_ack
add wave -noupdate -format Logic /bfm_system/my_core/my_core/uut/user_logic_i/ar_write_ack
TreeUpdate [SetDefaultTree]
WaveRestoreCursors {200 ns}
WaveRestoreZoom {0 ns} {2 us}
configure wave -namecolwidth 200
configure wave -valuecolwidth 100
configure wave -justifyvalue left
configure wave -signalnamewidth 1
configure wave -snapdistance 10
configure wave -datasetprefix 0
configure wave -rowmargin 4
configure wave -childrowmargin 2
