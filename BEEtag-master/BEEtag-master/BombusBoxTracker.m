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
outputMovieName = 'ExampleTrackingMovieBB6.avi';
outputMovie = 1; % Set to 1 if you want to save a movie

if outputMovie == 1
    vidObj = VideoWriter(outputMovieName);
    vidObj.FrameRate = 1;
    open(vidObj);
end

beePaths = cell(numel(TD), 1);
for i = 1:numImages
    im = imread(fullfile(imageFolder, imageFiles(i).name));
    imshow(im);
    hold on;
    if ~isempty(trackingData(i).F) && isfield(trackingData(i).F, 'number')
        for j = 1:numel(TD)
            if numel(TD(j).CentroidX) >= i && ~isempty(TD(j).CentroidX(i))
                try
                    plot([TD(j).CentroidX(i) TD(j).FrontX(i)], [TD(j).CentroidY(i) TD(j).FrontY(i)], 'b-', 'LineWidth', 3);
                    text(TD(j).CentroidX(i), TD(j).CentroidY(i), num2str(TD(j).number(i)), 'FontSize', 15, 'Color', 'r');
                    disp("Code location plotted");
                catch
                    disp("Failed to plot code location");
                    continue;
                end
            end
        end
    end
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
