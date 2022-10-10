new_project -name tester -location deleteme -mode single 
scan_chain_prg -name {07156}
set_programming_file -file {fpga_program.stp}
set_programming_action -action {ERASE_ALL}
set_programming_action -action {PROGRAM}
run_selected_actions
close_project
