%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
%   MeerKAT project http://www.kat.ac.za                                      %	
%   Copyright (C) 2010 Andrew Martens (martens.andrew@gmail.com)              %
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

function prog_dds_init(blk,varargin)

% prog_dds_init(blk, varargin)
%
% blk = The block to initialize.
% varargin = {'varname', 'value', ...} pairs
%
%
%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

defaults = {};
if same_state(blk, 'defaults', defaults, varargin{:}), return, end
check_mask_type(blk, 'prog_dds');
munge_block(blk, varargin{:});

lookup_bits = get_var('lookup_bits','defaults', defaults, varargin{:});
streams = get_var('streams', 'defaults', defaults, varargin{:});
data_bits = get_var('data_bits', 'defaults', defaults, varargin{:});
numerator = get_var('numerator', 'defaults', defaults, varargin{:});
denominator = get_var('denominator', 'defaults', defaults, varargin{:});
offset = get_var('offset', 'defaults', defaults, varargin{:});

%generate the initial sequence vector of pointers

%comments showing how vector was generated below
pnt_vec = [offset, mod([1:denominator-1]+(numerator*streams), denominator)];

pnt_vec = '[mod([0:denominator-1]+(numerator*streams), denominator), zeros(1,2^lookup_bits-denominator)]';
set_param([blk,'/prog_seq'], ... 
    'pnt_bits', 'lookup_bits', ...
    'pre_pnt_vec', pnt_vec, ...
    'pre_init_pnt', 'offset'); 

%generate the initial mixing products

%commented out code below shows how string specifying initial vector was
%derived

%% number of total lookup values
%i = [0:denominator*streams-1];
%offset = mod(floor(i/streams) + mod(i, streams)*numerator, denominator) 
%phase = (offset * 2 * pi)/ denominator;
%sin_vec = -sin(phase);
%%convert to unsigned values, with binary point at 0
%sin_vec_scale = sin_vec * 2^(data_bits-1); %scale to remove binary point
%sin_vec_pos = sin_vec_scale + 2^data_bits; %remove negative values
%sin_vec_up = mod(sin_vec_pos, 2^(data_bits)); %ensure negative values correctly recovered
%subplot(3,2,1); plot( offset );
%subplot(3,2,2); plot( phase );
%subplot(3,2,3); plot( sin_vec );
%subplot(3,2,4); plot( sin_vec_scale );
%subplot(3,2,5); plot( sin_vec_pos );
%subplot(3,2,6); plot( sin_vec_up );
%sin_vec_scale
%sin_vec_pos
%sin_vec_up

sin_vec = '[mod( -sin((mod(floor([0:denominator*streams-1]/streams) + mod([0:denominator*streams-1],streams) * numerator, denominator) * 2 * pi)/denominator) * 2^(data_bits-2) + 2^data_bits, 2^data_bits), zeros(1,(2^lookup_bits*streams)-(denominator*streams))]';
cos_vec = '[mod( cos((mod(floor([0:denominator*streams-1]/streams) + mod([0:denominator*streams-1],streams) * numerator, denominator) * 2 * pi)/denominator) * 2^(data_bits-2) + 2^data_bits, 2^data_bits), zeros(1,(2^lookup_bits*streams)-(denominator*streams))]';

set_param([blk, '/cos_bram'], ...
    'depth', '(2^lookup_bits)*streams', ...
    'initVector', cos_vec, 'latency', 'bram_latency', ...
    'distributed_mem', 'Block RAM', 'optimize', 'Speed');

set_param([blk, '/sin_bram'], ...
    'depth', '(2^lookup_bits)*streams', ...
    'initVector', sin_vec, 'latency', 'bram_latency', ...
    'distributed_mem', 'Block RAM', 'optimize', 'Speed');

set_param(blk,'AttributesFormatString','');

save_state(blk, 'defaults', defaults, varargin{:});   

