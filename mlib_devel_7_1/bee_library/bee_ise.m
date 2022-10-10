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

function varargout = BEE_ise(varargin)
% BEE_ISE BEE Xilinx ISE Batch Tools GUI

% Last Modified by GUIDE v2.5 17-Oct-2002 16:27:57

if nargin == 0  % LAUNCH GUI

    fig = openfig(mfilename,'reuse');

    % Use system color scheme for figure:
    set(fig,'Color',get(0,'defaultUicontrolBackgroundColor'));

    % Generate a structure of handles to pass to callbacks, and store it.
    handles = guihandles(fig);
    guidata(fig, handles);

    if nargout > 0
        varargout{1} = fig;
    end
    if ~isempty(gcs)
        set(handles.design_name,'String',gcs);
    end


    %     set(handles.syn_opt,'Value',3);
    %     set(handles.fpga_chip,'Value',2);
    %     set(handles.xsg_version,'Value',2);
    %     set(handles.clk_pin,'String','D21');


    try
        xsg = xlversion;
        xsg = strtok(xsg{1},' ');
        switch xsg
            case {'6.2.R14' '6.2'}
                set(handles.xsg_version,'Value',6);
            case {'6.1' '6.1.1'}
                set(handles.xsg_version,'Value',5);
            case '3.1'
                set(handles.xsg_version,'Value',4);
            case '2.3'
                set(handles.xsg_version,'Value',3);
        end
    end

elseif ischar(varargin{1}) % INVOKE NAMED SUBFUNCTION OR CALLBACK

    try
        if (nargout)
            [varargout{1:nargout}] = feval(varargin{:}); % FEVAL switchyard
        else
            feval(varargin{:}); % FEVAL switchyard
        end
    catch
        disp(lasterr);
    end

end


%| ABOUT CALLBACKS:
%| GUIDE automatically appends subfunction prototypes to this file, and
%| sets objects' callback properties to call them through the FEVAL
%| switchyard above. This comment describes that mechanism.
%|
%| Each callback subfunction declaration has the following form:
%| <SUBFUNCTION_NAME>(H, EVENTDATA, HANDLES, VARARGIN)
%|
%| The subfunction name is composed using the object's Tag and the
%| callback type separated by '_', e.g. 'slider2_Callback',
%| 'figure1_CloseRequestFcn', 'axis1_ButtondownFcn'.
%|
%| H is the callback object's handle (obtained using GCBO).
%|
%| EVENTDATA is empty, but reserved for future use.
%|
%| HANDLES is a structure containing handles of components in GUI using
%| tags as fieldnames, e.g. handles.figure1, handles.slider2. This
%| structure is created at GUI startup using GUIHANDLES and stored in
%| the figure's application data using GUIDATA. A copy of the structure
%| is passed to each callback.  You can store additional information in
%| this structure at GUI startup, and you can change the structure
%| during callbacks.  Call guidata(h, handles) after changing your
%| copy to replace the stored original so that subsequent callbacks see
%| the updates. Type "help guihandles" and "help guidata" for more
%| information.
%|
%| VARARGIN contains any extra arguments you have passed to the
%| callback. Specify the extra arguments by editing the callback
%| property in the inspector. By default, GUIDE sets the property to:
%| <MFILENAME>('<SUBFUNCTION_NAME>', gcbo, [], guidata(gcbo))
%| Add any extra arguments after the last argument, before the final
%| closing parenthesis.



% --------------------------------------------------------------------
function varargout = listbox1_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = run_Callback(h, eventdata, handles, varargin)
design_name = get(handles.design_name,'String');
syn_opt = get(handles.syn_opt,'Value');
imp_opt = get(handles.imp_opt,'Value');
conf_opt = get(handles.conf_opt,'Value');
design_flow = get(handles.design_flow,'Value');
fpga_chip = get(handles.fpga_chip,'Value');
clk_pin = get(handles.clk_pin,'String');
clk_pad_value = get(handles.clkpad,'Value');
assign_io = get(handles.assign_io,'Value');
xsg_version = get(handles.xsg_version,'Value');
use_lvds_clk = get(handles.lvds_clock,'Value');

