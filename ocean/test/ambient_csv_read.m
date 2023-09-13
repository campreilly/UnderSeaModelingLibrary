function ambient_csv_read
    %clc; close all;
    
    %% PRINTING NOTE
    % Print width a width of 8.99"
    % and a height of 5.34"
    % If you want to match GSM plots

    %% User Params
    % These can each be a list of the values you want plotted 
    models = ["WENZ"];  % ["NUSC", "WENZ", "CONST"]
    seasta = [0:6];       % [0:6]
    shplvl = [1:7];       % [1:7]ambient_csv_read.m"
    rainrt = [0];       % [0-3]
    produce_leged = false;

    data = read_ambient_csv;
       
    dat_cat = [];
    lab_cat = [];
    
    for i=1:length(models)
        for j=1:length(seasta)
            for k=1:length(shplvl)
                for l=1:length(rainrt)
                    run_ind = find(...
                        [data.run.model] == models(i) ...
                        & [data.run.seastate] == seasta(j) ...
                        & [data.run.shiplevel] == shplvl(k) ...
                        & [data.run.rainstate] == rainrt(l));
                    if length(run_ind) > 1
                        % error
                    else
                        lab_cat = [lab_cat; ...
                            sprintf('%s_%d_%d_%d',models(i), seasta(j), shplvl(k), rainrt(l))];
                        dat_cat = [dat_cat;data.run(run_ind).noise];
                    end
                end
            end
        end
    end
    
    h = figure;
    xlabel('f (Hz)');
    ylabel('dB // 1 uPa // rtHz');
    semilogx(data.f, dat_cat);
    if produce_leged
        legend(lab_cat);
    end
    xlim([1 100000]);
    ylim([0 120]);

end



function [data] = read_ambient_csv

delimiter = ',';
formatSpec = '%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%s%[^\n\r]';
fileID = fopen(uigetfile('*.csv'),'r');

%% Read columns of data according to the format.
% This call is based on the structure of the file used to generate this
% code. If an error occurs for a different file, try regenerating the code
% from the Import Tool.
dataArray = textscan(fileID, formatSpec, 'Delimiter', delimiter, 'TextType', 'string',  'ReturnOnError', false);

%% Close the text file.
fclose(fileID);

%% Convert the contents of columns containing numeric text to numbers.
% Replace non-numeric text with NaN.
raw = repmat({''},length(dataArray{1}),length(dataArray)-1);
for col=1:length(dataArray)-1
    raw(1:length(dataArray{col}),col) = mat2cell(dataArray{col}, ones(length(dataArray{col}), 1));
end
numericData = NaN(size(dataArray{1},1),size(dataArray,2));

for col=[2:104]
    % Converts text in the input cell array to numbers. Replaced non-numeric
    % text with NaN.
    rawData = dataArray{col};
    for row=1:size(rawData, 1)
        % Create a regular expression to detect and remove non-numeric prefixes and
        % suffixes.
        regexstr = '(?<prefix>.*?)(?<numbers>([-]*(\d+[\,]*)+[\.]{0,1}\d*[eEdD]{0,1}[-+]*\d*[i]{0,1})|([-]*(\d+[\,]*)*[\.]{1,1}\d+[eEdD]{0,1}[-+]*\d*[i]{0,1}))(?<suffix>.*)';
        try
            result = regexp(rawData(row), regexstr, 'names');
            numbers = result.numbers;
            
            % Detected commas in non-thousand locations.
            invalidThousandsSeparator = false;
            if numbers.contains(',')
                thousandsRegExp = '^\d+?(\,\d{3})*\.{0,1}\d*$';
                if isempty(regexp(numbers, thousandsRegExp, 'once'))
                    numbers = NaN;
                    invalidThousandsSeparator = true;
                end
            end
            % Convert numeric text to numbers.
            if ~invalidThousandsSeparator
                numbers = textscan(char(strrep(numbers, ',', '')), '%f');
                numericData(row, col) = numbers{1};
                raw{row, col} = numbers{1};
            end
        catch
            raw{row, col} = rawData{row};
        end
    end
end


%% Split data into numeric and string columns.
rawNumericColumns = raw(:, [2:104]);
rawStringColumns = string(raw(:, 1));


%% Replace non-numeric cells with NaN
R = cellfun(@(x) ~isnumeric(x) && ~islogical(x),rawNumericColumns); % Find non-numeric cells
rawNumericColumns(R) = {NaN}; % Replace non-numeric cells


data.f = [raw{1,5:104}];
for i=2:size(raw,1)
   data.run(i-1).model = raw{ i,1 };
   data.run(i-1).seastate = raw{ i,2 };
   data.run(i-1).shiplevel = raw{ i,3 };
   data.run(i-1).rainstate = raw{ i,4 };
   data.run(i-1).noise = [rawNumericColumns{ i,4:103 }];
end

end

