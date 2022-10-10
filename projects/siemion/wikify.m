function out = wikify(comment, lines)
warning off;

%change to diary dir.. not sure why this should be necessary
cd (strrep(userpath, ':',''));

%flush diary
diary off
diary on

LetterStore = char(97:122); % string containing all allowable letters (in this case lower case only)
randomstring = strcat('matlab_figure_', LetterStore(ceil(length(LetterStore).*rand(1,5))));


%get diary dir
diarypath = strcat(strrep(userpath, ':',''), '/', get(0,'DiaryFile'));
curcommand = strcat('sed ''/^$/d'' ', ' /', diarypath, ' | tail -', num2str(lines),' > /tmp/',randomstring,'.txt');
[x,y] = unix(curcommand);
print('-depsc', strcat('/tmp/',randomstring,'.eps'));
%[x,y] = unix(strcat('/usr/bin/wikiupload.pl "/tmp/',randomstring,'*" "', comment, '"'));
unix(strcat('/usr/bin/wikiupload.pl "/tmp/',randomstring,'*" "', comment, '"'));
strcat('/usr/bin/wikiupload.pl "/tmp/',randomstring,'*" "', comment, '"')
warning on;
out = comment;