use_synplify_pro = 0;

has_bee_io_blk = 0;
bee_io_blk_vhdl_file = 'sram_ctrl.vhd';
% bee_io_blk_prefix = '';

% if exist(getenv('BEE_DF_PATH'),'dir')
%     vhdl_lib_path = [getenv('BEE_DF_PATH'),'/lib/vhdl'];
% else
% %     errordlg('System environment variable BEE_DF_PATH need to be properly defined to point to the root directory of the BEE Design Flow installation');
% %     return;
% end

%Synplify Pro default options

technology = 'VIRTEX-E';
part = 'XCV2000E';
package = 'FG680';
speed_grade = '-6';
default_enum_encoding = 'sequential';
symbolic_fsm_compiler = 'false';
resource_sharing = 'true';
frequency = '50';
fanout_limit = '1000';
maxfan_hard = 'false';
disable_io_insertion = 'false';
write_verilog = 'false';
write_vhdl = 'false';
write_apr_constraint = 'true';
mti_root = '""';




[root,root_dir,glb_ce,glb_clr,clk_period] = get_xsg_info(design_name);
frequency = sprintf('%3.3f',10^3/str2num(clk_period));
root_clr = clear_name(root);


switch syn_opt
    case 1
        if xsg_version >= 3
            syn_str = '-synth xst_vhdl.opt ';
        else
            syn_str = '-synth vhdl_speed.opt ';
        end
    case 2
        syn_str = '';
        use_synplify_pro = 1;
    otherwise
        errordlg('Invalid Synthesis Option');
        return;
end

switch imp_opt
    case 1
        imp_str = '-implement fast_runtime.opt ';
    case 2
        imp_str = '-implement balanced.opt ';
    case 3
        imp_str = '-implement high_effort.opt ';
    otherwise
        errordlg('Invalid Implementation Option');
        return;
end

switch conf_opt
    case 1
        conf_str = '-config bitgen.opt ';
    case 2
        conf_str = '-config bitgen_jtag.opt ';
    otherwise
        errordlg('Invalid Configure Option');
        return;
end

switch fpga_chip
    case 1
        chip_str = '-p xcv600e-6-hq240 ';
        technology = 'VIRTEX-E';
        part = 'XCV600E';
        package = 'HQ240';
        speed_grade = '-6';
    case 2
        chip_str = '-p xcv2000e-6-fg680 ';
        technology = 'VIRTEX-E';
        part = 'XCV2000E';
        package = 'FG680';
        speed_grade = '-6';
    case 3
        chip_str = '-p xcv200e-6-fg256 ';
        technology = 'VIRTEX-E';
        part = 'XCV200E';
        package = 'FG256';
        speed_grade = '-6';
    case 4
        chip_str = '-p xcv300e-6-bg432 ';
        technology = 'VIRTEX-E';
        part = 'XCV300E';
        package = 'BG432';
        speed_grade = '-6';
    case 5
        chip_str = '-p xc2vp20-6-ff896 ';
        technology = 'VIRTEX2P';
        part = 'XC2VP20';
        package = 'FF896';
        speed_grade = '-6';
    case 6
        chip_str = '-p xc2vp70-7-ff1704 ';
        technology = 'VIRTEX2P';
        part = 'XC2VP70';
        package = 'FF1704';
        speed_grade = '-7';
    otherwise
        errordlg('Invalid FPGA chip selection');
        return;
end


%disp(['Checking for BEE IO block...']);
bpu_io_blk = find_system(design_name,'FollowLinks', 'on','LookUnderMasks','on','ReferenceBlock','bee_data_io/BPU Basic IO');
if isempty(bpu_io_blk)
    bpu_io_blk = find_system(design_name,'FollowLinks', 'on','LookUnderMasks','on','ReferenceBlock','bee_data_io_31/BPU Basic IO 31');
    bee_io_blk_vhdl_file = '';
