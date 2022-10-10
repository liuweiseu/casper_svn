%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%                                                                             %
%   Center for Astronomy Signal Processing and Electronics Research           %
%   http://seti.ssl.berkeley.edu/casper/                                      %
%   Copyright (C) 2006 University of California, Berkeley                     %
%                                                                             %
%   This program is free software; you can redistribute it and/or modify      %
%   it under the terms of the GNU General Public License as published by      %
%   the Free Software Foundation; either version 2 of the License, or         %
%   (at your option) any later version.                                       %
%                                                                             %
%   This program is distributed in the hope that it will be useful,           %
%   but WITHOUT ANY WARRANTY; without even the implied warranty of            %
%   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the             %
%   GNU General Public License for more details.                              %
%                                                                             %
%   You should have received a copy of the GNU General Public License along   %
%   with this program; if not, write to the Free Software Foundation, Inc.,   %
%   51 Franklin Street, Fifth Floor, Boston, MA 02110-1301 USA.               %
%                                                                             %
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

function bee_mc_gen(sys,xsg_ver,gate_clk)
%BEE_MC_GEN generates the necessary SHELL script to run the MC scripts.
%  bee_mc_gen(sys_name, xsg_ver, gate_clk)    'sys_name' is the Simulink system name.
%  'xsg_ver' is an optional flag, the valid values are '2.3', '3.1' When not specified,
%  V3.1 is assumed.
%  'gate_clk' is an optional flag, the valid values are 0 or 1 indicating
%  weather to use clock gating in MC. Default value is 0. When used,
%  'xsg_ver' flag is required.

if nargin < 3
    gate_clk = 0;
end

if nargin == 1
    disp('Warning: Xilinx System Generator version not specified, assuming V3.1');
    xsg_ver = '3.1';
else
    switch xsg_ver
        case {'2.3','2.2'}
            bee_mc_gen_old(sys,2,gate_clk);
        case '2.1'
            bee_mc_gen_old(sys,1,gate_clk);
        otherwise
            error(['Error: unknown version of Xilinx System Generator: ',xsg_ver]);
    end
end



if gate_clk 
    mc_flags = '-cg + -db + -gm - -logmode a -l mc_gen.log';
else
    mc_flags = '-cg - -db + -gm - -logmode a -l mc_gen.log';
end

