
clear all; close all;

% add Offline Analysis Toolbox to path
initOAT;

%%

folder_path = "/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/results/";
file_name = "spk_pooling.dat";

file_path = strcat(folder_path, file_name);

SR = SpikeReader(file_path);
spk = SR.readSpikes(1000);
[r, c] = size(spk);
test_train_ratio = 0.80;


%%
[coeffs, score, latent] = pca(spk);


X_train = [spk(r/2+1 : r/2+r/4*test_train_ratio, :); spk(r/2+r/4+1 : r/2+r/4*(1+test_train_ratio), :)];
X_test = [spk(r/2+r/4*test_train_ratio+1 : r/2+r/4, :); spk(r/2+r/4*(1+test_train_ratio)+1 : end, :)];


% X_train = [score(r/2+1 : r/2+r/4*test_train_ratio, :); score(r/2+r/4+1 : r/2+r/4*(1+test_train_ratio), :)];
% X_test = [score(r/2+r/4*test_train_ratio+1 : r/2+r/4, :); score(r/2+r/4*(1+test_train_ratio)+1 : end, :)];

Y_train = [ones(size(X_train, 1)/2,1); 2*ones(size(X_train, 1)/2,1)];
Y_test = [ones(size(X_test, 1)/2,1); 2*ones(size(X_test, 1)/2,1)];

%%
% 
% knn = fitcsvm(X_train, Y_train, 'OptimizeHyperparameters','auto',...
%     'HyperparameterOptimizationOptions',...
%     struct('MaxObjectiveEvaluations',100, 'UseParallel', true, 'Repartition', true));

tic
knn = fitcsvm(X_train, Y_train, 'BoxConstraint', 0.05, 'KernelScale', 2.4, 'KernelFunction', 'linear');
toc


% best: 26 - cityblock: 88.85/88.12 and 21 - cityblock: 89.58/86.46
% 20 - cityblock: 89.17/88.54 and 15 - cityblock: 89.95/88.54


% knn = fitcknn(X_train(:,f>t), Y_train, 'OptimizeHyperparameters','auto',...
%     'HyperparameterOptimizationOptions',...
%     struct('MaxObjectiveEvaluations',100, 'UseParallel', true, 'Repartition', true));

% tic
% knn = fitcknn(X_train, Y_train, 'Distance', 'cityblock', 'NumNeighbors', 20);
% toc
    
% Computation of the error probability
train_res = predict(knn,X_train);  2.4208 
acc_train = 1 - sum(Y_train~=train_res)/length(Y_train)

train_res = predict(knn,X_test);
acc_test = 1 - sum(Y_test~=train_res)/length(Y_test)


%%
% % init NetworkMonitor
% NM = NetworkMonitor('/home/sweet/1-workdir/carlsim4_ductai199x/projects/speech_recognition/results/sim_spreg487.dat');
% 
% % plot network activity
% disp('NetworkMonitor.plot')
% disp('-------------------')
% disp('Press ''p'' to pause.')
% disp('Press ''q'' to quit.')
% disp(['Press ''s'' to enter stepping mode; then press the ' ...
% 	'right arrow key to step'])
% disp('one frame forward, press the left arrow key to step one frame back.')
% disp('Press ''s'' again to leave stepping mode.')
% disp(' ')
% % NM.plot;
% NM.recordMovie('test.avi', [2401:2500, 3601:3700]);