end
if ~isempty(bpu_io_blk)
    %disp(['Found a BPU Basic IO block (',bee_io_blk_vhdl_file,')']);
    if length(bpu_io_blk) > 1
        error ('More than 1 BPU Basic IO block exist in the design!');
    else
        has_bee_io_blk = 1;
    end
end


old_dir = pwd;
cd(root_dir);
dirty_dn = design_name;
design_name = clear_name(design_name);
in_file_name = '';

if xsg_version == 1
    if exist([root_clr,'_xst.prj'])
        in_file_name = [root_clr,'_xst.prj'];
    elseif exist([design_name,'_xst.prj'])
        in_file_name = [design_name,'_xst.prj'];
    else
        errordlg(['File ',root_clr,'_xst.prj does not exist, please check and make sure System Generator ran correctly first']);
        return;
    end
elseif xsg_version == 5
    if exist(['xst_',root_clr,'_clk_wrapper.prj'])
        in_file_name = ['xst_',root_clr,'_clk_wrapper.prj'];
    elseif exist(['xst_',design_name,'_clk_wrapper.prj'])
        in_file_name = ['xst_',design_name,'_clk_wrapper.prj'];
    else
        errordlg(['File xst_',root_clr,'_clk_wrapper.prj does not exist, please check and make sure System Generator ran correctly first']);
        return;
    end
elseif xsg_version == 6
    if exist(['hdlFiles'])
        in_file_name = ['hdlFiles'];
    else
        errordlg(['File "hdlFiles" does not exist, please check and make sure System Generator ran correctly first']);
        return;
    end
elseif xsg_version > 2
    if exist(['xst_',root_clr,'.prj'])
        in_file_name = ['xst_',root_clr,'.prj'];
    elseif exist(['xst_',design_name,'.prj'])
        in_file_name = ['xst_',design_name,'.prj'];
    else
        errordlg(['File xst_',root_clr,'.prj does not exist, please check and make sure System Generator ran correctly first']);
        return;
    end
else
    error (['Unsupported Xilinx System Generator version']);
end

if use_synplify_pro
    dn_file_name = [design_name,'_synplicity.prj'];
else
    dn_file_name = [design_name,'.prj'];
end
dn_file = dir(dn_file_name);
src_file = dir(in_file_name);
if xsg_version < 4
    bee_bb_wrap(dirty_dn,xsg_version);
    bee_sf2vhd(dirty_dn,root_dir);
end
if xsg_version == 4
    bee_mcode_vhdl_fix(dirty_dn);
