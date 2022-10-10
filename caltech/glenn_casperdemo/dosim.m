% dosim.m
% Finds the current model, 
% runs it's pre simulation setup program, <model name>_pre.m
% Runs the simulation
% Runs it's post simulation processing program <model name>_post.m

% Find a model to simulate
if ~exist('model')
    warning(sprintf('model not defined, using %s',gcs))
    model = gcs;
end
pre_sim = sprintf('%s_pre',model);
post_sim = sprintf('%s_post',model);

% If a pre simulation setup file exists, run it
if exist(pre_sim)
    eval(pre_sim);
end

% Run the model and time how long it takes
tic
sim(model);
toc

% Run the post processing code
if exist(post_sim)
    eval(post_sim);
end