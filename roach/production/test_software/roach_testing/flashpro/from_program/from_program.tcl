new_project -name tester -location deleteme -mode single 
scan_chain_prg -name {07156}
set_programming_file -file {from_config.pdb}
set_programming_action -action {PROGRAM_FROM}
update_programming_file \
-feature {setup_security} \
-feature {prog_from} \
-from_content {ufc} \
-from_config_file {roach_monitor.ufc} \
-number_of_devices {1} \
-from_program_pages {0 1 2 3 4 5 6 7}
run_selected_actions
close_project