end
if ~exist(dn_file_name) | datenum(src_file.date) >= datenum(dn_file.date)
    infid = fopen(in_file_name,'r');
    outfid = fopen(dn_file_name,'w');
    if use_synplify_pro
        if has_bee_io_blk & bee_io_blk_vhdl_file ~= ''
            fprintf(outfid,'add_file -vhdl -lib work "%s/%s"\n',root_dir,bee_io_blk_vhdl_file);
        end
        while 1
            line = fgetl(infid);
            if ~ischar(line)
                break;
            elseif strcmp(line,'nosort')
                continue;
            else
                slash_pos = findstr(line,'//');
                colon_pos = findstr(line,':/');
                if (length(slash_pos) ~= 0 & slash_pos(1) == 1) | (length(colon_pos) ~= 0 & colon_pos(1) == 2 & ischar(line(1)))
                    dir_str = '';
                else
                    dir_str = [root_dir,'/'];
                end
                [fname,fext] = strtok(line,'.');
                switch fext
                    case {'.vhd' '.vhdl'}
                        file_type_flag = '-vhdl';
                    case {'.v'}
                        file_type_flag = '-verilog';
                    otherwise
                        error(['Unknown file extention: ',line]);
                end
                if xsg_version == 5
                    line = line([regexp(line,'\w+\.\w+'):length(line)]);
                end
                fprintf(outfid,['add_file ',file_type_flag,' -lib work "%s%s"\n'],dir_str,line);

                if strcmp(line,[design_name,'.vhd'])
                    break;
                end
            end
        end
        fprintf(outfid,'\nadd_file -constraint "%s.sdc"\n',design_name);
        fprintf(outfid,'\nproject -result_file "%s.edf"\n\n',design_name);
        fprintf(outfid,'set_option -top_module %s\n',design_name);
        fprintf(outfid,'set_option -technology %s\n',technology);
        fprintf(outfid,'set_option -part %s\n',part);
        fprintf(outfid,'set_option -package %s\n',package);
        fprintf(outfid,'set_option -speed_grade %s\n',speed_grade);
        fprintf(outfid,'set_option -default_enum_encoding %s\n',default_enum_encoding);
        fprintf(outfid,'set_option -symbolic_fsm_compiler %s\n',symbolic_fsm_compiler);
        fprintf(outfid,'set_option -resource_sharing %s\n',resource_sharing );
        fprintf(outfid,'set_option -frequency %s\n',frequency);
        fprintf(outfid,'set_option -fanout_limit %s\n',fanout_limit);
        fprintf(outfid,'set_option -maxfan_hard %s\n',maxfan_hard);
        fprintf(outfid,'set_option -disable_io_insertion %s\n',disable_io_insertion);
        fprintf(outfid,'set_option -write_verilog %s\n',write_verilog);
        fprintf(outfid,'set_option -write_vhdl %s\n',write_vhdl);
        fprintf(outfid,'set_option -write_apr_constraint %s\n',write_apr_constraint);
        %         fprintf(outfid,'set_option -mti_root %s\n',mti_root);



        fprintf(outfid,'\nproject -run synthesis\n');

    else
        while 1
            line = fgetl(infid);
            if ~ischar(line)
                break;
            else
                slash_pos = findstr(line,'//');
                colon_pos = findstr(line,':/');
                if strcmp(line,'nosort')
                    break;
                elseif (length(slash_pos) ~= 0 & slash_pos(1) == 1) | (length(colon_pos) ~= 0 & colon_pos(1) == 2 & ischar(line(1)))
                    fprintf(outfid,'%s\n',line);
                else
                    fprintf(outfid,'%s/%s\n',root_dir,line);
                end
                if strcmp(line,[design_name,'.vhd'])
                    break;
                end
            end
        end
    end
    fclose(infid);
    fclose(outfid);
end

switch (clk_pad_value)
    case 1
        clk_pad = 'BUFGP';
    case 2
        clk_pad = 'IBUF';
    case 3
        clk_pad = 'BUFGDLL';
    otherwise
        clk_pad = 'BUGGP';
end

if use_synplify_pro
    sdc_fid = fopen([root_dir,'/',design_name,'.sdc'],'w');
    fprintf(sdc_fid,['define_global_attribute  syn_edif_bit_format {%%n<%%i>}\n']);
    fprintf(sdc_fid,['define_global_attribute  syn_noarrayports {1}\n']);
    if ~isempty(clk_pin)
        fprintf(sdc_fid,['define_attribute {clk} xc_padtype {',clk_pad,'}\n']);
    end
    if has_bee_io_blk
        %         fprintf(sdc_fid,['define_attribute {',bee_io_blk_prefix,'_cclk_in[0]} xc_padtype {IBUF}\n']);
        %         fprintf(sdc_fid,['define_attribute {',bee_io_blk_prefix,'_cclk_in[0]} syn_noclockbuf {1}\n']);
        fprintf(sdc_fid,['define_attribute {cclk_in[0]} xc_padtype {IBUF}\n']);
        fprintf(sdc_fid,['define_attribute {cclk_in[0]} syn_noclockbuf {1}\n']);
    end
    fclose(sdc_fid);
else
    cst_fid = fopen([root_dir,'/',design_name,'.cst'],'w');
    if ~isempty(clk_pin)
        fprintf(cst_fid,['attribute CLOCK_SIGNAL of clk: signal is "yes";\n']);
        fprintf(cst_fid,['attribute clock_buffer of clk: signal is "',clk_pad,'";']);
    end
    fclose(cst_fid);
