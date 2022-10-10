#!/usr/bin/env python

import itertools
import IPython

ipshell = IPython.Shell.IPShellEmbed()

XA = ( \
('XXa_c00', 'XXa_c08', 'XXa_c16', 'XXa_c24', 'XXa_c32', 'XXa_c40', 'XXa_c48', 'XXa_c56'), \
('YYa_c00', 'YYa_c08', 'YYa_c16', 'YYa_c24', 'YYa_c32', 'YYa_c40', 'YYa_c48', 'YYa_c56'), \
('ZZa_c00', 'ZZa_c08', 'ZZa_c16', 'ZZa_c24', 'ZZa_c32', 'ZZa_c40', 'ZZa_c48', 'ZZa_c56'), \
('XYr_c00', 'XYr_c08', 'XYr_c16', 'XYr_c24', 'XYr_c32', 'XYr_c40', 'XYr_c48', 'XYr_c56'), \
('YZr_c00', 'YZr_c08', 'YZr_c16', 'YZr_c24', 'YZr_c32', 'YZr_c40', 'YZr_c48', 'YZr_c56'), \
('ZXr_c00', 'ZXr_c08', 'ZXr_c16', 'ZXr_c24', 'ZXr_c32', 'ZXr_c40', 'ZXr_c48', 'ZXr_c56'), \
('XYi_c00', 'XYi_c08', 'XYi_c16', 'XYi_c24', 'XYi_c32', 'XYi_c40', 'XYi_c48', 'XYi_c56'), \
('YZi_c00', 'YZi_c08', 'YZi_c16', 'YZi_c24', 'YZi_c32', 'YZi_c40', 'YZi_c48', 'YZi_c56'), \
('ZXi_c00', 'ZXi_c08', 'ZXi_c16', 'ZXi_c24', 'ZXi_c32', 'ZXi_c40', 'ZXi_c48', 'ZXi_c56'), \
)

XB = ( \
('XXa_c01', 'XXa_c09', 'XXa_c17', 'XXa_c25', 'XXa_c33', 'XXa_c41', 'XXa_c49', 'XXa_c57'), \
('YYa_c01', 'YYa_c09', 'YYa_c17', 'YYa_c25', 'YYa_c33', 'YYa_c41', 'YYa_c49', 'YYa_c57'), \
('ZZa_c01', 'ZZa_c09', 'ZZa_c17', 'ZZa_c25', 'ZZa_c33', 'ZZa_c41', 'ZZa_c49', 'ZZa_c57'), \
('XYr_c01', 'XYr_c09', 'XYr_c17', 'XYr_c25', 'XYr_c33', 'XYr_c41', 'XYr_c49', 'XYr_c57'), \
('YZr_c01', 'YZr_c09', 'YZr_c17', 'YZr_c25', 'YZr_c33', 'YZr_c41', 'YZr_c49', 'YZr_c57'), \
('ZXr_c01', 'ZXr_c09', 'ZXr_c17', 'ZXr_c25', 'ZXr_c33', 'ZXr_c41', 'ZXr_c49', 'ZXr_c57'), \
('XYi_c01', 'XYi_c09', 'XYi_c17', 'XYi_c25', 'XYi_c33', 'XYi_c41', 'XYi_c49', 'XYi_c57'), \
('YZi_c01', 'YZi_c09', 'YZi_c17', 'YZi_c25', 'YZi_c33', 'YZi_c41', 'YZi_c49', 'YZi_c57'), \
('ZXi_c01', 'ZXi_c09', 'ZXi_c17', 'ZXi_c25', 'ZXi_c33', 'ZXi_c41', 'ZXi_c49', 'ZXi_c57'), \
)

XC = ( \
('XXa_c02', 'XXa_c10', 'XXa_c18', 'XXa_c26', 'XXa_c34', 'XXa_c42', 'XXa_c50', 'XXa_c58'), \
('YYa_c02', 'YYa_c10', 'YYa_c18', 'YYa_c26', 'YYa_c34', 'YYa_c42', 'YYa_c50', 'YYa_c58'), \
('ZZa_c02', 'ZZa_c10', 'ZZa_c18', 'ZZa_c26', 'ZZa_c34', 'ZZa_c42', 'ZZa_c50', 'ZZa_c58'), \
('XYr_c02', 'XYr_c10', 'XYr_c18', 'XYr_c26', 'XYr_c34', 'XYr_c42', 'XYr_c50', 'XYr_c58'), \
('YZr_c02', 'YZr_c10', 'YZr_c18', 'YZr_c26', 'YZr_c34', 'YZr_c42', 'YZr_c50', 'YZr_c58'), \
('ZXr_c02', 'ZXr_c10', 'ZXr_c18', 'ZXr_c26', 'ZXr_c34', 'ZXr_c42', 'ZXr_c50', 'ZXr_c58'), \
('XYi_c02', 'XYi_c10', 'XYi_c18', 'XYi_c26', 'XYi_c34', 'XYi_c42', 'XYi_c50', 'XYi_c58'), \
('YZi_c02', 'YZi_c10', 'YZi_c18', 'YZi_c26', 'YZi_c34', 'YZi_c42', 'YZi_c50', 'YZi_c58'), \
('ZXi_c02', 'ZXi_c10', 'ZXi_c18', 'ZXi_c26', 'ZXi_c34', 'ZXi_c42', 'ZXi_c50', 'ZXi_c58'), \
)

YA = ( \
('XXa_c03', 'XXa_c11', 'XXa_c19', 'XXa_c27', 'XXa_c35', 'XXa_c43', 'XXa_c51', 'XXa_c59'), \
('YYa_c03', 'YYa_c11', 'YYa_c19', 'YYa_c27', 'YYa_c35', 'YYa_c43', 'YYa_c51', 'YYa_c59'), \
('ZZa_c03', 'ZZa_c11', 'ZZa_c19', 'ZZa_c27', 'ZZa_c35', 'ZZa_c43', 'ZZa_c51', 'ZZa_c59'), \
('XYr_c03', 'XYr_c11', 'XYr_c19', 'XYr_c27', 'XYr_c35', 'XYr_c43', 'XYr_c51', 'XYr_c59'), \
('YZr_c03', 'YZr_c11', 'YZr_c19', 'YZr_c27', 'YZr_c35', 'YZr_c43', 'YZr_c51', 'YZr_c59'), \
('ZXr_c03', 'ZXr_c11', 'ZXr_c19', 'ZXr_c27', 'ZXr_c35', 'ZXr_c43', 'ZXr_c51', 'ZXr_c59'), \
('XYi_c03', 'XYi_c11', 'XYi_c19', 'XYi_c27', 'XYi_c35', 'XYi_c43', 'XYi_c51', 'XYi_c59'), \
('YZi_c03', 'YZi_c11', 'YZi_c19', 'YZi_c27', 'YZi_c35', 'YZi_c43', 'YZi_c51', 'YZi_c59'), \
('ZXi_c03', 'ZXi_c11', 'ZXi_c19', 'ZXi_c27', 'ZXi_c35', 'ZXi_c43', 'ZXi_c51', 'ZXi_c59'), \
)

ipshell()
