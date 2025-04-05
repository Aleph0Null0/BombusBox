clear;
close all;
imageFolder = uigetdir(); % User selects the folder containing images
imageFiles = dir(fullfile(imageFolder, '*.jpg')); % Get list of .jpg images
numImages = length(imageFiles); % Count the number of images

% Sort images by name (assuming "1.jpg", "2.jpg", etc.)
[~, sortedIdx] = sort(str2double({imageFiles.name}));
imageFiles = imageFiles(sortedIdx);

% Create empty frame for tracking output
trackingData = struct();

%% Loop across images
for i = 1:numImages
    %% Read in each image and track codes in it
    disp(strcat('tracking frame_', num2str(i), '_of_', num2str(numImages)));
    imagePath = fullfile(imageFolder, imageFiles(i).name);
    im = imread(imagePath);
    disp(imagePath);
    % Example tracking options:
    %F = locateCodesBB(im, 'threshMode', 1, 'bradleyFilterSize', [25 25],'vis', 1);
    F = locateCodesBB(im);
    disp(F);
    % Remove duplicates of any particular tag from single image
    if ~isempty(F)
        [~, uniqueIdx] = unique([F.number], 'stable');
        F = F(uniqueIdx);
    end

    % Append this single frame data to the master tracking output
    trackingData(i).F = F;
end

%% Extract unique codes if 'codelist' is not defined
if ~exist('codelist', 'var')
    allNumbers = [];
    for i = 1:numImages
        if ~isempty(trackingData(i).F)
            fprintf('Checking code in image %f.0',i)
            curNumbers = [trackingData(i).F.number];
            allNumbers = [allNumbers curNumbers];
        end
    end
    codelist = unique(allNumbers);
end

%% Rearrange data into an easier format
trackingDataReshaped = struct();
for i = 1:numImages
    F = trackingData(i).F;
    for j = 1:numel(codelist)
        if isfield(F, 'number') && ~isempty([F.number])
            FS = F([F.number] == codelist(j));
            if ~isempty(FS)
                trackingDataReshaped(j).CentroidX(i) = FS.Centroid(1);
                trackingDataReshaped(j).CentroidY(i) = FS.Centroid(2);
                trackingDataReshaped(j).FrontX(i) = FS.frontX;
                trackingDataReshaped(j).FrontY(i) = FS.frontY;
                trackingDataReshaped(j).number(i) = FS.number;
            end
        else
            disp("EMPTY FRAME")
        end
    end
end

%% Save data
save('trackingData.mat', 'trackingDataReshaped')

%% Replay sequence with tracking data
TD = trackingDataReshaped;
outputMovieName = 'TestSample.avi';
outputMovie = 0; % Set to 1 if you want to save a movie
if outputMovie == 1
    vidObj = VideoWriter(outputMovieName);
    vidObj.FrameRate = 1;
    open(vidObj);
end

% Create a cell array to store the path coordinates for each tag
paths = cell(numel(TD), 1);
for j = 1:numel(TD)
    % Initialize empty arrays to store valid centroid positions
    paths{j}.x = [];
    paths{j}.y = [];
    paths{j}.frameIndices = [];
    
    % Collect all valid centroid positions for each tag
    for i = 1:numImages
        if numel(TD(j).CentroidX) >= i && ~isempty(TD(j).CentroidX(i))...
               && (TD(j).CentroidX(i) ~= 0 || TD(j).CentroidY(i) ~= 0) 
            paths{j}.x = [paths{j}.x TD(j).CentroidX(i)];
            paths{j}.y = [paths{j}.y TD(j).CentroidY(i)];
            paths{j}.frameIndices = [paths{j}.frameIndices i];
        end
    end
end

% Define colors for each tag path
colors = parula(numel(TD));  % Generate distinct colors for each tag

for i = 1:numImages
    im = imread(fullfile(imageFolder, imageFiles(i).name));
    imshow(im);
    hold on;
    
    % Plot all paths up to the current frame
    for j = 1:numel(TD)
        % Find which points in the path are up to the current frame
        validIndices = find(paths{j}.frameIndices <= i);
        
        if ~isempty(validIndices)
            % Get coordinates up to this frame
            currentX = paths{j}.x(validIndices);
            currentY = paths{j}.y(validIndices);
            
            % Plot the path connections up to this frame
            if length(validIndices) > 1
                plot(currentX, currentY, '-', 'Color', colors(j,:), 'LineWidth', 4);
            end
            
            % If the tag is in the current frame, plot it
            if ismember(i, paths{j}.frameIndices)
                try
                    % Draw line from centroid to front
                    plot([TD(j).CentroidX(i) TD(j).FrontX(i)],...
                         [TD(j).CentroidY(i) TD(j).FrontY(i)],...
                         'w-', 'LineWidth', 3);

                    % Add tag number
                    text(TD(j).CentroidX(i)+10, TD(j).CentroidY(i)+10,...
                         num2str(TD(j).number(i)),...
                         'FontSize', 15, 'Color', 'g');

                    % Highlight current position with a marker
                    plot(TD(j).CentroidX(i), TD(j).CentroidY(i),...
                        'o', 'MarkerSize', 8,...
                        'MarkerFaceColor', colors(j,:), 'MarkerEdgeColor', 'k');
                    
                    %disp(['Plotted tag ' num2str(TD(j).number(i)) ' in frame ' num2str(i)]);
                catch
                    disp(['Failed to plot tag ' num2str(j) ' in frame ' num2str(i)]);
                    continue;
                end
            end
        end
    end
    
    title(['Frame ' num2str(i) '/' num2str(numImages)], 'FontSize', 14);
    
    drawnow;
    if outputMovie == 1
        currFrame = getframe;
        writeVideo(vidObj, currFrame);
    end
    hold off;
end

if outputMovie == 1
    close(vidObj);
    disp("Video saved successfully.")
end

%% Generate and save final paths image
% Use the last frame as background or create a blank image
finalImage = imread(fullfile(imageFolder, imageFiles(end).name));

figure('Position', [100, 100, size(finalImage, 2), size(finalImage, 1)]);
imshow(finalImage);
hold on;

% Plot full path for each tag
for j = 1:numel(TD)
    if ~isempty(paths{j}.x) && length(paths{j}.x) > 1
        % Plot the complete path
        plot(paths{j}.x, paths{j}.y, '-', 'Color', colors(j,:), 'LineWidth', 4);
        
        % Add markers for start and end points
        plot(paths{j}.x(1), paths{j}.y(1), 'o',...
            'MarkerSize', 10, 'MarkerFaceColor', 'g', 'MarkerEdgeColor', 'k');
        plot(paths{j}.x(end), paths{j}.y(end), 's',...
            'MarkerSize', 10, 'MarkerFaceColor', 'b', 'MarkerEdgeColor', 'k');
        
        % Add tag number at the end position
        text(paths{j}.x(end)+10, paths{j}.y(end),...
            ['Tag ' num2str(TD(j).number(end))], 'FontSize', 18,...
            'Color', 'g', 'FontWeight', 'bold');
    end
end

title('Complete Tag Paths', 'FontSize', 16);
hold off;