end


if strcmp(root_clr,design_name)
    if exist([design_name,'.ucf.bak'])
        delete([design_name,'.ucf.bak']);
    end
    if xsg_version == 4
        if dos(['copy ',design_name,'.xcf ',design_name,'.ucf']) ~= 0
            errordlg(['Error renaming file ',design_name,'.ucf']);
            return;
        end
        dos(['copy ',design_name,'.xcf ',design_name,'.ucf.bak'])
    elseif xsg_version == 5 | xsg_version == 6
        if dos(['copy ',design_name,'_clk_wrapper.xcf ',design_name,'.ucf']) ~= 0
            errordlg(['Error renaming file ',design_name,'_clk_wrapper.ucf']);
            return;
        end
        dos(['copy ',design_name,'_clk_wrapper.xcf ',design_name,'.ucf.bak'])
    else
        if dos(['copy ',design_name,'.ucf ',design_name,'.ucf.bak']) ~= 0
            errordlg(['Error renaming file ',design_name,'.ucf']);
            return;
        end
    end
    [ucf_infid,msg] = fopen([root_dir,'/',design_name,'.ucf.bak'],'r');
    [ucf_fid,msg] = fopen([root_dir,'/',design_name,'.ucf'],'w');
    if ucf_infid == -1
        errordlg(sprintf('Error: cannot open file %s',[root_dir,'/',design_name,'.ucf.bak']));
        return;
    end
    if ucf_fid == -1
        errordlg(sprintf('Error: cannot open file %s',[root_dir,'/',design_name,'.ucf']));
        return;
    end

    while 1
        line = fgetl(ucf_infid);
        if ~ischar(line)
            break;
        end
        if xsg_version >= 4
            line = strrep(line,'(','<');
            line = strrep(line,')','>');
        end

        if assign_io & ~isempty(findstr(line,'LOC ='))
        elseif ~isempty(findstr(line,'NET "clk" LOC = "'))
        elseif ~isempty(findstr(line,'IOSTANDARD ='))
        elseif has_bee_io_blk
            if ~isempty(findstr(line,['LOC = "Z0"']))
                %             elseif ~isempty(findstr(line,['NET "',bee_io_blk_prefix,'_data_in_in']))
                %                 fprintf(ucf_fid,['NET "',bee_io_blk_prefix,'_data_inout',line(findstr(line,'<'):length(line)),'\n']);
            elseif ~isempty(findstr(line,['NET "data_in_in']))
                fprintf(ucf_fid,['NET "data_inout',line(findstr(line,'<'):length(line)),'\n']);
            else
                fprintf(ucf_fid,'%s\n',line);
            end
        else
            if ~isempty(findstr(line,['LOC = "Z0"']))
            elseif ~isempty(findstr(line,['NET "zbt_data_out_out']))
                fprintf(ucf_fid,['NET "zbt_data_inout',line(findstr(line,'<'):length(line)),'\n']);
            else
                fprintf(ucf_fid,'%s\n',line);
            end
        end
    end
    if ~isempty(clk_pin)
        fprintf(ucf_fid,['NET "clk" LOC = "',clk_pin,'";\n']);
        if use_lvds_clk
            fprintf(ucf_fid,['NET "clk" IOSTANDARD = LVDS;\n']);
        end
    end


    if assign_io
        gws = find_system(root,'FollowLinks','on','LookUnderMasks','all','FunctionName','xfix2m','locs_specified','on');
        gws = [gws; find_system(root,'FollowLinks','on','LookUnderMasks','all','FunctionName','m2xfix','locs_specified','on')];

        load bee_routes;
        for i=1:length(gws)
            gw = gws{i};
            if xsg_version ==1
                net_name = clear_name(strrep([get_param(gw,'Parent'),'/',get_param(gw,'Name')],[root,'/'],''));
            else
                net_name = clear_name([get_param(gw,'Parent'),'/',get_param(gw,'Name')]);
            end
            [dummy,io_std] = strtok(get_param(gw,'tag'),':');
            if ~isempty(io_std)
                io_std = upper(io_std(2:length(io_std)));
            end
            pins = eval(get_param(gw,'LOCs'));
            if length(pins) == 1
                fprintf(ucf_fid,'NET "%s<0>" LOC = "%s";\n',net_name,pins{1});
                if ~isempty(io_std)
                    fprintf(ucf_fid,'NET "%s<0>" IOSTANDARD = %s;\n',net_name,io_std);
                end
            else
                for j=1:length(pins)
                    fprintf(ucf_fid,'NET "%s<%d>" LOC = "%s";\n',net_name,j-1,pins{j});
                    if ~isempty(io_std)
                        fprintf(ucf_fid,'NET "%s<%d>" IOSTANDARD = %s;\n',net_name,j-1,io_std);
                    end
                end
            end
        end
    end

    %    if exist([root_dir,'/',design_name,'.io'])
    %        io_fid = fopen([root_dir,'/',design_name,'.io'],'r');
    %        while 1
    %            line = fgetl(io_fid);
    %            if ~ischar(line)
    %                break;
    %            else
    %                [net_name,R] = strtok(line);
    %                [bit_width,R] = strtok(R);
    %                [iostandard,R] = strtok(R);
    %                for j = 1:str2num(bit_width)
    %                    fprintf(ucf_fid,'NET "%s<%d>" IOSTANDARD = %s;\n',net_name,j-1,iostandard);
    %                end
    %            end
    %        end
    %        fclose(io_fid);
    %    end


    fclose(ucf_fid);
    fclose(ucf_infid);
