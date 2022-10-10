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

function patch_zbt_io(infile)

% Check arguments
if ~ischar(infile)
    error('First argument must be VHDL filename string');
end

% Open the toplevel VHDL file for reading
fid = fopen(infile, 'r');
if fid == -1
    error(sprintf('Failed to open file %s for reading', infile));
end



% Initialize loop variables
newfile = '';
got_arch = 0;

%
% Parse the file line-by-line
%
while 1
    linestr = fgets(fid);  % includes the newline...
    if linestr == -1, break; end
    
    % Case coverage check
    linenew = sprintf('!!!!! UNHANDED CASE !!!!! ==> %s', linestr);
    
    
    % Handle the string conversions, which occur before the architecture block
    if ~got_arch
        start = strfind(linestr, 'architecture');
        if ~isempty(start), got_arch = 1; end
        
        %for already patched file, just return;
        if ~isempty(strfind(linestr,'zbt_data_inout: inout'));
            fclose(fid);
            disp(['No patching needed for file: ',infile]);
            return;
        end
        
        % Also keep the line as-is if it's commented out
        dashes = strfind(linestr, '--');
        if ~isempty(dashes) & (dashes < 3)
            %fprintf(1, 'Skipping comment line: %s', linestr);
            linenew = linestr;
            
        else
            % Delete the line for the data output and enable ports
            suffix_start = strfind(linestr, 'zbt_data_out_out:');
            suffix_start = [suffix_start strfind(linestr, 'zbt_data_en_out:')];
            if ~isempty(suffix_start)
                %fprintf(1, 'Deleting output port line: %s', linestr);
                linenew = '';
                
            else
                % Convert the data input port to an INOUT port
                suffix_start = strfind(linestr, 'zbt_data_in_in:');
                if ~isempty(suffix_start)
                    %fprintf(1, 'Converting input port line: %s', linestr);
                    linenew = strrep(linestr, 'zbt_data_in_in:', 'zbt_data_inout:');
                    linenew = strrep(linenew, 'in std_', 'inout std_');
                else
                    % Otherwise, it's just another port
                    %fprintf(1, 'Leaving regular port line: %s', linestr);
                    linenew = linestr;
                end
            end
        end
        
        % And now to add the tristate bus behavior in the architecture block
    else
        
        % Find the beginning of the description section
        start = regexp(linestr, '^\s*begin');
        if ~isempty(start)
            % Insert our tristate behavior
            linenew = sprintf(['  signal zbt_data_en_out: ' ...
                    'std_logic_vector(0 downto 0);\n' ...
                    '  signal zbt_data_out_out: ' ...
                    'std_logic_vector(31 downto 0);\n' ...
                    '  signal zbt_data_in_in: ' ...
                    'std_logic_vector(31 downto 0);\n' ...
                    '\n\n' linestr '\n' ...
                    '  zbt_data_inout <= zbt_data_out_out ' ...
                    'when zbt_data_en_out(0) = ''1'' ' ...
                    'else "ZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZZ";\n' ...
                    '  zbt_data_in_in <= zbt_data_inout;\n']);
            %fprintf(1, 'Inserted tristate behavior\n');
            
        else
            % Otherwise keep all the other lines
            linenew = linestr;
        end
        
    end
    
    newfile = [newfile linenew];
    
end

% Rewrite the VHDL file
fclose(fid);
fid = fopen(infile, 'w');
if fid == -1
    error(sprintf('Failed to open file %s for writing', infile));
end
fwrite(fid, newfile, 'char');

% Close the files
fclose(fid);

% Exit
fprintf(1, 'Successfully inserted tristate bus data_inout\n');