bee_df_path = strrep(getenv('BEE_DF_PATH'),'\','/');
if exist(bee_df_path,'dir')
    tcl_lib_path = [bee_df_path,'/lib/tcl'];
else
    error('System environment variable BEE_DF_PATH need to be properly defined to point to the root directory of the BEE Design Flow installation');
end

if exist([bee_df_path,'/conf/env.mat'])
    eval(['load ',bee_df_path,'/conf/env.mat;']);
else
    disp(['Warning: cannont find env.mat file, assuming mc_lib_path = /tools/designs/BEE/lib/mc']);
    mc_lib_path = '/tools/designs/BEE/lib/mc';
end

[root,root_dir,glb_ce,glb_clr,clk_period] = get_xsg_info(sys);
[glb_root,R] = strtok(root,'/');

if exist([root_dir,'/MC']) ~= 7
    mkdir(root_dir,'MC');
end

load xilinx_coregen_types.mat;

core_fid = fopen([root_dir,'/corework/sysgen.sgxco'],'r');
mc_fid = fopen([root_dir,'/MC/mc_script.sh'],'w');
blk_type = '';
cur_params = [];
while 1
    line = fgetl(core_fid);
    if ~ischar(line)
        break;
    else
        if isempty(line) | strcmp(line(1),'#')
            continue;
        end
        [cmd, R] = strtok(line);
        switch cmd
            case {'SETPROJECT' 'SET'}
                continue;
            case 'SELECT'
                blk_type = clear_name(deblank(R(2:length(R))));
                if isfield(coregen_types,blk_type)
                    blk_type = getfield(coregen_types,blk_type);
                else
                    error(['Unknown coregen core type: ',R(2:length(R))]);
                end
                disp(blk_type);
            case 'CSET'
                [var_name, R] = strtok(R);
                [R, var_value] = strtok(R);
                var_value = deblank(var_value(2:length(var_value)));
                eval(['cur_param_list = ',blk_type,';']);
                if isfield(cur_param_list,var_name)
                    value_template = getfield(cur_param_list,var_name);
                    if length(fields(value_template)) == 1
                        cur_params = setfield(cur_params,value_template.alias,var_value);
                    else
                        var_name = value_template.alias;
                        if isfield(value_template,var_value)
                            var_value = getfield(value_template,var_value);
                            cur_params = setfield(cur_params,var_name,var_value);
                        else
                            error(['Unknown parameter valur emulation for core type "',blk_type,'": ',var_name,',',var_value]);
                        end
                    end
                else
                    continue;
                end
            case 'GENERATE'
                switch blk_type
                    case 'xlcounter'
                        if strcmp(cur_params.count_mode,'2')
                            cur_params.has_up = '1';
                        else
                            cur_params.has_up = '0';
                        end
                end
                disp(cur_params);
                params = fields(cur_params);
                params_str = ' -par ';
                for i = 1:length(params)
                    params_str = [params_str,params{i},'=',getfield(cur_params,params{i}),','];
                end
                params_str = [params_str(1:length(params_str)-1),' -i ',mc_lib_path,'/',blk_type,'.mcl'];
                fprintf(mc_fid,['mc ',mc_flags,' ',params_str,'\n']);
                
                clear blk_type, cur_param_list;      
                cur_params = [];
            otherwise
                error(['Unknown line format: ',line]);
        end
    end
end
%     switch blk_type
%         case {'xlmux' 'xllogical' 'xlddsreg' 'xlddsscmodereg'}
%             ports = get_param(blk,'inputs');
%             mc_args = ['num_inputs=',ports,','];
%         case 'xlfir'
%             chs = get_param(blk,'num_channels');
%             mc_args = ['num_channels=',chs,','];
%             coef = get_blk_wsvar(blk,'coef');
%             n_bits = get_blk_wsvar(blk,'coef_n_bits');
%             bin_pt = get_blk_wsvar(blk,'coef_bin_pt');
%             for i = 1:length(coef)                
%                 mc_args = [mc_args, 'c',num2str(i-1),'=',fix2string(coef(i),n_bits,bin_pt,1,1),','];
%             end
%         case 'xlcounter'
%             if xsg_ver == 1 & strcmp(get_param(blk,'reset_pin'),'on')
%                 blk_type = 'xlcounter_rst';
%             elseif xsg_ver == 2
%                 switch get_param(blk,'cnt_type')
%                     case 'Free Running'
%                         blk_type = 'xlcounter_free';
%                     case 'Count Limited'
%                         blk_type = 'xlcounter_limit';
%                 end
%             end
%             n_bits = get_param(blk,'n_bits');
%             cnt_by_val = get_param(blk,'cnt_by_val');
%             start_count = get_param(blk,'start_count');
%             mc_args = ['width=',n_bits,',count_by=',cnt_by_val,',ainit_val=',start_count,',sinit_val=',start_count,','];
%         case 'xlmult' 
%             user_latency = get_blk_wsvar(blk,'latency');
%             if strcmp(get_param(blk,'pipeline'),'on')
%                 inports = get_blk_inports(blk);
%                 min_width = min(get_port_bit_width(inports(1)),get_port_bit_width(inports(2)));
%                 if min_width < 3
%                     latency = 0;
%                 elseif min_width < 5
%                     latency = 1;
%                 elseif min_width < 9
%                     latency = 2;
%                 elseif min_width < 17
%                     latency = 3;
%                 elseif min_width < 34
%                     latency = 4;
%                 elseif min_width < 65
%                     latency = 5;
%                 else
%                     error(['Invalid bit_width: ',num2str(min_width)]);
%                 end
%             else
%                 latency = '0';
%             end
%             mc_args = ['latency=',num2str(min(latency+2,user_latency)),','];
%         otherwise
%             mc_args = '';
%     end
    
fclose(core_fid);
fclose(mc_fid);