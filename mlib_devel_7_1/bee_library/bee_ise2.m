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

function varargout = bee_ise2(varargin)
% BEE_ISE2 BEE Xilinx ISE Batch Tools GUI

% Last Modified by GUIDE v2.5 16-Mar-2005 22:04:46

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


    %get the Xilinx Sysgen version
    xsg = xlversion;
    xsg = strtok(xsg{1},' ');
    switch xsg
        case {'6.3' '6.3.p03' '7.1.01'}
            set(handles.xsg_version,'String','6.3');
        case {'6.2.R14' '6.2'}
            set(handles.xsg_version,'String','6.2');
        case {'6.1' '6.1.1'}
            set(handles.xsg_version,'String','6.1');
        otherwise
            errordlg(['Unsupported Xilinx System Generator version: ',xsg]);
            return;
    end
    fpga_hardware_Callback([], [], handles);

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
conf_opt = get(handles.conf_opt,'Value');
run_clean = get(handles.run_clean_dir,'Value');
run_xsg = get(handles.run_xsg,'Value');
run_synth = get(handles.run_synth,'Value');
run_edk_synth = get(handles.edk_ip_synth,'Value');
run_translate = get(handles.run_translate,'Value');
run_implementation = get(handles.run_implementation,'Value');
run_bitgen = get(handles.run_bitgen,'Value');
xsg_version = get(handles.xsg_version,'Value');
run_mode = get(handles.run_mode,'Value');
%use_cores_in_synth = get(handles.cores_in_synth,'Value');
clk_pin = get(handles.clk_pin_loc,'String');
clk_pad_types = get(handles.clk_pad_type,'String');
clk_pad = clk_pad_types{get(handles.clk_pad_type,'Value')};
clk_io_types = get(handles.clk_iostandard,'String');
clk_iostandard = clk_io_types{get(handles.clk_iostandard,'Value')};

[root_sys,root_dir,glb_ce,glb_clr,clk_period,chip,xilinx_family, synth, part, speed, package] = get_xsg_info(design_name);

if ~strcmp(root_sys, design_name)
    errordlg([design_name,' does not contain the Xilinx System Generator block, make sure XSG has been run on the correct Simulink subsystem.'],'Error');
    return;
end