end


% if (has_bee_io_blk)
% %     patch_data_io([design_name,'.vhd'],bee_io_blk_prefix);
%     patch_data_io([design_name,'.vhd']);
%     if (bee_io_blk_vhdl_file ~= '')
%         dos(strrep(['copy ',vhdl_lib_path,'\',bee_io_blk_vhdl_file,' .\'],'/','\'));
%     end
% end
% patch_zbt_io([design_name,'.vhd']);


gws = find_system(dirty_dn,'FollowLinks','on','LookUnderMasks','all','FunctionName','xfix2m','locs_specified','off','hdl_port','on');
gws = [gws; find_system(dirty_dn,'FollowLinks','on','LookUnderMasks','all','FunctionName','m2xfix','locs_specified','off')];
unassigned_gws = [];
for i=1:length(gws)
    gw = gws{i};
    prt_sys = get_param(gw,'parent');
    bb = find_system(prt_sys,'FollowLinks','on','LookUnderMasks','all','FunctionName','xl_BlackBox');
    if isempty(bb)
        unassigned_gws = [unassigned_gws; {gw}];
    end
end
if  ~isempty(unassigned_gws)
    has_unassigned_pins = 1;

    err_str = ['The Following ',num2str(length(unassigned_gws)),' Xilinx Gateways do not have I/O pins assigned, configuration step will not run with unassigned pins!\n'];
    for i=1:length(unassigned_gws)
        err_str = [err_str,'\n  ',num2str(i),') ',unassigned_gws{i}];
    end
    errordlg(sprintf(err_str),'Warning');
else
    has_unassigned_pins = 0;
end




switch design_flow
    case 1 %synth only
        if use_synplify_pro
            cmd_str = ['synplify_pro -batch ',design_name,'_synplicity.prj'];
        else
            cmd_str = ['xflow ',chip_str,syn_str,design_name,'.prj'];
        end
    case 2 %translate only
        if use_synplify_pro
            cmd_str = ['xflow -implement translate.opt ',chip_str,design_name,'.edf'];
        else
            cmd_str = ['xflow -implement translate.opt ',chip_str,design_name,'.ngc'];
        end
    case 3 %pre chipscope
        if use_synplify_pro
            cmd_str = ['synplify_pro -batch ',design_name,'_synplicity.prj',char(10)];
            cmd_str = [cmd_str, 'xflow -implement translate.opt ',chip_str,design_name,'.edf'];
        else
            cmd_str = ['xflow -implement translate.opt ',chip_str,syn_str,design_name,'.prj'];
        end

    case 4 %post chipscope
        if has_unassigned_pins
            errordlg('Simulink design has unassigned I/O pads on gateways, bit stream will not be generated, please define all pin pads before procesing.','Error');
            return;
        end
        if use_synplify_pro
            cmd_str = ['xflow ',chip_str,imp_str,conf_str,design_name,'.edf'];
        else
            cmd_str = ['xflow ',chip_str,imp_str,conf_str,design_name,'.ngc'];
        end
    case 5 %implement only
        if use_synplify_pro
            cmd_str = ['xflow ',chip_str,imp_str,design_name,'.edf'];
        else
            cmd_str = ['xflow ',chip_str,imp_str,design_name,'.ngc'];
        end
    case 6 %configure only
        if has_unassigned_pins
            errordlg('Simulink design has unassigned I/O pads on gateways, bit stream will not be generated, please define all pin pads before procesing.','Error');
            return;
        end
        cmd_str = ['xflow ',chip_str,conf_str,design_name,'.ncd'];
    case 7 %complete build
        if has_unassigned_pins
            errordlg('Simulink design has unassigned I/O pads on gateways, bit stream will not be generated, please define all pin pads before procesing.','Error');
            return;
        end
        if use_synplify_pro
            cmd_str = ['synplify_pro -batch ',design_name,'_synplicity.prj',char(10)];
            cmd_str = [cmd_str, 'xflow ',chip_str,imp_str,conf_str,design_name,'.edf'];
        else
            cmd_str = ['xflow ',chip_str,syn_str,imp_str,conf_str,design_name,'.prj'];
        end

    case 8 % Back Annotate Pin Loc to UCF
        cmd_str = ['pin2ucf ',design_name,'.ncd -r pin2ucf.log -o ',design_name,'.ucf'];
    case 9 % Behavior Simulation
        cmd_str = ['modelsim pn_behavioral.do &'];
    case 10 % Post Translate Functional simulation
        %cmd_str = ['ngdanno -o ',design_name,'_translate.nga ',design_name,'.ngd',char(10)];
        cmd_str = ['ngd2vhdl -w ',design_name,'.ngd ',design_name,'_translate.vhd',char(10)];
        cmd_str = [cmd_str,'modelsim pn_posttranslate.do &'];
    case 11 % Post PAR Timing sitmulation
        cmd_str = ['ngdanno -o ',design_name,'.nga ',design_name,'.ncd ',design_name,'_map.ngm',char(10)];
        cmd_str = [cmd_str,'ngd2vhdl -w ',design_name,'.nga ',design_name,'_timesim.vhd',char(10)];
        cmd_str = [cmd_str,'modelsim pn_postpar.do &'];
    otherwise
        errordlg('Invalid ISE Design Flow Choice');
        return;
end


if exist('xflow.bat')
    delete xflow.bat;
    delete *.opt;
end

disp(['Command [',pwd,']: ',cmd_str]);
R = cmd_str;
start_time = now;
while 1
    [T,R] = strtok(R,char(10));
    result = dos(T);
    if result ~= 0 | isempty(R)
        break;
    end
end
duration = now - start_time;

cd(old_dir);

if result ~= 0
    if design_flow == 1 & use_synplify_pro
        errordlg('Error detected while runing ISE flow, please check the Synplify log file for details');
    else
        errordlg('Error detected while runing ISE flow, please check the xflow.log file for details');
    end
else
    msgbox(sprintf('Xilinx ISE Design Flow Operation Completed Sucessfully.\n  Total Run Time: %d days %s seconds',floor(duration),datestr(rem(duration,1),13)));
end




% --------------------------------------------------------------------
function varargout = imp_opt_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = listbox3_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = design_name_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = design_flow_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = fpga_chip_Callback(h, eventdata, handles, varargin)
fpga_chip = get(handles.fpga_chip,'Value');
switch fpga_chip
    case 1
        set(handles.clk_pin,'String','P89');
    case 2
        set(handles.clk_pin,'String','D21');
    case 3
        set(handles.clk_pin,'String','N8');
    case 4
        set(handles.clk_pin,'String','A16');
    case {5 6}
        set(handles.clk_pin,'String','');
        set(handles.conf_opt,'Value',2);
    otherwise
        errordlg('Invalid FPGA chip selection');
        return;
end


% --------------------------------------------------------------------
function varargout = log_menu_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = view_log_Callback(h, eventdata, handles, varargin)
design_name = get(handles.design_name,'String');
view_choice = get(handles.log_menu,'Value');


[root,root_dir,glb_ce,glb_clr,clk_period] = get_xsg_info(design_name);

editor = '"c:/Program Files/Windows NT/Accessories/wordpad" ';

design_name = clear_name(design_name);
switch view_choice
    case 1 %XFLOW log
        if ~exist([root_dir,'/xflow.log'])
            errordlg('XFLOW log file does not exist, please make sure you have run the flow first.');
            return;
        end
        dos([editor,root_dir,'/xflow.log &']);
    case 2 %MAP report
        if ~exist([root_dir,'/',design_name,'_map.mrp'])
            errordlg('MAP report file does not exist, please make sure you have run the MAP first.');
            return;
        end
        dos([editor,root_dir,'/',design_name,'_map.mrp &']);
    case 3 %Timing report
        if ~exist([root_dir,'/',design_name,'.twr'])
            errordlg('Timing report file does not exist, please make sure you have run the implementation first.');
            return;
        end
        dos([editor,root_dir,'/',design_name,'.twr &']);
    case 4 %UCF
        if ~exist([root_dir,'/',design_name,'.ucf'])
            errordlg('User Constraint file does not exist, please make sure you have run the System Generator first.');
            return;
        end
        dos([editor,root_dir,'/',design_name,'.ucf &']);
    case 5 %Synplify Pro Log
        if ~exist([root_dir,'/',design_name,'.srr'])
            errordlg('Synplify Log file does not exist, please make sure you have run the synthesis first.');
            return;
        end
        dos([editor,root_dir,'/',design_name,'.srr &']);
end
return;


% --------------------------------------------------------------------
function varargout = get_gcs_Callback(h, eventdata, handles, varargin)
set(handles.design_name,'String',gcs);
return;


% --------------------------------------------------------------------
function varargout = open_sys_Callback(h, eventdata, handles, varargin)
design_name = get(handles.design_name,'String');
if isempty(gcs) | ~strcmp(gcs,design_name)
    try
        open_system(design_name);
    catch
        errordlg(sprintf('Error cannot open system %c%s%c',39, design_name, 39));
        return;
    end
end
return;


% --------------------------------------------------------------------
function varargout = checkbox3_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------



% --------------------------------------------------------------------
function varargout = clk_pin_Callback(h, eventdata, handles, varargin)





% --------------------------------------------------------------------
function varargout = syn_opt_Callback(h, eventdata, handles, varargin)




% --------------------------------------------------------------------
function varargout = assign_io_Callback(h, eventdata, handles, varargin)


% --- Executes during object creation, after setting all properties.
function xsg_version_CreateFcn(hObject, eventdata, handles)
% hObject    handle to xsg_version (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.IS
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in xsg_version.
function xsg_version_Callback(hObject, eventdata, handles)
% hObject    handle to xsg_version (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns xsg_version contents as cell array
%        contents{get(hObject,'Value')} returns selected item from xsg_version


% --- Executes during object creation, after setting all properties.
function clkpad_CreateFcn(hObject, eventdata, handles)
% hObject    handle to clkpad (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end


% --- Executes on selection change in clkpad.
function clkpad_Callback(hObject, eventdata, handles)
% hObject    handle to clkpad (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns clkpad contents as cell array
%        contents{get(hObject,'Value')} returns selected item from clkpad


% --- Executes on button press in lvds_clock.
function lvds_clock_Callback(hObject, eventdata, handles)
% hObject    handle to lvds_clock (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of lvds_clock