if run_clean
    dos(['rd /Q /S ',strrep(root_dir,'/','\')]);
end

if exist(root_dir) ~= 7
    mkdir(root_dir);
end



frequency = sprintf('%3.3f',10^3/str2num(clk_period));

file_name = clear_name(design_name);
file_name = [file_name,'_clk_wrapper'];

chip_str = [' -p ',chip,' '];
switch synth
    case 'Synplify Pro'
        use_synplify_pro = 1;
        syn_cmd = ['synplify_pro -batch synplify_',clear_name(design_name),'.prj -tcl bee_ise_synplify.tcl -runall\n'];
        fid = fopen([root_dir,'/bee_ise_synplify.tcl'],'w');
%         if use_cores_in_synth
%             files = dir(root_dir);
%             for i=1:length(files)
%                 cur_file = files(i);
%                 if regexp(cur_file.name,'.edn$')
%                     fprintf(fid,['add_file ',cur_file.name,'\n']);
%                 end
%             end
%         end
        fprintf(fid,'\nadd_file -constraint "%s.sdc"\n',file_name);
        if run_edk_synth
            fprintf(fid,['set_option -disable_io_insertion true\n']);
            syn_cmd = [syn_cmd,'mkdir pcore\n','del pcore/*\n','xcopy *.edn pcore /Y\n','xcopy *.coe pcore /Y\n','xcopy *.mif pcore /Y\n','xcopy *.edf pcore /Y\n','del pcore.zip\n','zip pcore.zip pcore/*\n'];
        end
%          fprintf(fid,['set_option -fanout_limit 10\n']);
%          fprintf(fid,['set_option -pipe 1\n']);
%          fprintf(fid,['set_option -retiming 1\n']);
%         fprintf(fid,['set_option -symbolic_fsm_compiler 1\n']);
%         fprintf(fid,['set_option -resource_sharing 1\n']);
        fclose(fid);
        sdc_fid = fopen([root_dir,'/',file_name,'.sdc'],'w');
        %fprintf(sdc_fid,['define_global_attribute  syn_edif_bit_format {%%n<%%i>}\n']);
        fprintf(sdc_fid,['define_global_attribute  syn_noarrayports {1}\n']);
        if ~isempty(clk_pin)
            fprintf(sdc_fid,['define_attribute {clk} xc_padtype {',clk_pad,'}\n']);
        end
        fclose(sdc_fid);
    case 'XST'
        file_ext = '.prj';
        syn_cmd = ['copy /Y hdlFiles ',file_name,file_ext,'\n'];
        syn_cmd = [syn_cmd,'xflow ',chip_str,' -synth xst_vhdl.opt ',file_name,file_ext,'\n'];
        syn_cmd = [syn_cmd,'del xflow.bat *.opt\n'];
        use_synplify_pro = 0;
    otherwise
        errordlg(['Invalid synthesizer: ',synth]);
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


if use_synplify_pro
    file_ext = '.edf';
else
    file_ext = '.ngc';
end

% switch design_flow
%     case 1 %synth only
%         cmd_str = syn_cmd;
%     case 2 %translate only
%         imp_str = ' -implement translate.opt ';
%         cmd_str = ['xflow ',chip_str,imp_str,file_name,file_ext,'\n'];
%     case 3 %pre chipscope
%         imp_str = ' -implement translate.opt ';
%         cmd_str = [syn_cmd,'xflow ',chip_str,imp_str,file_name,file_ext,'\n'];
%     case 4 %post chipscope
%         cmd_str = ['xflow ',chip_str,imp_str,conf_str,file_name,file_ext,'\n'];
%     case 5 %implement only
%         cmd_str = ['xflow ',chip_str,imp_str,file_name,file_ext,'\n'];
%     case 6 %configure only
%         file_ext = '.ncd';
%         cmd_str = ['xflow ',chip_str,conf_str,file_name,file_ext,'\n'];
%     case 7 %complete build
%         cmd_str = [syn_cmd,'xflow ',chip_str,imp_str,conf_str,file_name,file_ext,'\n'];
%     otherwise
%         errordlg('Invalid ISE Design Flow Choice');
%         return;
% end



if ~run_synth & ~run_edk_synth
    syn_cmd = '';
end
if run_implementation
    imp_str = ' -implement balanced.opt ';
elseif run_translate
    imp_str = ' -implement translate.opt ';
else
    imp_str = '';
end
if ~run_bitgen
    conf_str = '';
elseif ~run_implementation
    file_ext = '.ncd';
end

if run_synth | run_edk_synth | run_translate | run_implementation | run_bitgen
    cmd_str = syn_cmd;
else
    cmd_str = '';
end

if run_translate | run_implementation | run_bitgen
    cmd_str = [cmd_str,'xflow ',chip_str,imp_str,conf_str,file_name,file_ext,'\n'];
end

old_dir = pwd;

fid = fopen([root_dir,'/bee_ise_run.bat'],'w');
matches = regexp(root_dir,'^(?<drive_letter>\w:)','names');
if ~isempty(matches)
    fprintf(fid,[matches.drive_letter,'\n']);
end
fprintf(fid,['cd ',strrep(root_dir,'/','\\'),'\n']);
fprintf(fid,['del *.opt xflow.bat\n']);
fprintf(fid,cmd_str);
fclose(fid);

start_time = now;
if run_xsg
    disp('Running system generator');
    xsg_result = xlGenerateButton([root_sys,'/ System Generator']);
else
    xsg_result = 0;
end
xsg_run_time = now - start_time;

%edit UCF file
[ucf_fid,msg] = fopen([root_dir,'/',file_name,'.ucf'],'w');
if ucf_fid == -1
    errordlg(sprintf('Error: cannot write to %s',[root_dir,'/',file_name,'.ucf']));
    return;
end

if exist([root_dir,'/',file_name,'.ncf'])
    [cf_fid,msg] = fopen([root_dir,'/',file_name,'.ncf'],'r');
    while 1
        line = fgetl(cf_fid);
        if ~ischar(line)
            break;
        end
            line = strrep(line,'(','<');
            line = strrep(line,')','>');
        if ~isempty(regexp(line,'^NET\s+"clk"\s+TNM_NET'))
            continue;
        end
        if ~isempty(regexp(line,'^TIMESPEC\s+"\w*clk\w*"\s+='))
            continue;
        end
        if ~isempty(regexp(line,'^NET\s+"clk"\s+LOC\s+='))
            continue;
        end
        fprintf(ucf_fid,[line,'\n']);
    end
    fclose(cf_fid);
end

if ~isempty(clk_pin)
    fprintf(ucf_fid,['NET "clk" LOC = "',clk_pin,'" | IOSTANDARD = ',clk_iostandard,';\n']);
end

fprintf(ucf_fid,'NET "clk" TNM_NET = "clk";\n');
fprintf(ucf_fid,'TIMESPEC "TS_clk" = PERIOD "clk" %s ns HIGH 50 %%;\n',clk_period);

fclose(ucf_fid);


switch run_mode
    case 1 % Directly run tool flow in Matlab
        start_time = now;
        if xsg_result == 0
            result = dos([strrep(root_dir,'/','\'),'\bee_ise_run.bat']);
        else
            result = 1;
        end
        duration = now - start_time + xsg_run_time;
        if result ~= 0
            if xsg_result ~= 0
                help xlGenerateButton;
                errordlg(sprintf('Error running Xilinx System Generator: status %d',xsg_result));
            else
                errordlg(sprintf('Error detected while runing ISE flow, please check the xflow.log file for details.\n  Total Run Time: %d days %s seconds',floor(duration),datestr(rem(duration,1),13)));
            end
        else
            msgbox(sprintf('Xilinx ISE Design Flow Operation Completed Sucessfully.\n  Total Run Time: %d days %s seconds',floor(duration),datestr(rem(duration,1),13)));
        end
    case 2 % Run in seperate CMD window
        result = dos([strrep(root_dir,'/','\'),'\bee_ise_run.bat &']);
    case 3 % Generate batch files only
        msgbox(['bee_ise_run.bat file has been generated in ',root_dir]);
    case 4 % Submit to batch system
        fid = fopen([root_dir,'/bee_ise_run.sub'],'w');
        fprintf(fid,'universe = vanilla\n');
        fprintf(fid,'executable = bee_ise_run.bat\n');
        fprintf(fid,'log = bee_ise_run.log\noutput = bee_ise_run.out\nerror = bee_ise_run.err\n');
        fprintf(fid,['Initialdir = ',strrep(root_dir,'/','\\'),'\n']);
        fprintf(fid,'getenv = True\n');
        fprintf(fid,'queue\n');
        fclose(fid);
        batch_cmd = ['cd ',strrep(root_dir,'/','\\'),'\n'];
        batch_cmd = [batch_cmd, 'condor_submit bee_ise_run.sub\n'];
        batch_cmd = [batch_cmd, 'cd ',strrep(old_dir,'/','\\'),'\n'];
        fid = fopen([root_dir,'/bee_ise_condor.bat'],'w');
        fprintf(fid,batch_cmd);
        fclose(fid);
        dos([strrep(root_dir,'/','\'),'\bee_ise_condor.bat']);
        msgbox('Your BEE_ISE batch job has been submitted to the Condor system, please check the job status for results');
    otherwise
        error(['Unknown run_mode value: ',num2str(run_mode)]);
end




% --------------------------------------------------------------------
function varargout = view_log_Callback(h, eventdata, handles, varargin)
design_name = get(handles.design_name,'String');
view_choice = get(handles.log_menu,'Value');


[root,root_dir,glb_ce,glb_clr,clk_period] = get_xsg_info(design_name);

editor = '"c:/Program Files/Windows NT/Accessories/wordpad" ';

design_name = [clear_name(design_name),'_clk_wrapper'];

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
            if ~exist([root_dir,'/stdout.log'])
                errordlg('Synplify Log file does not exist, please make sure you have run the synthesis first.');
                return;
            else
                dos([editor,root_dir,'/stdout.log &']);
            end
        else
            dos([editor,root_dir,'/',design_name,'.srr &']);
        end
    case 6 %BEE_ISE batch file
        if ~exist([root_dir,'/bee_ise_run.bat'])
            errordlg('BEE_ISE batch file does not exist.');
            return;
        end
        dos([editor,root_dir,'/bee_ise_run.bat &']);
    case 7 %NCF file
        if ~exist([root_dir,'/',design_name,'.ncf'])
            errordlg('NCF Constraint file does not exist, please make sure you have run the System Generator first.');
            return;
        end
        dos([editor,root_dir,'/',design_name,'.ncf &']);
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



% --- Executes on button press in run_mode.
function run_mode_Callback(hObject, eventdata, handles)
% hObject    handle to run_mode (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_mode




% --- Executes on button press in run_synth.
function run_synth_Callback(hObject, eventdata, handles)
% hObject    handle to run_synth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_synth


% --- Executes on button press in run_.
function run__Callback(hObject, eventdata, handles)
% hObject    handle to run_ (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_


% --- Executes on button press in run_trans.
function run_trans_Callback(hObject, eventdata, handles)
% hObject    handle to run_trans (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_trans




% --- Executes on button press in cores_in_synth.
function cores_in_synth_Callback(hObject, eventdata, handles)
% hObject    handle to cores_in_synth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of cores_in_synth





function clk_pin_loc_Callback(hObject, eventdata, handles)
% hObject    handle to clk_pin_loc (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: get(hObject,'String') returns contents of clk_pin_loc as text
%        str2double(get(hObject,'String')) returns contents of clk_pin_loc as a double


% --- Executes during object creation, after setting all properties.
function clk_pin_loc_CreateFcn(hObject, eventdata, handles)
% hObject    handle to clk_pin_loc (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: edit controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end





% --- Executes on selection change in fpga_hardware.
function fpga_hardware_Callback(hObject, eventdata, handles)
% hObject    handle to fpga_hardware (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns fpga_hardware contents as cell array
%        contents{get(hObject,'Value')} returns selected item from fpga_hardware
fpga_hardware = get(handles.fpga_hardware,'Value');
switch fpga_hardware
    case 1 % BEE1
        set(handles.clk_pin_loc,'String','D21');
        set(handles.conf_opt,'Value',1);
        set(handles.clk_iostandard,'Value',1);
        set(handles.clk_pad_type,'Value',1);
    case 2 % BEE2
        set(handles.clk_pin_loc,'String','AN22');
        set(handles.conf_opt,'Value',1);
        set(handles.clk_iostandard,'Value',3);
        set(handles.clk_pad_type,'Value',1);
    case 3 % custom
        set(handles.clk_pin_loc,'String','');
        set(handles.conf_opt,'Value',2);
        set(handles.clk_iostandard,'Value',1);
        set(handles.clk_pad_type,'Value',1);
    otherwise
        error('Uknown FPGA hardware selection');
end

% --- Executes during object creation, after setting all properties.
function fpga_hardware_CreateFcn(hObject, eventdata, handles)
% hObject    handle to fpga_hardware (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




% --- Executes on button press in clk_iostandard.
function clk_iostandard_Callback(hObject, eventdata, handles)
% hObject    handle to clk_iostandard (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of clk_iostandard




% --- Executes on selection change in clk_pad_type.
function clk_pad_type_Callback(hObject, eventdata, handles)
% hObject    handle to clk_pad_type (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns clk_pad_type contents as cell array
%        contents{get(hObject,'Value')} returns selected item from clk_pad_type


% --- Executes during object creation, after setting all properties.
function clk_pad_type_CreateFcn(hObject, eventdata, handles)
% hObject    handle to clk_pad_type (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




% --- Executes on button press in run_xsg.
function run_xsg_Callback(hObject, eventdata, handles)
% hObject    handle to run_xsg (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_xsg


% --- Executes on button press in run_translate.
function run_translate_Callback(hObject, eventdata, handles)
% hObject    handle to run_translate (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_translate


% --- Executes on button press in run_implementation.
function run_implementation_Callback(hObject, eventdata, handles)
% hObject    handle to run_implementation (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_implementation


% --- Executes on button press in run_bitgen.
function run_bitgen_Callback(hObject, eventdata, handles)
% hObject    handle to run_bitgen (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_bitgen


% --- Executes on selection change in design_flow.
function design_flow_Callback(hObject, eventdata, handles)
% hObject    handle to design_flow (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns design_flow contents as cell array
%        contents{get(hObject,'Value')} returns selected item from design_flow

flow_shortcuts = get(handles.design_flow,'String');
flow_choice = get(handles.design_flow,'Value');
switch flow_shortcuts{flow_choice}
    case 'Complete Build'
        set(handles.run_xsg,'Value',1);
        set(handles.run_synth,'Value',1);
        set(handles.run_translate,'Value',0);
        set(handles.run_implementation,'Value',1);
        set(handles.run_bitgen,'Value',1);
    case 'All but configuration'
        set(handles.run_xsg,'Value',1);
        set(handles.run_synth,'Value',1);
        set(handles.run_translate,'Value',0);
        set(handles.run_implementation,'Value',1);
        set(handles.run_bitgen,'Value',0);
    case 'Pre ChipScope (Synth + Trans)'
        set(handles.run_xsg,'Value',1);
        set(handles.run_synth,'Value',1);
        set(handles.run_translate,'Value',1);
        set(handles.run_implementation,'Value',0);
        set(handles.run_bitgen,'Value',0);
    case 'Post ChipScope (Imp+ Config)'
        set(handles.run_xsg,'Value',0);
        set(handles.run_synth,'Value',0);
        set(handles.run_translate,'Value',0);
        set(handles.run_implementation,'Value',1);
        set(handles.run_bitgen,'Value',1);
    otherwise
        error(['Unknown design flow shortcut: ',flow_shortcuts{flow_choice}]);
end


% --- Executes on selection change in clk_iostandard.
function io_standard_Callback(hObject, eventdata, handles)
% hObject    handle to clk_iostandard (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hints: contents = get(hObject,'String') returns clk_iostandard contents as cell array
%        contents{get(hObject,'Value')} returns selected item from clk_iostandard


% --- Executes during object creation, after setting all properties.
function clk_iostandard_CreateFcn(hObject, eventdata, handles)
% hObject    handle to clk_iostandard (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    empty - handles not created until after all CreateFcns called

% Hint: popupmenu controls usually have a white background on Windows.
%       See ISPC and COMPUTER.
if ispc
    set(hObject,'BackgroundColor','white');
else
    set(hObject,'BackgroundColor',get(0,'defaultUicontrolBackgroundColor'));
end




% --- Executes on button press in edk_ip_synth.
function edk_ip_synth_Callback(hObject, eventdata, handles)
% hObject    handle to edk_ip_synth (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of edk_ip_synth




% --- Executes on button press in run_clean_dir.
function run_clean_dir_Callback(hObject, eventdata, handles)
% hObject    handle to run_clean_dir (see GCBO)
% eventdata  reserved - to be defined in a future version of MATLAB
% handles    structure with handles and user data (see GUIDATA)

% Hint: get(hObject,'Value') returns toggle state of run_clean_dir


